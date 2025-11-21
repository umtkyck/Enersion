#!/usr/bin/env python3
"""
Analog Input Controller - RS485 GUI Application
Monitors 26x 4-20mA, 6x 0-10V, and 4x NTC inputs via RS485
"""

import sys
import struct
import traceback
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                            QHBoxLayout, QLabel, QComboBox, QPushButton, 
                            QGridLayout, QGroupBox, QProgressBar, QTabWidget,
                            QMessageBox, QMenuBar, QMenu, QAction, QScrollArea,
                            QInputDialog)
from PyQt5.QtCore import Qt, QTimer, QObject, pyqtSignal, pyqtSlot, QThread
from PyQt5.QtGui import QFont
import serial.tools.list_ports

from rs485_protocol import RS485Protocol, RS485_ADDR_CONTROLLER_420, RS485_ADDR_GUI, MCUVersion
from version import get_version_string, APP_NAME, APP_DESCRIPTION, APP_COMPANY

# RS485 Command codes for analog reading
CMD_READ_ANALOG_420 = 0x40
CMD_ANALOG_420_RESPONSE = 0x41
CMD_READ_ANALOG_VOLTAGE = 0x42
CMD_ANALOG_VOLTAGE_RESPONSE = 0x43

class HealthMonitorWorker(QObject):
    """Worker thread for health monitoring"""
    
    health_updated = pyqtSignal(int, dict)  # mcu_id, health_data
    connection_lost = pyqtSignal(int)  # mcu_id
    
    def __init__(self, protocol: RS485Protocol, target_address: int):
        super().__init__()
        self.protocol = protocol
        self.target_address = target_address
        self.running = False
    
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
            try:
                # Send heartbeat
                result = self.protocol.heartbeat(self.target_address)
                
                if result:
                    mcu_id, health = result
                    
                    # Get detailed status
                    status = self.protocol.get_status(self.target_address)
                    
                    health_data = {
                        'health': health,
                        'connected': True,
                        'status': status
                    }
                    
                    self.health_updated.emit(self.target_address, health_data)
                else:
                    self.connection_lost.emit(self.target_address)
                    
            except Exception as e:
                print(f"Health monitor error for MCU {self.target_address}: {e}")
                self.connection_lost.emit(self.target_address)
            
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
        self.status_label = QLabel("Disconnected")
        self.status_label.setStyleSheet("color: red; font-weight: bold;")
        status_layout.addWidget(self.status_label)
        status_layout.addStretch()
        layout.addLayout(status_layout)
        
        # Health bar
        health_layout = QHBoxLayout()
        health_layout.addWidget(QLabel("Health:"))
        self.health_bar = QProgressBar()
        self.health_bar.setRange(0, 100)
        self.health_bar.setValue(0)
        self.health_bar.setFormat("%v%")
        health_layout.addWidget(self.health_bar)
        layout.addLayout(health_layout)
        
        # Version
        self.version_label = QLabel("Version: N/A")
        layout.addWidget(self.version_label)
        
        # Uptime
        self.uptime_label = QLabel("Uptime: N/A")
        layout.addWidget(self.uptime_label)
        
        # Statistics
        stats_layout = QGridLayout()
        stats_layout.addWidget(QLabel("RX Packets:"), 0, 0)
        self.rx_label = QLabel("0")
        stats_layout.addWidget(self.rx_label, 0, 1)
        
        stats_layout.addWidget(QLabel("TX Packets:"), 1, 0)
        self.tx_label = QLabel("0")
        stats_layout.addWidget(self.tx_label, 1, 1)
        
        stats_layout.addWidget(QLabel("Errors:"), 2, 0)
        self.error_label = QLabel("0")
        stats_layout.addWidget(self.error_label, 2, 1)
        
        layout.addLayout(stats_layout)
        
        self.setLayout(layout)
    
    def update_connection(self, connected: bool):
        """Update connection status"""
        self.connected = connected
        if connected:
            self.status_label.setText("Connected")
            self.status_label.setStyleSheet("color: green; font-weight: bold;")
        else:
            self.status_label.setText("Disconnected")
            self.status_label.setStyleSheet("color: red; font-weight: bold;")
            self.health_bar.setValue(0)
    
    def update_health(self, health_data: dict):
        """Update health information"""
        health = health_data.get('health', 0)
        self.health_bar.setValue(health)
        
        # Update color based on health
        if health >= 80:
            color = "green"
        elif health >= 50:
            color = "orange"
        else:
            color = "red"
        
        self.health_bar.setStyleSheet(f"""
            QProgressBar {{
                border: 2px solid grey;
                border-radius: 5px;
                text-align: center;
            }}
            QProgressBar::chunk {{
                background-color: {color};
            }}
        """)
        
        # Update status if available
        status = health_data.get('status')
        if status:
            self.uptime_label.setText(f"Uptime: {self._format_uptime(status.uptime)}")
            self.rx_label.setText(str(status.rx_packet_count))
            self.tx_label.setText(str(status.tx_packet_count))
            self.error_label.setText(str(status.error_count))
    
    def update_version(self, version: MCUVersion):
        """Update version information"""
        self.version_label.setText(f"Version: {version}")
    
    @staticmethod
    def _format_uptime(seconds: int) -> str:
        """Format uptime"""
        hours = seconds // 3600
        minutes = (seconds % 3600) // 60
        secs = seconds % 60
        return f"{hours:02d}:{minutes:02d}:{secs:02d}"

