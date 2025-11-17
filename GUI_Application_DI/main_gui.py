"""
******************************************************************************
@file           : main_gui.py
@brief          : Digital Input Monitor GUI Application
******************************************************************************
@attention

Main application for monitoring Digital Inputs:
- RS485 communication
- 56 Digital Input channels
- Real-time monitoring
- Health monitoring

All code and comments in English language

******************************************************************************
"""

import sys
import os
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from rs485_protocol import *
from version import *
import serial.tools.list_ports

class HealthMonitorWorker(QObject):
    """Worker thread for health monitoring - Digital IN Controller Only"""
    
    health_updated = pyqtSignal(int, dict)  # mcu_id, health_data
    connection_lost = pyqtSignal(int)  # mcu_id
    
    def __init__(self, protocol: RS485Protocol):
        super().__init__()
        self.protocol = protocol
        self.running = False
        self.mcu_addresses = [
            RS485_ADDR_CONTROLLER_DIO  # Only monitor DIO controller (Digital Inputs)
        ]
    
    def start_monitoring(self):
        """Start monitoring"""
        self.running = True
        self.monitor()
    
    def stop_monitoring(self):
        """Stop monitoring"""
        self.running = False
    
    @pyqtSlot()
    def monitor(self):
        """Monitor MCU health"""
        while self.running:
            for mcu_addr in self.mcu_addresses:
                try:
                    # Send heartbeat
                    result = self.protocol.heartbeat(mcu_addr)
                    
                    if result:
                        mcu_id, health = result
                        
                        # Get detailed status
                        status = self.protocol.get_status(mcu_addr)
                        
                        health_data = {
                            'health': health,
                            'connected': True,
                            'status': status
                        }
                        
                        self.health_updated.emit(mcu_addr, health_data)
                    else:
                        self.connection_lost.emit(mcu_addr)
                        
                except Exception as e:
                    print(f"Health monitor error for MCU {mcu_addr}: {e}")
                    self.connection_lost.emit(mcu_addr)
            
            # Sleep before next check
            QThread.msleep(2000)  # Check every 2 seconds

class MCUWidget(QGroupBox):
    """Widget to display MCU status"""
    
    def __init__(self, mcu_name: str, mcu_addr: int):
        super().__init__(mcu_name)
        self.mcu_addr = mcu_addr
        self.connected = False
        
        self.init_ui()
    
    def init_ui(self):
        """Initialize UI"""
        layout = QVBoxLayout()
        
        # Connection status
        status_layout = QHBoxLayout()
        status_layout.addWidget(QLabel("Status:"))
        
        self.status_indicator = QLabel("● Disconnected")
        self.status_indicator.setStyleSheet("color: red; font-weight: bold;")
        status_layout.addWidget(self.status_indicator)
        status_layout.addStretch()
        layout.addLayout(status_layout)
        
        # Health status
        health_layout = QHBoxLayout()
        health_layout.addWidget(QLabel("Health:"))
        
        self.health_label = QLabel("---")
        health_layout.addWidget(self.health_label)
        health_layout.addStretch()
        layout.addLayout(health_layout)
        
        # Version info
        version_layout = QHBoxLayout()
        version_layout.addWidget(QLabel("Version:"))
        
        self.version_label = QLabel("Unknown")
        version_layout.addWidget(self.version_label)
        version_layout.addStretch()
        layout.addLayout(version_layout)
        
        # Packet statistics
        stats_layout = QHBoxLayout()
        stats_layout.addWidget(QLabel("Packets:"))
        
        self.stats_label = QLabel("TX: 0 | RX: 0 | Err: 0")
        stats_layout.addWidget(self.stats_label)
        stats_layout.addStretch()
        layout.addLayout(stats_layout)
        
        self.setLayout(layout)
        self.setMaximumHeight(150)
    
    def update_status(self, connected: bool, health_data: dict = None):
        """Update MCU status"""
        self.connected = connected
        
        if connected:
            self.status_indicator.setText("● Connected")
            self.status_indicator.setStyleSheet("color: green; font-weight: bold;")
            
            if health_data:
                health = health_data.get('health', 0)
                self.health_label.setText(f"{health}%")
                
                if health >= 80:
                    self.health_label.setStyleSheet("color: green; font-weight: bold;")
                elif health >= 50:
                    self.health_label.setStyleSheet("color: orange; font-weight: bold;")
                else:
                    self.health_label.setStyleSheet("color: red; font-weight: bold;")
                
                status = health_data.get('status')
                if status:
                    self.stats_label.setText(
                        f"TX: {status.get('tx_packets', 0)} | "
                        f"RX: {status.get('rx_packets', 0)} | "
                        f"Err: {status.get('errors', 0)}"
                    )
        else:
            self.status_indicator.setText("● Disconnected")
            self.status_indicator.setStyleSheet("color: red; font-weight: bold;")
            self.health_label.setText("---")
            self.stats_label.setText("TX: 0 | RX: 0 | Err: 0")
    
    def set_version(self, version_str: str):
        """Set version info"""
        self.version_label.setText(version_str)

