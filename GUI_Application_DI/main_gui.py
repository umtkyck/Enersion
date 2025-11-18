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
        self.target_device_address = RS485_ADDR_CONTROLLER_DIO  # Default: 0x02
        
        try:
            self.init_ui()
            self.scan_devices()
        except Exception as e:
            print(f"Warning during initialization: {e}")
            # Continue anyway - user can still try to connect
    
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
        
        # Connection panel (matching DO GUI style)
        conn_group = QGroupBox("RS485 Connection")
        conn_layout = QHBoxLayout()
        
        conn_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(200)
        conn_layout.addWidget(self.port_combo)
        
        self.refresh_btn = QPushButton("🔄 Refresh")
        self.refresh_btn.setToolTip("Refresh COM port list")
        self.refresh_btn.clicked.connect(self.scan_devices)
        conn_layout.addWidget(self.refresh_btn)
        
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.setMinimumHeight(35)
        self.connect_btn.clicked.connect(self.connect)
        conn_layout.addWidget(self.connect_btn)
        
        self.disconnect_btn = QPushButton("Disconnect")
        self.disconnect_btn.setMinimumHeight(35)
        self.disconnect_btn.setEnabled(False)
        self.disconnect_btn.clicked.connect(self.disconnect)
        conn_layout.addWidget(self.disconnect_btn)
        
        conn_layout.addStretch()
        
        conn_group.setLayout(conn_layout)
        main_layout.addWidget(conn_group)
        
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
        
        # Menu bar
        menubar = self.menuBar()
        
        # File menu
        file_menu = menubar.addMenu("File")
        exit_action = QAction("Exit", self)
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)
        
        # Tools menu
        tools_menu = menubar.addMenu("Tools")
        
        scan_action = QAction("Scan Device", self)
        scan_action.triggered.connect(self.scan_device_after_connect)
        tools_menu.addAction(scan_action)
        
        refresh_action = QAction("Refresh Ports", self)
        refresh_action.triggered.connect(self.scan_devices)
        tools_menu.addAction(refresh_action)
        
        tools_menu.addSeparator()
        
        address_action = QAction("Select Device Address...", self)
        address_action.triggered.connect(self.select_device_address)
        tools_menu.addAction(address_action)
        
        # Help menu
        help_menu = menubar.addMenu("Help")
        about_action = QAction("About", self)
        about_action.triggered.connect(self.show_about)
        help_menu.addAction(about_action)
    
    def scan_devices(self):
        """Scan available COM ports"""
        try:
            ports = serial.tools.list_ports.comports()
            self.port_combo.clear()
            
            if not ports:
                self.port_combo.addItem("No COM ports found")
                self.connect_btn.setEnabled(False)
                return
            
            for port in ports:
                self.port_combo.addItem(f"{port.device} - {port.description}")
                
            self.connect_btn.setEnabled(True)
            
        except Exception as e:
            print(f"Error scanning COM ports: {e}")
            self.port_combo.clear()
            self.port_combo.addItem("Error scanning ports")
            self.connect_btn.setEnabled(False)
    
    def scan_device_after_connect(self):
        """Scan for Controller DI device (after connection)"""
        if not self.protocol or not self.protocol.serial or not self.protocol.serial.is_open:
            QMessageBox.warning(self, "Error", "Not connected to RS485!\n\nPlease connect to a COM port first.")
            return
        
        self.statusBar().showMessage("Scanning for Controller DI...")
        QApplication.processEvents()
        
        # Ping Controller DI device (use selected address)
        if self.protocol.ping(self.target_device_address):
            self.mcu_di_widget.update_status(True)
            
            # Get version
            version = self.protocol.get_version(self.target_device_address)
            if version:
                version_str = f"v{version['major']}.{version['minor']}.{version['patch']}.{version['build']}"
                self.mcu_di_widget.set_version(version_str)
            
            self.statusBar().showMessage(f"✓ Device 0x{self.target_device_address:02X} found and connected!")
            QMessageBox.information(self, "Scan Complete", 
                                   f"Device at address 0x{self.target_device_address:02X} detected and ready!\n\n"
                                   "You can now monitor the digital inputs.")
        else:
            self.mcu_di_widget.update_status(False)
            self.statusBar().showMessage(f"✗ Device 0x{self.target_device_address:02X} not found")
            QMessageBox.warning(self, "Scan Complete", 
                               f"Device at address 0x{self.target_device_address:02X} not detected.\n\n"
                               "Please check:\n"
                               "- Device address (Tools → Select Device Address)\n"
                               "- RS485 connections (A/B wires)\n"
                               "- MCU power and firmware\n"
                               "- COM7 debug console output\n\n"
                               "Try different addresses:\n"
                               "• 0x02 = Controller DI\n"
                               "• 0x03 = Controller OUT")
    
    def select_device_address(self):
        """Select target device RS485 address"""
        dialog = QDialog(self)
        dialog.setWindowTitle("Select Device Address")
        dialog.setMinimumWidth(400)
        
        layout = QVBoxLayout()
        
        # Info label
        info_label = QLabel("Select the RS485 address of the target device:")
        info_label.setWordWrap(True)
        layout.addWidget(info_label)
        
        # Address selection
        addr_layout = QHBoxLayout()
        addr_layout.addWidget(QLabel("Device Address (hex):"))
        
        addr_combo = QComboBox()
        addr_combo.addItem("0x01 - Controller 4-20mA", 0x01)
        addr_combo.addItem("0x02 - Controller DI (Digital Inputs)", 0x02)
        addr_combo.addItem("0x03 - Controller OUT (Digital Outputs)", 0x03)
        addr_combo.addItem("0x04 - Custom Device", 0x04)
        addr_combo.addItem("0x05 - Custom Device", 0x05)
        addr_combo.addItem("0xFF - Broadcast", 0xFF)
        
        # Set current selection
        for i in range(addr_combo.count()):
            if addr_combo.itemData(i) == self.target_device_address:
                addr_combo.setCurrentIndex(i)
                break
        
        addr_layout.addWidget(addr_combo)
        layout.addLayout(addr_layout)
        
        # Current address display
        current_label = QLabel(f"<b>Current address:</b> 0x{self.target_device_address:02X}")
        current_label.setStyleSheet("padding: 10px; background-color: #e8f4f8; border-radius: 5px;")
        layout.addWidget(current_label)
        
        # Warning
        warning_label = QLabel("⚠️ <b>Note:</b> Make sure the selected address matches your firmware configuration.")
        warning_label.setWordWrap(True)
        warning_label.setStyleSheet("color: #856404; background-color: #fff3cd; padding: 10px; border-radius: 5px;")
        layout.addWidget(warning_label)
        
        # Buttons
        button_layout = QHBoxLayout()
        ok_btn = QPushButton("OK")
        ok_btn.clicked.connect(dialog.accept)
        cancel_btn = QPushButton("Cancel")
        cancel_btn.clicked.connect(dialog.reject)
        button_layout.addStretch()
        button_layout.addWidget(ok_btn)
        button_layout.addWidget(cancel_btn)
        layout.addLayout(button_layout)
        
        dialog.setLayout(layout)
        
        if dialog.exec_() == QDialog.Accepted:
            old_addr = self.target_device_address
            self.target_device_address = addr_combo.currentData()
            self.statusBar().showMessage(f"Device address changed: 0x{old_addr:02X} → 0x{self.target_device_address:02X}")
            
            # Update MCU widget name
            self.mcu_di_widget.name_label.setText(f"Controller (Address: 0x{self.target_device_address:02X})")
            
            QMessageBox.information(self, "Address Changed", 
                                   f"Target device address set to: 0x{self.target_device_address:02X}\n\n"
                                   "Please reconnect to apply changes.")
    
    def show_about(self):
        """Show about dialog"""
        about_text = f"""
        <h2>Digital IN Controller</h2>
        <p><b>Version:</b> {VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}.{VERSION_BUILD}</p>
        <p><b>Description:</b> RS485 Digital Input Monitor</p>
        <p><b>Features:</b></p>
        <ul>
            <li>56 Digital Input Channels (DI0-DI55)</li>
            <li>Real-time monitoring</li>
            <li>Auto-refresh capability</li>
            <li>RS485 communication</li>
            <li>Health monitoring</li>
        </ul>
        <p><b>Hardware:</b> STM32H7 + RS485 Transceiver</p>
        <p><b>Company:</b> Enersion</p>
        <p><b>Current Device Address:</b> 0x{self.target_device_address:02X}</p>
        <p>© 2025 All rights reserved</p>
        """
        QMessageBox.about(self, "About Digital IN Controller", about_text)
    
    def connect(self):
        """Connect to RS485"""
        if self.port_combo.currentIndex() < 0:
            QMessageBox.warning(self, "Warning", "Please select a COM port")
            return
        
        port_text = self.port_combo.currentText()
        
        # Check for error messages in combo box
        if "No COM ports" in port_text or "Error scanning" in port_text:
            QMessageBox.warning(self, "Warning", "No valid COM port selected")
            return
            
        port = port_text.split(" - ")[0]
        
        try:
            # Close existing connection if any
            if self.protocol:
                try:
                    self.protocol.close()
                except:
                    pass
                self.protocol = None
            
            self.statusBar().showMessage(f"Connecting to {port}...")
            QApplication.processEvents()
            
            baudrate = 115200
            self.protocol = RS485Protocol(port, baudrate)
            
            # Connect to serial port
            if not self.protocol.connect():
                QMessageBox.warning(self, "Connection Failed", 
                                   f"Failed to open {port}.\n\nPlease check:\n"
                                   "- Port is not in use by another application\n"
                                   "- USB cable is connected\n"
                                   "- Device drivers are installed")
                if self.protocol:
                    try:
                        self.protocol.close()
                    except:
                        pass
                    self.protocol = None
                return
            
            # Wait for MCU to be ready (like DO GUI does)
            import time
            time.sleep(0.5)  # 500ms delay for MCU to stabilize
            
            # Test connection with PING (use selected device address)
            self.statusBar().showMessage(f"Connecting to {port} (Device: 0x{self.target_device_address:02X})...")
            QApplication.processEvents()
            
            print(f"[DEBUG] About to PING address 0x{self.target_device_address:02X}...")
            ping_result = self.protocol.ping(self.target_device_address)
            print(f"[DEBUG] PING result: {ping_result}")
            
            if ping_result:
                self.statusBar().showMessage(f"Connected to {port}")
                self.connect_btn.setEnabled(False)
                self.disconnect_btn.setEnabled(True)
                self.port_combo.setEnabled(False)
                
                # Get version
                version = self.protocol.get_version(self.target_device_address)
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
                                    f"Device at address 0x{self.target_device_address:02X} not detected.\n\n"
                                    "Please check:\n"
                                    "- RS485 connections (A/B wires)\n"
                                    "- Device address (Tools → Select Device Address)\n"
                                    "- MCU power and LED status\n"
                                    "- Firmware flashed correctly\n"
                                    "- COM7 debug output shows startup\n\n"
                                    "Current ports:\n"
                                    f"⚠️ COM3 = DI RS485 (USE THIS FOR DIGITAL IN!)\n"
                                    f"• COM10 = DO RS485")
                if self.protocol:
                    try:
                        self.protocol.close()
                    except:
                        pass
                self.protocol = None
                self.statusBar().showMessage("Connection failed - please check connections")
                
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to connect to {port}:\n\n{str(e)}\n\nPlease check:\n- COM port is not in use\n- RS485 adapter is connected")
            if self.protocol:
                try:
                    self.protocol.close()
                except:
                    pass
            self.protocol = None
            self.statusBar().showMessage("Connection error - ready to retry")
    
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
    try:
        app = QApplication(sys.argv)
        
        # Set application style
        app.setStyle('Fusion')
        
        window = MainWindow()
        window.show()
        
        sys.exit(app.exec_())
    except Exception as e:
        print(f"Application error: {e}")
        import traceback
        traceback.print_exc()
        input("Press Enter to exit...")

if __name__ == '__main__':
    main()
