# Enersion Enterprise Software Documentation

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Python Web Application](#python-web-application)
4. [Qt/QML Application](#qtqml-application)
5. [C Test Application](#c-test-application)
6. [Windows Test GUIs](#windows-test-guis)
7. [Deployment](#deployment)
8. [API Reference](#api-reference)

---

## Overview

The Enersion software runs on the MYIR STM32MP257 Linux board and provides:

- **Web-based HMI** - Touch-friendly interface via browser
- **Qt/QML GUI** - Native desktop application
- **CLI Tools** - Command-line testing and automation
- **REST API** - Integration with external systems

### Software Options

| Application | Technology | Status | Best For |
|-------------|------------|--------|----------|
| **Web App** | Python + Flask | ✅ Ready | Touchscreen HMI |
| **Qt GUI** | C++ + QML | ✅ Code Ready | Native performance |
| **CLI Test** | C | ✅ Ready | Testing/Debugging |

---

## Architecture

### Layered Design

```
┌─────────────────────────────────────────────────────────────┐
│                      Presentation Layer                      │
│  ┌───────────────┐  ┌───────────────┐  ┌────────────────┐  │
│  │  Web GUI      │  │  Qt/QML GUI   │  │  CLI App       │  │
│  │  (HTML/CSS/JS)│  │  (QML)        │  │  (Terminal)    │  │
│  └───────┬───────┘  └───────┬───────┘  └───────┬────────┘  │
├──────────┴──────────────────┴──────────────────┴────────────┤
│                       Service Layer                          │
│  ┌────────────────┐  ┌────────────────┐  ┌──────────────┐   │
│  │  DI Service    │  │  DO Service    │  │  ANA Service │   │
│  │  - Read states │  │  - Write outs  │  │  - Read ADC  │   │
│  │  - Monitoring  │  │  - PWM control │  │  - Scaling   │   │
│  └────────┬───────┘  └────────┬───────┘  └──────┬───────┘   │
├───────────┴───────────────────┴─────────────────┴───────────┤
│                      Protocol Layer                          │
│  ┌──────────────────────────────────────────────────────┐   │
│  │               Enersion Protocol                       │   │
│  │  - Packet encoding/decoding                          │   │
│  │  - CRC-16 calculation                                │   │
│  │  - Command/Response handling                         │   │
│  └──────────────────────────┬───────────────────────────┘   │
├─────────────────────────────┴───────────────────────────────┤
│                         HAL Layer                            │
│  ┌────────────────────┐  ┌─────────────────────────────┐    │
│  │  RS485 Serial      │  │  GPIO Direction Control     │    │
│  │  /dev/ttySTM9      │  │  /sys/class/gpio/gpio138    │    │
│  │  115200 8N1        │  │  TX=HIGH, RX=LOW            │    │
│  └────────────────────┘  └─────────────────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│                     Linux Kernel                             │
│  ┌────────────────────┐  ┌─────────────────────────────┐    │
│  │  UART Driver       │  │  GPIO Sysfs                 │    │
│  └────────────────────┘  └─────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

## Python Web Application

**Location:** `LinuxApp/python/`

### File Structure

```
LinuxApp/python/
├── web_app.py              # Main Flask application
├── requirements.txt        # Python dependencies
├── run.sh                  # Linux run script
├── run.bat                 # Windows run script
├── README.md               # Documentation
│
├── hal/
│   ├── __init__.py
│   └── rs485.py            # RS485 HAL implementation
│
├── protocol/
│   ├── __init__.py
│   └── enersion.py         # Protocol implementation
│
├── gui/
│   ├── __init__.py
│   ├── theme.py            # UI theme definitions
│   └── widgets.py          # Custom widgets (if using tkinter)
│
└── templates/              # HTML templates (if using Flask)
    └── index.html
```

### RS485 HAL Module

```python
# hal/rs485.py

import serial
import os
import time

class RS485:
    """RS485 communication with GPIO direction control"""
    
    # Default configuration
    DEFAULT_PORT = "/dev/ttySTM9"
    DEFAULT_BAUD = 115200
    GPIO_PIN = "138"  # PI10
    
    def __init__(self, port=None, baudrate=None):
        self.port = port or self.DEFAULT_PORT
        self.baudrate = baudrate or self.DEFAULT_BAUD
        self.serial = None
        self.gpio_path = f"/sys/class/gpio/gpio{self.GPIO_PIN}"
        
    def open(self):
        """Initialize RS485 interface"""
        # Export GPIO if needed
        self._gpio_init()
        
        # Open serial port
        self.serial = serial.Serial(
            port=self.port,
            baudrate=self.baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.5
        )
        
        # Start in receive mode
        self._set_rx_mode()
        
    def close(self):
        """Close RS485 interface"""
        if self.serial:
            self._set_rx_mode()
            self.serial.close()
            
    def write(self, data):
        """Write data to RS485 bus"""
        self._set_tx_mode()
        time.sleep(0.001)  # Allow direction switch
        self.serial.write(data)
        self.serial.flush()
        time.sleep(0.001)  # Wait for transmission
        self._set_rx_mode()
        
    def read(self, size=256, timeout=0.5):
        """Read data from RS485 bus"""
        self._set_rx_mode()
        self.serial.timeout = timeout
        return self.serial.read(size)
        
    def _gpio_init(self):
        """Initialize GPIO for direction control"""
        if not os.path.exists(self.gpio_path):
            with open("/sys/class/gpio/export", "w") as f:
                f.write(self.GPIO_PIN)
            time.sleep(0.1)
        
        with open(f"{self.gpio_path}/direction", "w") as f:
            f.write("out")
            
    def _set_tx_mode(self):
        """Enable transmit mode"""
        with open(f"{self.gpio_path}/value", "w") as f:
            f.write("1")
            
    def _set_rx_mode(self):
        """Enable receive mode"""
        with open(f"{self.gpio_path}/value", "w") as f:
            f.write("0")
```

### Protocol Module

```python
# protocol/enersion.py

import struct

class EnersionProtocol:
    """Enersion communication protocol"""
    
    # Packet markers
    START_BYTE = 0xAA
    END_BYTE = 0x55
    
    # Device addresses
    ADDR_BROADCAST = 0x00
    ADDR_CTRL_420 = 0x01
    ADDR_CTRL_DIO = 0x02
    ADDR_CTRL_OUT = 0x03
    ADDR_MASTER = 0x10
    
    # Commands
    CMD_PING = 0x01
    CMD_PING_RESP = 0x02
    CMD_GET_VERSION = 0x03
    CMD_VERSION_RESP = 0x04
    CMD_GET_STATUS = 0x10
    CMD_READ_DI = 0x20
    CMD_DI_RESP = 0x21
    CMD_WRITE_DO = 0x30
    CMD_DO_RESP = 0x31
    CMD_READ_DO = 0x32
    
    def __init__(self, rs485):
        self.rs485 = rs485
        self.source_addr = self.ADDR_MASTER
        
    def build_packet(self, dest, cmd, data=None):
        """Build a complete Enersion packet"""
        if data is None:
            data = b''
            
        # Build payload
        payload = bytes([dest, self.source_addr, cmd, len(data)]) + data
        
        # Calculate CRC
        crc = self.calc_crc16(payload)
        
        # Complete packet
        packet = bytes([self.START_BYTE]) + payload + \
                 struct.pack('<H', crc) + bytes([self.END_BYTE])
        
        return packet
        
    def parse_packet(self, data):
        """Parse received packet"""
        if len(data) < 8:
            return None
            
        if data[0] != self.START_BYTE or data[-1] != self.END_BYTE:
            return None
            
        dest = data[1]
        src = data[2]
        cmd = data[3]
        length = data[4]
        
        if len(data) < 8 + length:
            return None
            
        payload_data = data[5:5+length]
        crc_received = struct.unpack('<H', data[5+length:7+length])[0]
        
        # Verify CRC
        crc_calc = self.calc_crc16(data[1:5+length])
        if crc_calc != crc_received:
            return None
            
        return {
            'dest': dest,
            'src': src,
            'cmd': cmd,
            'data': payload_data
        }
        
    def calc_crc16(self, data):
        """Calculate CRC-16 (Modbus polynomial)"""
        crc = 0xFFFF
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 0x0001:
                    crc = (crc >> 1) ^ 0xA001
                else:
                    crc >>= 1
        return crc
        
    def ping(self, device_addr):
        """Send ping command"""
        packet = self.build_packet(device_addr, self.CMD_PING)
        self.rs485.write(packet)
        
        response = self.rs485.read(16)
        parsed = self.parse_packet(response)
        
        return parsed is not None and parsed['cmd'] == self.CMD_PING_RESP
        
    def read_di(self):
        """Read all 64 digital inputs"""
        packet = self.build_packet(self.ADDR_CTRL_DIO, self.CMD_READ_DI)
        self.rs485.write(packet)
        
        response = self.rs485.read(32)
        parsed = self.parse_packet(response)
        
        if parsed and parsed['cmd'] == self.CMD_DI_RESP:
            return list(parsed['data'])
        return None
        
    def write_do(self, states):
        """Write all 64 digital outputs (8 bytes)"""
        if len(states) != 8:
            raise ValueError("States must be 8 bytes")
            
        packet = self.build_packet(self.ADDR_CTRL_OUT, self.CMD_WRITE_DO, 
                                   bytes(states))
        self.rs485.write(packet)
        
        response = self.rs485.read(16)
        parsed = self.parse_packet(response)
        
        return parsed is not None and parsed['cmd'] == self.CMD_DO_RESP
        
    def read_do(self):
        """Read current digital output states"""
        packet = self.build_packet(self.ADDR_CTRL_OUT, self.CMD_READ_DO)
        self.rs485.write(packet)
        
        response = self.rs485.read(32)
        parsed = self.parse_packet(response)
        
        if parsed and parsed['cmd'] == 0x33:  # DO_READ_RESP
            return list(parsed['data'])
        return None
```

### Web Application (Flask)

```python
# web_app.py

from flask import Flask, render_template, jsonify, request
import threading
import time

app = Flask(__name__)

# Global state
di_states = [0] * 8  # 64 bits = 8 bytes
do_states = [0] * 8
connection_status = {"di": False, "do": False}

# RS485 communication thread
def comm_thread():
    global di_states, do_states, connection_status
    
    from hal.rs485 import RS485
    from protocol.enersion import EnersionProtocol
    
    rs485 = RS485()
    protocol = EnersionProtocol(rs485)
    
    try:
        rs485.open()
        
        while True:
            # Ping and read DI
            try:
                if protocol.ping(protocol.ADDR_CTRL_DIO):
                    connection_status["di"] = True
                    result = protocol.read_di()
                    if result:
                        di_states = result
                else:
                    connection_status["di"] = False
            except Exception:
                connection_status["di"] = False
                
            # Ping and read DO
            try:
                if protocol.ping(protocol.ADDR_CTRL_OUT):
                    connection_status["do"] = True
                    result = protocol.read_do()
                    if result:
                        do_states = result
                else:
                    connection_status["do"] = False
            except Exception:
                connection_status["do"] = False
                
            time.sleep(0.1)  # 100ms update rate
            
    except Exception as e:
        print(f"Communication error: {e}")
    finally:
        rs485.close()

# Routes
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/status')
def get_status():
    return jsonify({
        "di": {
            "connected": connection_status["di"],
            "states": di_states
        },
        "do": {
            "connected": connection_status["do"],
            "states": do_states
        }
    })

@app.route('/api/di')
def get_di():
    # Expand bytes to individual bits
    bits = []
    for byte in di_states:
        for i in range(8):
            bits.append((byte >> i) & 1)
    return jsonify({"channels": bits})

@app.route('/api/do', methods=['GET'])
def get_do():
    bits = []
    for byte in do_states:
        for i in range(8):
            bits.append((byte >> i) & 1)
    return jsonify({"channels": bits})

@app.route('/api/do', methods=['POST'])
def set_do():
    global do_states
    data = request.json
    
    if 'channel' in data and 'state' in data:
        # Set single channel
        channel = data['channel']
        state = data['state']
        
        byte_idx = channel // 8
        bit_idx = channel % 8
        
        if state:
            do_states[byte_idx] |= (1 << bit_idx)
        else:
            do_states[byte_idx] &= ~(1 << bit_idx)
            
        # Send to device
        # protocol.write_do(do_states)
        
    elif 'states' in data:
        # Set all channels
        do_states = data['states']
        # protocol.write_do(do_states)
        
    return jsonify({"success": True})

if __name__ == '__main__':
    # Start communication thread
    comm = threading.Thread(target=comm_thread, daemon=True)
    comm.start()
    
    # Start web server
    app.run(host='0.0.0.0', port=8080, debug=False)
```

### HTML Template

```html
<!-- templates/index.html -->
<!DOCTYPE html>
<html>
<head>
    <title>Enersion Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        :root {
            --bg-primary: #0a0f1a;
            --bg-secondary: #1a2332;
            --accent: #00d4ff;
            --success: #00ff88;
            --danger: #ff4444;
            --text: #ffffff;
        }
        
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body {
            font-family: 'Segoe UI', system-ui, sans-serif;
            background: var(--bg-primary);
            color: var(--text);
            min-height: 100vh;
        }
        
        .header {
            background: linear-gradient(135deg, var(--bg-secondary), #2a3a52);
            padding: 1rem 2rem;
            border-bottom: 2px solid var(--accent);
        }
        
        .header h1 {
            font-size: 1.5rem;
            font-weight: 300;
            letter-spacing: 2px;
        }
        
        .container {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 1.5rem;
            padding: 1.5rem;
            max-width: 1400px;
            margin: 0 auto;
        }
        
        .card {
            background: var(--bg-secondary);
            border-radius: 12px;
            padding: 1.5rem;
            border: 1px solid rgba(255,255,255,0.1);
        }
        
        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1rem;
            padding-bottom: 0.5rem;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        .status-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            animation: pulse 2s infinite;
        }
        
        .status-dot.online { background: var(--success); }
        .status-dot.offline { background: var(--danger); }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        .channel-grid {
            display: grid;
            grid-template-columns: repeat(8, 1fr);
            gap: 8px;
        }
        
        .channel {
            aspect-ratio: 1;
            border-radius: 8px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            font-size: 0.75rem;
            cursor: pointer;
            transition: all 0.2s;
            border: 2px solid transparent;
        }
        
        .channel.di { 
            background: rgba(0,212,255,0.1);
            pointer-events: none;
        }
        .channel.di.on { 
            background: var(--accent);
            color: var(--bg-primary);
        }
        
        .channel.do { background: rgba(0,255,136,0.1); }
        .channel.do.on { 
            background: var(--success);
            color: var(--bg-primary);
        }
        .channel.do:hover {
            border-color: var(--success);
            transform: scale(1.05);
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>⚡ ENERSION CONTROL SYSTEM</h1>
    </div>
    
    <div class="container">
        <div class="card">
            <div class="card-header">
                <h2>Digital Inputs (64 Channels)</h2>
                <div class="status-dot" id="di-status"></div>
            </div>
            <div class="channel-grid" id="di-grid"></div>
        </div>
        
        <div class="card">
            <div class="card-header">
                <h2>Digital Outputs (64 Channels)</h2>
                <div class="status-dot" id="do-status"></div>
            </div>
            <div class="channel-grid" id="do-grid"></div>
        </div>
    </div>
    
    <script>
        // Initialize grids
        const diGrid = document.getElementById('di-grid');
        const doGrid = document.getElementById('do-grid');
        
        for (let i = 0; i < 64; i++) {
            // DI channels
            const diCh = document.createElement('div');
            diCh.className = 'channel di';
            diCh.id = `di-${i}`;
            diCh.innerHTML = `<span>${i}</span>`;
            diGrid.appendChild(diCh);
            
            // DO channels
            const doCh = document.createElement('div');
            doCh.className = 'channel do';
            doCh.id = `do-${i}`;
            doCh.innerHTML = `<span>${i}</span>`;
            doCh.onclick = () => toggleDO(i);
            doGrid.appendChild(doCh);
        }
        
        // Toggle DO channel
        function toggleDO(channel) {
            const el = document.getElementById(`do-${channel}`);
            const newState = !el.classList.contains('on');
            
            fetch('/api/do', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({channel: channel, state: newState ? 1 : 0})
            });
            
            el.classList.toggle('on', newState);
        }
        
        // Poll status
        function updateStatus() {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    // Update connection status
                    document.getElementById('di-status').className = 
                        'status-dot ' + (data.di.connected ? 'online' : 'offline');
                    document.getElementById('do-status').className = 
                        'status-dot ' + (data.do.connected ? 'online' : 'offline');
                    
                    // Update DI states
                    for (let i = 0; i < 64; i++) {
                        const byteIdx = Math.floor(i / 8);
                        const bitIdx = i % 8;
                        const state = (data.di.states[byteIdx] >> bitIdx) & 1;
                        document.getElementById(`di-${i}`).classList.toggle('on', state);
                    }
                    
                    // Update DO states
                    for (let i = 0; i < 64; i++) {
                        const byteIdx = Math.floor(i / 8);
                        const bitIdx = i % 8;
                        const state = (data.do.states[byteIdx] >> bitIdx) & 1;
                        document.getElementById(`do-${i}`).classList.toggle('on', state);
                    }
                });
        }
        
        setInterval(updateStatus, 100);
        updateStatus();
    </script>
</body>
</html>
```

### Running the Web Application

```bash
# On MYIR board
cd /home/root/enersion/python

# Install dependencies
pip3 install flask pyserial

# Run
python3 web_app.py

# Access in browser: http://192.168.0.10:8080
```

---

## Qt/QML Application

**Location:** `LinuxApp/src/`

### Architecture

```
src/
├── hal/
│   ├── rs485_serial.c          # RS485 HAL
│   └── rs485_gpio.c            # GPIO control
├── protocol/
│   ├── enersion_protocol.c     # Protocol implementation
│   └── enersion_crc.c          # CRC calculation
├── service/
│   ├── di_service.cpp          # DI business logic
│   └── do_service.cpp          # DO business logic
├── app/
│   ├── main.cpp                # Entry point
│   ├── app_controller.cpp      # Qt controller
│   └── models/
│       ├── digital_input_model.cpp
│       └── digital_output_model.cpp
└── ui/qml/
    ├── main.qml                # Main window
    ├── Style.qml               # Theme
    └── pages/
        ├── DashboardPage.qml
        ├── DigitalInputPage.qml
        └── DigitalOutputPage.qml
```

### Building Qt Application

```bash
# On MYIR board (Qt5 is pre-installed)
cd /home/root/enersion

# Create build directory
mkdir build && cd build

# Configure
qmake ../CMakeLists.txt
# Or use cmake if available
cmake ..

# Build
make -j4

# Run
./enersion_gui
```

---

## C Test Application

**Location:** `LinuxApp/src/test/`

### Usage

```bash
# Build
cd LinuxApp/src/test
make

# Run
./enersion_test

# Commands
./enersion_test ping di      # Ping DI controller
./enersion_test ping do      # Ping DO controller
./enersion_test read di      # Read DI states
./enersion_test write do AA  # Write pattern to DO
```

---

## Windows Test GUIs

Located in root directory for development testing:

- `GUI_Application_DI/` - Digital Input testing
- `GUI_Application_DO/` - Digital Output testing
- `GUI_Application_ANA/` - Analog Input testing

### Running

```bash
cd GUI_Application_DI
python main_gui.py
```

---

## Deployment

### Upload to MYIR Board

```bash
# From Windows
scp -r LinuxApp/python root@192.168.0.10:/home/root/enersion/

# Or using FileZilla/WinSCP
# Host: 192.168.0.10
# User: root
# Pass: 123
```

### Auto-Start Service

```bash
# Create systemd service
cat > /etc/systemd/system/enersion.service << 'EOF'
[Unit]
Description=Enersion Control System
After=network.target

[Service]
Type=simple
User=root
WorkingDirectory=/home/root/enersion/python
ExecStart=/usr/bin/python3 web_app.py
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

# Enable service
systemctl enable enersion
systemctl start enersion
```

---

## API Reference

### REST Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | Web interface |
| GET | `/api/status` | System status |
| GET | `/api/di` | Get DI states |
| GET | `/api/do` | Get DO states |
| POST | `/api/do` | Set DO states |

### GET /api/status

```json
{
  "di": {
    "connected": true,
    "states": [255, 0, 128, 64, 32, 16, 8, 4]
  },
  "do": {
    "connected": true,
    "states": [0, 0, 0, 0, 0, 0, 0, 0]
  }
}
```

### GET /api/di

```json
{
  "channels": [1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, ...]
}
```

### POST /api/do

**Set single channel:**
```json
{
  "channel": 5,
  "state": 1
}
```

**Set all channels:**
```json
{
  "states": [255, 0, 128, 64, 32, 16, 8, 4]
}
```

---

*Document Version: 1.0*  
*Last Updated: December 2024*