class AnalogInputWidget(QGroupBox):
    """Widget for displaying 4-20mA, 0-10V, and NTC analog inputs"""
    
    def __init__(self, protocol: RS485Protocol):
        super().__init__("Analog Inputs (Controller 420)")
        self.protocol = protocol
        self.target_device_address = RS485_ADDR_CONTROLLER_420
        
        # Data storage
        self.analog_420_values = [0.0] * 26
        self.voltage_values = [0.0] * 6
        
        self.init_ui()
        
        # Auto-refresh timer
        self.refresh_timer = QTimer()
        self.refresh_timer.timeout.connect(self.refresh_all_data)
        
    def init_ui(self):
        """Initialize UI"""
        main_layout = QVBoxLayout()
        
        # Tabs for different analog types
        tabs = QTabWidget()
        tabs.setMinimumHeight(400)
        
        # ========== 4-20mA Tab ==========
        analog_420_tab = QWidget()
        analog_420_layout = QVBoxLayout()
        
        scroll_420 = QScrollArea()
        scroll_420.setWidgetResizable(True)
        widget_420 = QWidget()
        grid_420 = QGridLayout()
        grid_420.setSpacing(5)
        
        self.analog_420_labels = []
        for i in range(26):
            label_name = QLabel(f"AI{i}:")
            label_name.setMinimumWidth(40)
            label_value = QLabel("-- mA | RAW: --")
            label_value.setStyleSheet("border: 1px solid gray; padding: 5px; min-width: 180px;")
            grid_420.addWidget(label_name, i // 4, (i % 4) * 2)
            grid_420.addWidget(label_value, i // 4, (i % 4) * 2 + 1)
            self.analog_420_labels.append(label_value)
        
        widget_420.setLayout(grid_420)
        scroll_420.setWidget(widget_420)
        analog_420_layout.addWidget(scroll_420)
        analog_420_tab.setLayout(analog_420_layout)
        tabs.addTab(analog_420_tab, "4-20mA (26 channels)")
        
        # ========== 0-10V Tab ==========
        voltage_tab = QWidget()
        voltage_layout = QVBoxLayout()
        
        grid_v = QGridLayout()
        grid_v.setSpacing(5)
        self.voltage_labels = []
        for i in range(6):
            label_name = QLabel(f"V{i}:")
            label_name.setMinimumWidth(40)
            label_value = QLabel("-- V | RAW: --")
            label_value.setStyleSheet("border: 1px solid gray; padding: 5px; min-width: 180px;")
            grid_v.addWidget(label_name, i // 3, (i % 3) * 2)
            grid_v.addWidget(label_value, i // 3, (i % 3) * 2 + 1)
            self.voltage_labels.append(label_value)
        
        voltage_layout.addLayout(grid_v)
        voltage_layout.addStretch()
        voltage_tab.setLayout(voltage_layout)
        tabs.addTab(voltage_tab, "0-10V (6 channels)")
        
        main_layout.addWidget(tabs)
        
        # Control buttons
        button_layout = QHBoxLayout()
        
        self.read_now_btn = QPushButton("Read All Now")
        self.read_now_btn.clicked.connect(self.read_all_inputs)
        self.read_now_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 10px;")
        button_layout.addWidget(self.read_now_btn)
        
        self.auto_refresh_btn = QPushButton("Start Auto-Refresh (1s)")
        self.auto_refresh_btn.clicked.connect(self.toggle_auto_refresh)
        self.auto_refresh_btn.setStyleSheet("background-color: #2196F3; color: white; font-weight: bold; padding: 10px;")
        button_layout.addWidget(self.auto_refresh_btn)
        
        main_layout.addLayout(button_layout)
        
        # Status info
        self.status_label = QLabel("Ready - Click 'Read All Now' to read analog inputs")
        self.status_label.setStyleSheet("padding: 5px; background-color: #f0f0f0;")
        self.status_label.setAlignment(Qt.AlignCenter)
        main_layout.addWidget(self.status_label)
        
        self.setLayout(main_layout)
    
    def set_target_address(self, address):
        """Set target device address"""
        self.target_device_address = address
    
    def read_all_inputs(self):
        """Read all analog inputs from controller"""
        if not self.protocol or not self.protocol.is_connected():
            self.status_label.setText("❌ Not connected to RS485")
            return
        
        self.status_label.setText("Reading analog inputs...")
        QApplication.processEvents()
        
        # Read 4-20mA inputs
        self.read_420ma_inputs()
        
        # Read 0-10V inputs
        self.read_voltage_inputs()
        
        self.status_label.setText("✓ All analog inputs updated")
    
    def read_420ma_inputs(self):
        """Read 4-20mA analog inputs"""
        try:
            # Send read command
            response = self.protocol.send_command_and_wait(
                self.target_device_address,
                CMD_READ_ANALOG_420,
                b'',
                timeout=2.0
            )
            
            if response and len(response.data) >= 156:  # 26 channels × 6 bytes (2 raw + 4 float) = 156 bytes
                # Parse 26 channels: raw (uint16) + float value
                for i in range(26):
                    offset = i * 6
                    raw_adc = struct.unpack('<H', response.data[offset:offset+2])[0]
                    value = struct.unpack('<f', response.data[offset+2:offset+6])[0]
                    self.analog_420_values[i] = value
                    
                    # Update label with color coding
                    if 4.0 <= value <= 20.0:
                        color = "green"
                    elif value < 3.8:
                        color = "red"  # Wire break
                    elif value > 21.0:
                        color = "red"  # Over-range
                    else:
                        color = "orange"  # Warning range
                    
                    self.analog_420_labels[i].setText(f"{value:.2f} mA | RAW: {raw_adc}")
                    self.analog_420_labels[i].setStyleSheet(
                        f"border: 2px solid {color}; padding: 5px; min-width: 180px; font-weight: bold;"
                    )
        except Exception as e:
            print(f"Error reading 4-20mA inputs: {e}")
            self.status_label.setText(f"❌ Error reading 4-20mA: {e}")
    
    def read_voltage_inputs(self):
        """Read 0-10V voltage inputs"""
        try:
            response = self.protocol.send_command_and_wait(
                self.target_device_address,
                CMD_READ_ANALOG_VOLTAGE,
                b'',
                timeout=2.0
            )
            
            if response and len(response.data) >= 36:  # 6 channels × 6 bytes (2 raw + 4 float) = 36 bytes
                for i in range(6):
                    offset = i * 6
                    raw_adc = struct.unpack('<H', response.data[offset:offset+2])[0]
                    value = struct.unpack('<f', response.data[offset+2:offset+6])[0]
                    self.voltage_values[i] = value
                    
                    # Update label with color coding
                    if 0.0 <= value <= 10.0:
                        color = "green"
                    else:
                        color = "red"  # Out of range
                    
                    self.voltage_labels[i].setText(f"{value:.2f} V | RAW: {raw_adc}")
                    self.voltage_labels[i].setStyleSheet(
                        f"border: 2px solid {color}; padding: 5px; min-width: 180px; font-weight: bold;"
                    )
        except Exception as e:
            print(f"Error reading voltage inputs: {e}")
            self.status_label.setText(f"❌ Error reading voltages: {e}")
    
    
    def refresh_all_data(self):
        """Auto-refresh all analog data"""
        self.read_all_inputs()
    
    def toggle_auto_refresh(self):
        """Toggle auto-refresh timer"""
        if self.refresh_timer.isActive():
            self.refresh_timer.stop()
            self.auto_refresh_btn.setText("Start Auto-Refresh (1s)")
            self.auto_refresh_btn.setStyleSheet("background-color: #2196F3; color: white; font-weight: bold; padding: 10px;")
        else:
            self.refresh_timer.start(1000)  # 1 second
            self.auto_refresh_btn.setText("Stop Auto-Refresh")
            self.auto_refresh_btn.setStyleSheet("background-color: #f44336; color: white; font-weight: bold; padding: 10px;")


class MainWindow(QMainWindow):
    """Main application window"""
    
    def __init__(self):
        super().__init__()
        self.protocol = None
        self.health_worker = None
        self.health_thread = None
        self.target_device_address = RS485_ADDR_CONTROLLER_420
        
        self.init_ui()
        self.scan_devices()
    
    def init_ui(self):
        """Initialize UI"""
        self.setWindowTitle(get_version_string())
        self.setGeometry(100, 100, 900, 700)
        
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        
        # Menu bar
        self.create_menu_bar()
        
        # ========== Connection Panel ==========
        conn_group = QGroupBox("RS485 Connection")
        conn_layout = QHBoxLayout()
        
        conn_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(250)
        conn_layout.addWidget(self.port_combo)
        
        self.refresh_btn = QPushButton("Refresh")
        self.refresh_btn.clicked.connect(self.scan_devices)
        conn_layout.addWidget(self.refresh_btn)
        
        conn_layout.addWidget(QLabel("Baud Rate:"))
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["9600", "19200", "38400", "57600", "115200"])
        self.baud_combo.setCurrentText("115200")
        conn_layout.addWidget(self.baud_combo)
        
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.clicked.connect(self.toggle_connection)
        self.connect_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        conn_layout.addWidget(self.connect_btn)
        
        conn_group.setLayout(conn_layout)
        main_layout.addWidget(conn_group)
        
        # ========== Controller Status ==========
        self.mcu_widget = MCUWidget("Controller Status (Analog Inputs)", self.target_device_address)
        main_layout.addWidget(self.mcu_widget)
        
        # ========== Analog Inputs Widget ==========
        self.analog_widget = None
        
        # Add stretch to push everything to top
        main_layout.addStretch()
        
        self.statusBar().showMessage("Ready")
    
    def create_menu_bar(self):
        """Create menu bar"""
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
        
        refresh_ports_action = QAction("Refresh Ports", self)
        refresh_ports_action.triggered.connect(self.scan_devices)
        tools_menu.addAction(refresh_ports_action)
        
        select_addr_action = QAction("Select Device Address...", self)
        select_addr_action.triggered.connect(self.select_device_address)
        tools_menu.addAction(select_addr_action)
        
        # Help menu
        help_menu = menubar.addMenu("Help")
        about_action = QAction("About", self)
        about_action.triggered.connect(self.show_about)
        help_menu.addAction(about_action)
    
    def select_device_address(self):
        """Allow user to select target RS485 device address"""
        addresses = {
            "0x01 - Controller 420 (4-20mA)": 0x01,
            "0x02 - Controller DIO (Digital I/O)": 0x02,
            "0x03 - Controller OUT (Digital OUT)": 0x03,
        }
        
        item, ok = QInputDialog.getItem(self, "Select Device Address",
                                       "Target RS485 Device:", list(addresses.keys()), 0, False)
        if ok and item:
            self.target_device_address = addresses[item]
            if self.analog_widget:
                self.analog_widget.set_target_address(self.target_device_address)
            
            # Update health worker target if running
            if self.health_worker:
                self.health_worker.target_address = self.target_device_address
                
            QMessageBox.information(self, "Address Updated", 
                                   f"Target device address set to 0x{self.target_device_address:02X}")
    
    def scan_devices(self):
        """Scan for available COM ports"""
        try:
            self.port_combo.clear()
            ports = list(serial.tools.list_ports.comports())
            
            if ports:
                for port in ports:
                    self.port_combo.addItem(f"{port.device} - {port.description}", port.device)
                self.connect_btn.setEnabled(True)
            else:
                self.port_combo.addItem("No COM ports found")
                self.connect_btn.setEnabled(False)
                
        except Exception as e:
            print(f"Error scanning ports: {e}")
            self.port_combo.addItem("Error scanning ports")
            self.connect_btn.setEnabled(False)
    
    def toggle_connection(self):
        """Toggle RS485 connection"""
        if self.protocol is None or not self.protocol.is_connected():
            self.connect()
        else:
            self.disconnect()
            
    def connect(self):
        """Connect to RS485"""
        port = self.port_combo.currentData()
        
        # Check for invalid port selections
        if not port or "No COM ports" in self.port_combo.currentText() or "Error scanning" in self.port_combo.currentText():
            QMessageBox.warning(self, "Error", "No valid COM port selected.\n\nPlease refresh ports and select a valid COM port.")
            return
        
        try:
            baudrate = int(self.baud_combo.currentText())
            
            # Ensure clean state
            if self.protocol:
                try:
                    self.protocol.disconnect()
                except:
                    pass
            
            self.protocol = RS485Protocol(port, baudrate)
            
            if self.protocol.connect():
                self.connect_btn.setText("Disconnect")
                self.connect_btn.setStyleSheet("background-color: #ff6b6b; font-weight: bold;")
                self.port_combo.setEnabled(False)
                self.baud_combo.setEnabled(False)
                self.refresh_btn.setEnabled(False)
                
                self.statusBar().showMessage(f"Connected to {port} @ {baudrate} baud")
                
                # Add Analog Input widget if not exists
                if self.analog_widget is None:
                    self.analog_widget = AnalogInputWidget(self.protocol)
                    self.analog_widget.set_target_address(self.target_device_address)
                    self.centralWidget().layout().insertWidget(2, self.analog_widget)
                else:
                    self.analog_widget.protocol = self.protocol
                
                # Start health monitoring
                self.start_health_monitoring()
                
                # Initial device scan - wait longer for MCU to be ready
                QTimer.singleShot(1500, self.scan_device_after_connect)
                
            else:
                QMessageBox.critical(self, "Error", 
                                    f"Failed to open {port}\n\n"
                                    "Check:\n"
                                    "- Port is not in use by another program\n"
                                    "- USB cable is connected\n"
                                    "- Driver is installed")
                self.protocol = None
                
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Connection error: {e}\n\n{traceback.format_exc()}")
            self.protocol = None
    
    def disconnect(self):
        """Disconnect from RS485"""
        # Stop health monitoring first
        self.stop_health_monitoring()
        
        if self.protocol:
            self.protocol.disconnect()
            self.protocol = None
        
        self.connect_btn.setText("Connect")
        self.connect_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        self.port_combo.setEnabled(True)
        self.baud_combo.setEnabled(True)
        self.refresh_btn.setEnabled(True)
        
        self.mcu_widget.update_connection(False)
        self.status_label.setText("Disconnected")
        self.status_label.setStyleSheet("color: red; font-weight: bold;")
        
        self.statusBar().showMessage("Disconnected")
    
    def start_health_monitoring(self):
        """Start periodic health monitoring"""
        if self.protocol:
            self.health_thread = QThread()
            self.health_worker = HealthMonitorWorker(self.protocol, self.target_device_address)
            self.health_worker.moveToThread(self.health_thread)
            
            # Connect signals
            self.health_thread.started.connect(self.health_worker.start_monitoring)
            self.health_worker.health_updated.connect(self.on_health_updated)
            self.health_worker.connection_lost.connect(self.on_connection_lost)
            
            self.health_thread.start()
    
    def stop_health_monitoring(self):
        """Stop health monitoring"""
        if self.health_worker:
            self.health_worker.stop_monitoring()
        
        if self.health_thread:
            self.health_thread.quit()
            self.health_thread.wait()
            self.health_thread = None
            self.health_worker = None

    @pyqtSlot(int, dict)
    def on_health_updated(self, mcu_addr: int, health_data: dict):
        """Handle health update"""
        if mcu_addr == self.target_device_address:
            self.mcu_widget.update_connection(True)
            self.mcu_widget.update_health(health_data)

    @pyqtSlot(int)
    def on_connection_lost(self, mcu_addr: int):
        """Handle connection lost"""
        if mcu_addr == self.target_device_address:
            self.mcu_widget.update_connection(False)

    def scan_device_after_connect(self):
        """Scan for Controller 420 device after connection"""
        if not self.protocol or not self.protocol.is_connected():
            return
        
        self.statusBar().showMessage("Scanning for Controller 420...")
        QApplication.processEvents()
        
        # Ping Controller 420 device
        if self.protocol.ping(self.target_device_address):
            self.mcu_widget.update_connection(True)
            
            # Get version
            version = self.protocol.get_version(self.target_device_address)
            if version:
                self.mcu_widget.update_version(version)
            
            self.statusBar().showMessage(f"✓ Controller 420 (0x{self.target_device_address:02X}) connected!")
            QMessageBox.information(self, "Scan Complete", 
                                   f"Controller 420 (0x{self.target_device_address:02X}) detected and ready!\n\n"
                                   "You can now monitor analog inputs:\n"
                                   "- 26x 4-20mA channels (with RAW ADC)\n"
                                   "- 6x 0-10V channels (with RAW ADC)")
        else:
            self.mcu_widget.update_connection(False)
            self.statusBar().showMessage(f"✗ Controller 420 not found")
            QMessageBox.warning(self, "Scan Complete", 
                               f"Controller 420 (0x{self.target_device_address:02X}) not detected.\n\n"
                               "Please check:\n"
                               "- RS485 connection (correct COM port)\n"
                               "- Controller power\n"
                               "- Baud rate settings (115200)\n"
                               f"- Controller address (should be 0x{self.target_device_address:02X})")
    
    def show_about(self):
        """Show about dialog"""
        QMessageBox.about(self, "About", 
                         f"{get_version_string()}\n\n"
                         f"{APP_DESCRIPTION}\n\n"
                         f"Company: {APP_COMPANY}")
    
    def closeEvent(self, event):
        """Handle window close"""
        self.disconnect()
        event.accept()


def main():
    """Main application entry point"""
    try:
        app = QApplication(sys.argv)
        app.setStyle('Fusion')  # Modern look
        
        window = MainWindow()
        window.show()
        
        sys.exit(app.exec_())
    except Exception as e:
        print(f"Fatal error: {e}")
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
