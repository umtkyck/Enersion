#!/usr/bin/env python3
"""
Enersion Control System - Web-Based GUI

Runs a local web server for the GUI.
Access via browser: http://localhost:8080

Author: Enersion
Version: 1.0.0
"""

import os
import sys
import json
import time
import threading
import logging
from http.server import HTTPServer, SimpleHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
import socketserver

# Add parent to path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from hal.rs485 import RS485, RS485Config, list_serial_ports
from protocol.enersion import EnersionProtocol, Address, DIOState

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Global state
app_state = {
    'connected': False,
    'port': '/dev/ttySTM9',
    'baudrate': 115200,
    'di_states': [False] * 64,
    'do_states': [False] * 64,
    'di_count': 0,
    'do_count': 0,
    'error': None
}

rs485 = None
protocol = None
polling = False
poll_thread = None


class EnersionHandler(SimpleHTTPRequestHandler):
    """HTTP request handler for the GUI"""
    
    def __init__(self, *args, **kwargs):
        # Serve from web directory
        self.directory = os.path.join(os.path.dirname(__file__), 'web')
        super().__init__(*args, **kwargs)
    
    def do_GET(self):
        """Handle GET requests"""
        parsed = urlparse(self.path)
        
        if parsed.path == '/api/state':
            self._send_json(app_state)
        elif parsed.path == '/api/ports':
            self._send_json({'ports': list_serial_ports()})
        elif parsed.path == '/':
            self._serve_index()
        else:
            # Serve static files
            super().do_GET()
    
    def do_POST(self):
        """Handle POST requests"""
        parsed = urlparse(self.path)
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length).decode('utf-8')
        
        try:
            data = json.loads(body) if body else {}
        except:
            data = {}
        
        if parsed.path == '/api/connect':
            result = self._handle_connect(data)
            self._send_json(result)
        elif parsed.path == '/api/disconnect':
            result = self._handle_disconnect()
            self._send_json(result)
        elif parsed.path == '/api/toggle':
            channel = data.get('channel', 0)
            result = self._handle_toggle(channel)
            self._send_json(result)
        elif parsed.path == '/api/set_all':
            state = data.get('state', False)
            result = self._handle_set_all(state)
            self._send_json(result)
        elif parsed.path == '/api/set_pattern':
            pattern = data.get('pattern', 0)
            result = self._handle_set_pattern(pattern)
            self._send_json(result)
        else:
            self._send_json({'error': 'Unknown endpoint'}, 404)
    
    def _send_json(self, data, status=200):
        """Send JSON response"""
        self.send_response(status)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())
    
    def _serve_index(self):
        """Serve the main HTML page"""
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.end_headers()
        self.wfile.write(get_html_page().encode())
    
    def _handle_connect(self, data):
        """Handle connect request"""
        global rs485, protocol, polling, poll_thread
        
        port = data.get('port', '/dev/ttySTM9')
        baudrate = data.get('baudrate', 115200)
        
        try:
            config = RS485Config(device=port, baudrate=baudrate)
            rs485 = RS485(config)
            
            if not rs485.open():
                return {'success': False, 'error': f'Failed to open {port}'}
            
            protocol = EnersionProtocol(rs485)
            
            # Test connection
            di_ok = protocol.ping(Address.CTRL_DIO)
            do_ok = protocol.ping(Address.CTRL_OUT)
            
            if not di_ok and not do_ok:
                logger.warning("No controllers responded")
            
            app_state['connected'] = True
            app_state['port'] = port
            app_state['baudrate'] = baudrate
            app_state['error'] = None
            
            # Start polling
            polling = True
            poll_thread = threading.Thread(target=poll_loop, daemon=True)
            poll_thread.start()
            
            return {'success': True, 'di_online': di_ok, 'do_online': do_ok}
            
        except Exception as e:
            logger.error(f"Connect error: {e}")
            return {'success': False, 'error': str(e)}
    
    def _handle_disconnect(self):
        """Handle disconnect request"""
        global rs485, protocol, polling
        
        polling = False
        time.sleep(0.2)
        
        if rs485:
            rs485.close()
            rs485 = None
        protocol = None
        
        app_state['connected'] = False
        return {'success': True}
    
    def _handle_toggle(self, channel):
        """Handle toggle output request"""
        if not app_state['connected'] or not protocol:
            return {'success': False, 'error': 'Not connected'}
        
        if 0 <= channel < 64:
            app_state['do_states'][channel] = not app_state['do_states'][channel]
            write_outputs()
            return {'success': True, 'channel': channel, 'state': app_state['do_states'][channel]}
        
        return {'success': False, 'error': 'Invalid channel'}
    
    def _handle_set_all(self, state):
        """Handle set all outputs request"""
        if not app_state['connected'] or not protocol:
            return {'success': False, 'error': 'Not connected'}
        
        app_state['do_states'] = [state] * 64
        write_outputs()
        return {'success': True}
    
    def _handle_set_pattern(self, pattern):
        """Handle set pattern request"""
        if not app_state['connected'] or not protocol:
            return {'success': False, 'error': 'Not connected'}
        
        for i in range(64):
            bit_idx = i % 8
            app_state['do_states'][i] = bool(pattern & (1 << bit_idx))
        
        write_outputs()
        return {'success': True}
    
    def log_message(self, format, *args):
        """Suppress default logging"""
        pass