class DigitalInputWidget(QGroupBox):
    """Widget for digital input monitoring - 56 channels"""
    
    def __init__(self, protocol: RS485Protocol):
        super().__init__("Digital Input Monitor - 56 Channels")
        self.protocol = protocol
        self.auto_refresh = False
        self.init_ui()
        
        # Auto-refresh timer
        self.refresh_timer = QTimer()
        self.refresh_timer.timeout.connect(self.read_inputs)
    
    def init_ui(self):
        """Initialize UI"""
        layout = QVBoxLayout()
        
        # Info label
        info_label = QLabel("Controller DIO (Address: 0x02) - RS485 Digital Input Monitor")
        info_label.setStyleSheet("color: #0066cc; font-weight: bold; padding: 5px;")
        layout.addWidget(info_label)
        
        # Digital Inputs
        di_group = QGroupBox("Digital Inputs (DI0 - DI55)")
        di_scroll = QScrollArea()
        di_scroll.setWidgetResizable(True)
        di_scroll.setMinimumHeight(300)
        di_widget = QWidget()
        di_layout = QGridLayout()
        di_layout.setSpacing(5)
        
        self.di_labels = []
        for i in range(56):
            label = QLabel(f"DI{i:02d}: --")
            label.setMinimumWidth(85)
            label.setMinimumHeight(25)
            label.setStyleSheet("border: 1px solid gray; padding: 3px; background-color: #f0f0f0;")
            di_layout.addWidget(label, i // 8, i % 8)
            self.di_labels.append(label)
        
        di_widget.setLayout(di_layout)
        di_scroll.setWidget(di_widget)
        di_group_layout = QVBoxLayout()
        di_group_layout.addWidget(di_scroll)
        di_group.setLayout(di_group_layout)
        layout.addWidget(di_group)
        
        # Control buttons
        button_layout = QHBoxLayout()
        
        self.read_di_btn = QPushButton("Read Inputs Now")
        self.read_di_btn.setMinimumHeight(40)
        self.read_di_btn.setStyleSheet("background-color: #2196F3; color: white; font-weight: bold;")
        self.read_di_btn.clicked.connect(self.read_inputs)
        button_layout.addWidget(self.read_di_btn)
        
        self.auto_refresh_btn = QPushButton("Start Auto-Refresh (1s)")
        self.auto_refresh_btn.setMinimumHeight(40)
        self.auto_refresh_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        self.auto_refresh_btn.clicked.connect(self.toggle_auto_refresh)
        button_layout.addWidget(self.auto_refresh_btn)
        
        layout.addLayout(button_layout)
        
        # Status info
        self.status_label = QLabel("Ready - Click 'Read Inputs Now' to poll digital inputs")
        self.status_label.setStyleSheet("padding: 5px; background-color: #f0f0f0;")
        layout.addWidget(self.status_label)
        
        self.setLayout(layout)
    
    def read_inputs(self):
        """Read digital inputs (56 channels)"""
        try:
            if not self.auto_refresh:
                self.status_label.setText("Reading input state from controller...")
            QApplication.processEvents()
            
            data = self.protocol.read_digital_inputs(RS485_ADDR_CONTROLLER_DIO)
            if data:
                # Update labels (56 inputs = 7 bytes)
                active_count = 0
                for i in range(56):
                    byte_idx = i // 8
                    bit_idx = i % 8
                    if byte_idx < len(data):
                        state = (data[byte_idx] >> bit_idx) & 0x01
                        if i < len(self.di_labels):
                            if state:
                                self.di_labels[i].setText(f"DI{i:02d}: HIGH")
                                self.di_labels[i].setStyleSheet("border: 2px solid green; padding: 3px; background-color: #d4edda; color: #155724; font-weight: bold;")
                                active_count += 1
                            else:
                                self.di_labels[i].setText(f"DI{i:02d}: LOW")
                                self.di_labels[i].setStyleSheet("border: 1px solid gray; padding: 3px; background-color: #f8f9fa; color: #6c757d;")
                
                self.status_label.setText(f"Read complete: {active_count} inputs are HIGH")
                self.status_label.setStyleSheet("padding: 5px; background-color: #d4edda; color: #155724;")
            else:
                self.status_label.setText("Error: Failed to read inputs (no response from controller)")
                self.status_label.setStyleSheet("padding: 5px; background-color: #f8d7da; color: #721c24;")
        except Exception as e:
            self.status_label.setText(f"Error: {e}")
            self.status_label.setStyleSheet("padding: 5px; background-color: #f8d7da; color: #721c24;")
    
    def toggle_auto_refresh(self):
        """Toggle auto-refresh"""
        self.auto_refresh = not self.auto_refresh
        
        if self.auto_refresh:
            self.refresh_timer.start(1000)  # Refresh every 1 second
            self.auto_refresh_btn.setText("Stop Auto-Refresh")
            self.auto_refresh_btn.setStyleSheet("background-color: #f44336; color: white; font-weight: bold;")
            self.status_label.setText("Auto-refresh enabled (1 second interval)")
            self.status_label.setStyleSheet("padding: 5px; background-color: #d1ecf1; color: #0c5460;")
        else:
            self.refresh_timer.stop()
            self.auto_refresh_btn.setText("Start Auto-Refresh (1s)")
            self.auto_refresh_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
            self.status_label.setText("Auto-refresh stopped")
        self.status_label.setStyleSheet("padding: 5px; background-color: #f0f0f0;")

class MainWindow(QMainWindow):
    """Main application window"""
    
    def __init__(self):
        super().__init__()
        self.protocol = None
        self.health_monitor_worker = None
        self.health_monitor_thread = None
        
        self.init_ui()
        self.scan_devices()
    
    def init_ui(self):
        """Initialize UI"""
        self.setWindowTitle("Digital IN Controller")
        self.setGeometry(100, 100, 900, 700)
        self.resize(800, 600)
        
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Main layout
        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)
        
        # Top panel
        top_panel = QWidget()
        top_panel.setStyleSheet("background-color: #34495e; color: white; padding: 10px;")
        top_layout = QHBoxLayout()
        top_panel.setLayout(top_layout)
        
        # Title
        title_label = QLabel(f"{VERSION_NAME} v{VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}")
        title_label.setStyleSheet("font-size: 18px; font-weight: bold; color: white;")
        top_layout.addWidget(title_label)
        
        top_layout.addStretch()
        
        # Connection controls
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(150)
        top_layout.addWidget(QLabel("COM Port:"))
        top_layout.addWidget(self.port_combo)
        
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.setMinimumWidth(100)
        self.connect_btn.clicked.connect(self.connect)
        top_layout.addWidget(self.connect_btn)
        
        self.disconnect_btn = QPushButton("Disconnect")
        self.disconnect_btn.setMinimumWidth(100)
        self.disconnect_btn.setEnabled(False)
        self.disconnect_btn.clicked.connect(self.disconnect)
        top_layout.addWidget(self.disconnect_btn)
        
        main_layout.addWidget(top_panel)
        
        # MCU Status Panel
        self.mcu_di_widget = MCUWidget("Controller DIO (Digital Inputs - 56 Channels)", RS485_ADDR_CONTROLLER_DIO)
        main_layout.addWidget(self.mcu_di_widget)
        
        self.mcu_widgets = {
            RS485_ADDR_CONTROLLER_DIO: self.mcu_di_widget
        }
        
        # Digital Input Widget
        self.digital_input_widget = None
        
        # Status bar
        self.statusBar().showMessage("Ready - Please connect to COM port")
    
    def scan_devices(self):
        """Scan available COM ports"""
        ports = serial.tools.list_ports.comports()
        self.port_combo.clear()
        
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}")
    
    def connect(self):
        """Connect to RS485"""
        if self.port_combo.currentIndex() < 0:
            QMessageBox.warning(self, "Warning", "Please select a COM port")
            return
        
        port_text = self.port_combo.currentText()
        port = port_text.split(" - ")[0]
        
        try:
            self.protocol = RS485Protocol(port, RS485_ADDR_GUI)
            
            # Test connection with PING
            if self.protocol.ping(RS485_ADDR_CONTROLLER_DIO):
                self.statusBar().showMessage(f"Connected to {port}")
                self.connect_btn.setEnabled(False)
                self.disconnect_btn.setEnabled(True)
                self.port_combo.setEnabled(False)
                
                # Get version
                version = self.protocol.get_version(RS485_ADDR_CONTROLLER_DIO)
                if version:
                    version_str = f"v{version['major']}.{version['minor']}.{version['patch']}.{version['build']}"
                    self.mcu_di_widget.set_version(version_str)
                    self.mcu_di_widget.update_status(True)
                
                # Add Digital Input Widget
                if self.digital_input_widget is None:
                    self.digital_input_widget = DigitalInputWidget(self.protocol)
                    self.centralWidget().layout().addWidget(self.digital_input_widget)
                
                # Start health monitoring
                self.start_health_monitoring()
                
            else:
                QMessageBox.warning(self, "Connection Failed", 
                                    "Controller DIO not detected. Please check:\n"
                                    "- RS485 connections\n"
                                    "- MCU power\n"
                                    "- Firmware flashed correctly")
                self.protocol.close()
                self.protocol = None
                
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to connect: {e}")
            if self.protocol:
                self.protocol.close()
            self.protocol = None
    
    def disconnect(self):
        """Disconnect from RS485"""
        self.stop_health_monitoring()
        
        if self.protocol:
            self.protocol.close()
            self.protocol = None
            
        self.connect_btn.setEnabled(True)
        self.disconnect_btn.setEnabled(False)
        self.port_combo.setEnabled(True)
        
        # Update MCU widget
        self.mcu_di_widget.update_status(False)
        
        self.statusBar().showMessage("Disconnected")
    
    def start_health_monitoring(self):
        """Start health monitoring thread"""
        if self.protocol:
            self.health_monitor_worker = HealthMonitorWorker(self.protocol)
            self.health_monitor_thread = QThread()
            
            self.health_monitor_worker.moveToThread(self.health_monitor_thread)
            self.health_monitor_worker.health_updated.connect(self.on_health_updated)
            self.health_monitor_worker.connection_lost.connect(self.on_connection_lost)
            
            self.health_monitor_thread.started.connect(self.health_monitor_worker.start_monitoring)
            self.health_monitor_thread.start()
    
    def stop_health_monitoring(self):
        """Stop health monitoring thread"""
        if self.health_monitor_worker:
            self.health_monitor_worker.stop_monitoring()
        
        if self.health_monitor_thread:
            self.health_monitor_thread.quit()
            self.health_monitor_thread.wait()
    
    def on_health_updated(self, mcu_addr: int, health_data: dict):
        """Handle health update"""
        if mcu_addr in self.mcu_widgets:
            self.mcu_widgets[mcu_addr].update_status(True, health_data)
    
    def on_connection_lost(self, mcu_addr: int):
        """Handle connection lost"""
        if mcu_addr in self.mcu_widgets:
            self.mcu_widgets[mcu_addr].update_status(False)
    
    def closeEvent(self, event):
        """Handle window close"""
        self.stop_health_monitoring()
        if self.protocol:
            self.protocol.close()
        event.accept()

def main():
    app = QApplication(sys.argv)
    
    # Set application style
    app.setStyle('Fusion')
    
    window = MainWindow()
    window.show()
    
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