def poll_loop():
    """Background polling loop"""
    global polling
    
    while polling and app_state['connected']:
        try:
            # Read DI
            di_state = protocol.read_digital_inputs()
            if di_state:
                app_state['di_states'] = di_state.to_list()
                app_state['di_count'] = sum(app_state['di_states'])
            
            # Read DO
            do_state = protocol.read_digital_outputs()
            if do_state:
                app_state['do_states'] = do_state.to_list()
                app_state['do_count'] = sum(app_state['do_states'])
            
            time.sleep(0.1)
            
        except Exception as e:
            logger.error(f"Poll error: {e}")
            time.sleep(1.0)


def write_outputs():
    """Write DO states to hardware"""
    if not protocol:
        return
    
    try:
        state = DIOState()
        state.from_list(app_state['do_states'])
        protocol.write_digital_outputs(state)
        app_state['do_count'] = sum(app_state['do_states'])
    except Exception as e:
        logger.error(f"Write error: {e}")


def get_html_page():
    """Generate the HTML page"""
    return '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Enersion Control System</title>
    <style>
        :root {
            --primary: #00D4AA;
            --primary-dark: #00A88A;
            --bg-dark: #0A0E14;
            --bg-card: #141B22;
            --bg-hover: #1A2530;
            --text-primary: #FFFFFF;
            --text-secondary: #8B9CAF;
            --text-muted: #5A6B7D;
            --success: #00E676;
            --error: #FF5252;
            --warning: #FFB300;
            --di-active: #00E676;
            --do-active: #FF9100;
            --inactive: #2D3A47;
            --border: #2D3A47;
        }
        
        * { box-sizing: border-box; margin: 0; padding: 0; }
        
        body {
            font-family: 'Segoe UI', Roboto, sans-serif;
            background: var(--bg-dark);
            color: var(--text-primary);
            min-height: 100vh;
        }
        
        .container {
            display: flex;
            min-height: 100vh;
        }
        
        /* Sidebar */
        .sidebar {
            width: 220px;
            background: #0D1218;
            padding: 20px 0;
            display: flex;
            flex-direction: column;
        }
        
        .logo {
            text-align: center;
            padding: 20px;
        }
        
        .logo-icon { font-size: 40px; }
        .logo-text { font-size: 18px; font-weight: bold; letter-spacing: 3px; margin-top: 8px; }
        
        .nav-item {
            display: flex;
            align-items: center;
            padding: 14px 20px;
            cursor: pointer;
            border-left: 3px solid transparent;
            transition: all 0.2s;
        }
        
        .nav-item:hover { background: var(--bg-card); }
        .nav-item.active { border-left-color: var(--primary); background: var(--bg-hover); }
        .nav-item.active .nav-icon { color: var(--primary); }
        .nav-item.active .nav-text { color: var(--text-primary); }
        
        .nav-icon { font-size: 18px; margin-right: 12px; color: var(--text-secondary); }
        .nav-text { color: var(--text-secondary); }
        
        .status-box {
            margin: auto 16px 16px;
            background: var(--bg-card);
            border-radius: 8px;
            padding: 12px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .status-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: var(--error);
        }
        
        .status-dot.connected { background: var(--success); animation: pulse 2s infinite; }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        /* Main content */
        .main {
            flex: 1;
            display: flex;
            flex-direction: column;
        }
        
        .header {
            height: 64px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 0 24px;
            border-bottom: 1px solid var(--border);
        }
        
        .page-title { font-size: 24px; font-weight: bold; }
        
        .header-pills {
            display: flex;
            gap: 16px;
        }
        
        .pill {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 20px;
            padding: 8px 16px;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .pill-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
        }
        
        /* Content */
        .content {
            flex: 1;
            padding: 24px;
            overflow-y: auto;
        }
        
        .page { display: none; }
        .page.active { display: block; }
        
        /* Cards */
        .card {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
        }
        
        .card-header {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 16px;
        }
        
        .card-icon { font-size: 18px; color: var(--primary); }
        .card-title { font-size: 16px; font-weight: 600; }
        
        /* Grid */
        .dio-grid {
            display: grid;
            grid-template-columns: repeat(8, 1fr);
            gap: 6px;
            max-width: 550px;
            margin: 20px auto;
        }
        
        .dio-channel {
            aspect-ratio: 1;
            background: var(--inactive);
            border: 2px solid var(--border);
            border-radius: 8px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            transition: all 0.2s;
            font-family: monospace;
        }
        
        .dio-channel:hover { background: var(--bg-hover); }
        .dio-channel.active.di { background: var(--di-active); }
        .dio-channel.active.do { background: var(--do-active); }
        .dio-channel.active { color: #000; border-color: transparent; }
        
        .channel-num { font-size: 16px; font-weight: bold; }
        .channel-state { font-size: 10px; opacity: 0.8; }
        
        /* Buttons */
        .btn {
            background: var(--primary);
            color: #000;
            border: none;
            padding: 12px 24px;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
        }
        
        .btn:hover { background: var(--primary-dark); }
        .btn:disabled { opacity: 0.5; cursor: not-allowed; }
        
        .btn.outline {
            background: transparent;
            border: 2px solid var(--primary);
            color: var(--primary);
        }
        
        .btn.danger { background: var(--error); }
        .btn.danger:hover { background: #FF6B6B; }
        
        .btn-group {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
        }
        
        /* Stats */
        .stats-row {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 20px;
        }
        
        .stat-value {
            font-size: 48px;
            font-weight: bold;
        }
        
        .stat-value.di { color: var(--di-active); }
        .stat-value.do { color: var(--do-active); }
        
        /* Forms */
        .form-row {
            display: flex;
            align-items: center;
            gap: 16px;
            margin-bottom: 16px;
        }
        
        .form-label { color: var(--text-secondary); min-width: 100px; }
        
        select, input {
            background: var(--bg-hover);
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 10px 16px;
            color: var(--text-primary);
            font-size: 14px;
        }
        
        select:focus, input:focus {
            outline: none;
            border-color: var(--primary);
        }
        
        /* Legend */
        .legend {
            display: flex;
            gap: 24px;
            margin-bottom: 16px;
        }
        
        .legend-item {
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .legend-box {
            width: 20px;
            height: 20px;
            border-radius: 4px;
        }
        
        /* Mobile */
        @media (max-width: 768px) {
            .sidebar { width: 60px; }
            .logo-text, .nav-text { display: none; }
            .nav-item { justify-content: center; padding: 14px; }
            .nav-icon { margin-right: 0; }
            .status-box { display: none; }
            .stats-row { grid-template-columns: 1fr; }
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- Sidebar -->
        <nav class="sidebar">
            <div class="logo">
                <div class="logo-icon">⚡</div>
                <div class="logo-text">ENERSION</div>
            </div>
            
            <div class="nav-item active" onclick="showPage(0)">
                <span class="nav-icon">⌂</span>
                <span class="nav-text">Dashboard</span>
            </div>
            <div class="nav-item" onclick="showPage(1)">
                <span class="nav-icon">▣</span>
                <span class="nav-text">Digital Inputs</span>
            </div>
            <div class="nav-item" onclick="showPage(2)">
                <span class="nav-icon">◧</span>
                <span class="nav-text">Digital Outputs</span>
            </div>
            <div class="nav-item" onclick="showPage(3)">
                <span class="nav-icon">⚙</span>
                <span class="nav-text">Settings</span>
            </div>
            
            <div class="status-box">
                <div class="status-dot" id="statusDot"></div>
                <div>
                    <div style="font-size: 12px; font-weight: 600;" id="statusText">Disconnected</div>
                    <div style="font-size: 10px; color: var(--text-muted);" id="statusPort">No device</div>
                </div>
            </div>
        </nav>
        
        <!-- Main -->
        <main class="main">
            <header class="header">
                <h1 class="page-title" id="pageTitle">Dashboard</h1>
                <div class="header-pills">
                    <div class="pill">
                        <div class="pill-dot" style="background: var(--di-active)"></div>
                        <span>DI</span>
                        <strong id="diPill">0/64</strong>
                    </div>
                    <div class="pill">
                        <div class="pill-dot" style="background: var(--do-active)"></div>
                        <span>DO</span>
                        <strong id="doPill">0/64</strong>
                    </div>
                    <div class="pill" id="timePill">00:00:00</div>
                </div>
            </header>
            
            <div class="content">
                <!-- Dashboard -->
                <div class="page active" id="page0">
                    <h2 style="font-size: 36px; margin-bottom: 8px;">Welcome to Enersion</h2>
                    <p style="color: var(--text-secondary); margin-bottom: 24px;">Industrial Control System for Digital I/O Management</p>
                    
                    <div class="stats-row">
                        <div class="card">
                            <div class="card-header">
                                <span class="card-icon">▣</span>
                                <span class="card-title">Digital Inputs</span>
                            </div>
                            <div style="display: flex; align-items: baseline;">
                                <span class="stat-value di" id="diCount">0</span>
                                <span style="font-size: 24px; color: var(--text-muted);">/ 64</span>
                            </div>
                        </div>
                        <div class="card">
                            <div class="card-header">
                                <span class="card-icon">◧</span>
                                <span class="card-title">Digital Outputs</span>
                            </div>
                            <div style="display: flex; align-items: baseline;">
                                <span class="stat-value do" id="doCount">0</span>
                                <span style="font-size: 24px; color: var(--text-muted);">/ 64</span>
                            </div>
                        </div>
                    </div>
                    
                    <div class="card">
                        <div class="card-header">
                            <span class="card-icon">⚡</span>
                            <span class="card-title">Quick Actions</span>
                        </div>
                        <div class="btn-group">
                            <button class="btn" onclick="showPage(1)">▣ View Inputs</button>
                            <button class="btn" onclick="showPage(2)">◧ Control Outputs</button>
                            <button class="btn outline" onclick="setAllOutputs(false)">○ All OFF</button>
                            <button class="btn outline" onclick="setAllOutputs(true)">● All ON</button>
                        </div>
                    </div>
                </div>
                
                <!-- Digital Inputs -->
                <div class="page" id="page1">
                    <div class="legend">
                        <div class="legend-item">
                            <div class="legend-box" style="background: var(--di-active);"></div>
                            <span>Input HIGH (Active)</span>
                        </div>
                        <div class="legend-item">
                            <div class="legend-box" style="background: var(--inactive);"></div>
                            <span>Input LOW (Inactive)</span>
                        </div>
                    </div>
                    
                    <div class="card">
                        <div class="dio-grid" id="diGrid"></div>
                    </div>
                </div>
                
                <!-- Digital Outputs -->
                <div class="page" id="page2">
                    <div class="card" style="margin-bottom: 16px;">
                        <div class="btn-group">
                            <span style="color: var(--text-secondary); margin-right: 8px;">Quick Actions:</span>
                            <button class="btn" onclick="setAllOutputs(true)">● All ON</button>
                            <button class="btn danger" onclick="setAllOutputs(false)">○ All OFF</button>
                            <button class="btn outline" onclick="setPattern(0xAA)">Alternate</button>
                            <button class="btn outline" onclick="setPattern(0x0F)">First Half</button>
                        </div>
                    </div>
                    
                    <div class="legend">
                        <div class="legend-item">
                            <div class="legend-box" style="background: var(--do-active);"></div>
                            <span>Output ON</span>
                        </div>
                        <div class="legend-item">
                            <div class="legend-box" style="background: var(--inactive);"></div>
                            <span>Output OFF</span>
                        </div>
                        <span style="color: var(--text-muted);">💡 Click any channel to toggle</span>
                    </div>
                    
                    <div class="card">
                        <div class="dio-grid" id="doGrid"></div>
                    </div>
                </div>
                
                <!-- Settings -->
                <div class="page" id="page3">
                    <div class="card">
                        <div class="card-header">
                            <span class="card-icon">🔌</span>
                            <span class="card-title">Connection Settings</span>
                        </div>
                        
                        <div class="form-row">
                            <span class="form-label">Serial Port:</span>
                            <select id="portSelect" style="width: 250px;">
                                <option>/dev/ttySTM9</option>
                            </select>
                            
                            <span class="form-label">Baud Rate:</span>
                            <select id="baudSelect">
                                <option>9600</option>
                                <option>19200</option>
                                <option>38400</option>
                                <option>57600</option>
                                <option selected>115200</option>
                                <option>230400</option>
                            </select>
                        </div>
                        
                        <div class="btn-group">
                            <button class="btn" id="connectBtn" onclick="toggleConnect()">Connect</button>
                            <button class="btn outline" onclick="refreshPorts()">⟳ Refresh Ports</button>
                        </div>
                    </div>
                    
                    <div class="card">
                        <div class="card-header">
                            <span class="card-icon">ℹ</span>
                            <span class="card-title">Device Information</span>
                        </div>
                        <div style="display: grid; grid-template-columns: repeat(3, 1fr); gap: 20px;">
                            <div><div style="color: var(--text-muted); font-size: 12px;">Board</div><div>MYIR STM32MP257</div></div>
                            <div><div style="color: var(--text-muted); font-size: 12px;">RS485 Port</div><div>/dev/ttySTM9</div></div>
                            <div><div style="color: var(--text-muted); font-size: 12px;">GPIO</div><div>PI10 (138)</div></div>
                            <div><div style="color: var(--text-muted); font-size: 12px;">DI Controller</div><div>Address 0x02</div></div>
                            <div><div style="color: var(--text-muted); font-size: 12px;">DO Controller</div><div>Address 0x03</div></div>
                            <div><div style="color: var(--text-muted); font-size: 12px;">Protocol</div><div>Enersion v1.0</div></div>
                        </div>
                    </div>
                </div>
            </div>
        </main>
    </div>
    
    <script>
        const pageTitles = ['Dashboard', 'Digital Inputs', 'Digital Outputs', 'Settings'];
        let connected = false;
        
        // Initialize grids
        function initGrids() {
            const diGrid = document.getElementById('diGrid');
            const doGrid = document.getElementById('doGrid');
            
            for (let i = 0; i < 64; i++) {
                // DI channel
                const diCh = document.createElement('div');
                diCh.className = 'dio-channel di';
                diCh.id = `di-${i}`;
                diCh.innerHTML = `<span class="channel-num">${String(i+1).padStart(2,'0')}</span><span class="channel-state">OFF</span>`;
                diGrid.appendChild(diCh);
                
                // DO channel
                const doCh = document.createElement('div');
                doCh.className = 'dio-channel do';
                doCh.id = `do-${i}`;
                doCh.innerHTML = `<span class="channel-num">${String(i+1).padStart(2,'0')}</span><span class="channel-state">OFF</span>`;
                doCh.onclick = () => toggleOutput(i);
                doGrid.appendChild(doCh);
            }
        }
        
        // Show page
        function showPage(index) {
            document.querySelectorAll('.page').forEach((p, i) => {
                p.classList.toggle('active', i === index);
            });
            document.querySelectorAll('.nav-item').forEach((n, i) => {
                n.classList.toggle('active', i === index);
            });
            document.getElementById('pageTitle').textContent = pageTitles[index];
        }
        
        // Update state
        function updateState(state) {
            connected = state.connected;
            
            // Status
            const dot = document.getElementById('statusDot');
            const text = document.getElementById('statusText');
            const port = document.getElementById('statusPort');
            const btn = document.getElementById('connectBtn');
            
            dot.classList.toggle('connected', connected);
            text.textContent = connected ? 'Connected' : 'Disconnected';
            port.textContent = connected ? state.port : 'No device';
            btn.textContent = connected ? 'Disconnect' : 'Connect';
            btn.classList.toggle('danger', connected);
            
            // Counts
            document.getElementById('diCount').textContent = state.di_count;
            document.getElementById('doCount').textContent = state.do_count;
            document.getElementById('diPill').textContent = `${state.di_count}/64`;
            document.getElementById('doPill').textContent = `${state.do_count}/64`;
            
            // DI Grid
            state.di_states.forEach((active, i) => {
                const el = document.getElementById(`di-${i}`);
                el.classList.toggle('active', active);
                el.querySelector('.channel-state').textContent = active ? 'ON' : 'OFF';
            });
            
            // DO Grid
            state.do_states.forEach((active, i) => {
                const el = document.getElementById(`do-${i}`);
                el.classList.toggle('active', active);
                el.querySelector('.channel-state').textContent = active ? 'ON' : 'OFF';
            });
        }
        
        // API calls
        async function fetchState() {
            try {
                const res = await fetch('/api/state');
                const state = await res.json();
                updateState(state);
            } catch (e) {
                console.error('Fetch error:', e);
            }
        }
        
        async function toggleConnect() {
            const port = document.getElementById('portSelect').value;
            const baudrate = parseInt(document.getElementById('baudSelect').value);
            
            if (connected) {
                await fetch('/api/disconnect', { method: 'POST' });
            } else {
                await fetch('/api/connect', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ port, baudrate })
                });
            }
            fetchState();
        }
        
        async function toggleOutput(channel) {
            await fetch('/api/toggle', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ channel })
            });
        }
        
        async function setAllOutputs(state) {
            await fetch('/api/set_all', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ state })
            });
        }
        
        async function setPattern(pattern) {
            await fetch('/api/set_pattern', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ pattern })
            });
        }
        
        async function refreshPorts() {
            const res = await fetch('/api/ports');
            const data = await res.json();
            const select = document.getElementById('portSelect');
            select.innerHTML = data.ports.map(p => `<option>${p}</option>`).join('');
        }
        
        // Time
        function updateTime() {
            const now = new Date();
            document.getElementById('timePill').textContent = now.toLocaleTimeString();
        }
        
        // Init
        initGrids();
        refreshPorts();
        fetchState();
        updateTime();
        
        setInterval(fetchState, 200);
        setInterval(updateTime, 1000);
    </script>
</body>
</html>'''


class ThreadedHTTPServer(socketserver.ThreadingMixIn, HTTPServer):
    """Threaded HTTP server"""
    allow_reuse_address = True


def main():
    """Main entry point"""
    port = 8080
    
    print("=" * 50)
    print(" Enersion Control System - Web UI")
    print("=" * 50)
    print()
    print(f" Open in browser: http://localhost:{port}")
    print(" Press Ctrl+C to stop")
    print()
    
    server = ThreadedHTTPServer(('0.0.0.0', port), EnersionHandler)
    
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down...")
        server.shutdown()


if __name__ == "__main__":
    main()

