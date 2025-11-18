"""
******************************************************************************
@file           : main_gui.py
@brief          : Main GUI Application for PLC Controller
******************************************************************************
@attention

Main application with:
- RS485 communication
- Device monitoring
- Digital I/O control
- Health monitoring
- Heartbeat monitoring

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
    """Worker thread for health monitoring - Digital OUT Controller Only"""
    
    health_updated = pyqtSignal(int, dict)  # mcu_id, health_data
    connection_lost = pyqtSignal(int)  # mcu_id
    
    def __init__(self, protocol: RS485Protocol):
        super().__init__()
        self.protocol = protocol
        self.running = False
        self.mcu_addresses = [
            RS485_ADDR_CONTROLLER_OUT  # Only monitor OUT controller
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
    """Widget for analog input display"""
    
    def __init__(self, protocol: RS485Protocol):
        super().__init__("Analog Inputs (Controller 420)")
        self.protocol = protocol
        self.init_ui()
    
    def init_ui(self):
        """Initialize UI"""
        main_layout = QVBoxLayout()
        
        # Tabs for different analog types
        tabs = QTabWidget()
        tabs.setMinimumHeight(300)  # Minimum height for tabs to be visible
        
        # 4-20mA Tab
        analog_420_tab = QWidget()
        analog_420_layout = QVBoxLayout()
        
        scroll_420 = QScrollArea()
        scroll_420.setWidgetResizable(True)
        scroll_420.setMinimumHeight(250)
        widget_420 = QWidget()
        grid_420 = QGridLayout()
        grid_420.setSpacing(5)
        
        self.analog_420_labels = []
        for i in range(26):
            label_name = QLabel(f"AI{i}:")
            label_name.setMinimumWidth(40)
            label_value = QLabel("-- mA")
            label_value.setStyleSheet("border: 1px solid gray; padding: 5px; min-width: 100px;")
            grid_420.addWidget(label_name, i // 4, (i % 4) * 2)
            grid_420.addWidget(label_value, i // 4, (i % 4) * 2 + 1)
            self.analog_420_labels.append(label_value)
        
        widget_420.setLayout(grid_420)
        scroll_420.setWidget(widget_420)
        analog_420_layout.addWidget(scroll_420)
        analog_420_tab.setLayout(analog_420_layout)
        tabs.addTab(analog_420_tab, "4-20mA (26 channels)")
        
        # 0-10V Tab
        voltage_tab = QWidget()
        voltage_layout = QVBoxLayout()
        
        grid_v = QGridLayout()
        grid_v.setSpacing(5)
        self.voltage_labels = []
        for i in range(6):
            label_name = QLabel(f"V{i}:")
            label_name.setMinimumWidth(40)
            label_value = QLabel("-- V")
            label_value.setStyleSheet("border: 1px solid gray; padding: 5px; min-width: 100px;")
            grid_v.addWidget(label_name, i // 3, (i % 3) * 2)
            grid_v.addWidget(label_value, i // 3, (i % 3) * 2 + 1)
            self.voltage_labels.append(label_value)
        
        voltage_layout.addLayout(grid_v)
        voltage_layout.addStretch()
        voltage_tab.setLayout(voltage_layout)
        tabs.addTab(voltage_tab, "0-10V (6 channels)")
        
        # NTC Tab
        ntc_tab = QWidget()
        ntc_layout = QVBoxLayout()
        
        grid_ntc = QGridLayout()
        grid_ntc.setSpacing(5)
        self.ntc_labels = []
        for i in range(4):
            label_name = QLabel(f"NTC{i}:")
            label_name.setMinimumWidth(50)
            label_value = QLabel("-- °C")
            label_value.setStyleSheet("border: 1px solid gray; padding: 5px; min-width: 100px;")
            grid_ntc.addWidget(label_name, i // 2, (i % 2) * 2)
            grid_ntc.addWidget(label_value, i // 2, (i % 2) * 2 + 1)
            self.ntc_labels.append(label_value)
        
        ntc_layout.addLayout(grid_ntc)
        ntc_layout.addStretch()
        ntc_tab.setLayout(ntc_layout)
        tabs.addTab(ntc_tab, "NTC Temperature (4 channels)")
        
        main_layout.addWidget(tabs)
        
        # Control buttons
        button_layout = QHBoxLayout()
        
        self.read_analog_btn = QPushButton("Read All Analog Inputs")
        self.read_analog_btn.setMinimumHeight(30)
        self.read_analog_btn.clicked.connect(self.read_analog_inputs)
        button_layout.addWidget(self.read_analog_btn)
        
        main_layout.addLayout(button_layout)
        
        self.setLayout(main_layout)
    
    def read_analog_inputs(self):
        """Read all analog inputs"""
        try:
            # Read 4-20mA
            data_420 = self.protocol.read_analog_420mA(RS485_ADDR_CONTROLLER_420)
            if data_420:
                for i, ch_data in enumerate(data_420):
                    if i < len(self.analog_420_labels):
                        current = ch_data['current_mA']
                        percent = ((current - 4) / 16) * 100 if current >= 4 else 0
                        self.analog_420_labels[i].setText(f"{current:.2f} mA ({percent:.1f}%)")
            
            # Read 0-10V
            data_v = self.protocol.read_analog_voltage(RS485_ADDR_CONTROLLER_420)
            if data_v:
                for i, ch_data in enumerate(data_v):
                    if i < len(self.voltage_labels):
                        voltage = ch_data['voltage_V']
                        percent = (voltage / 10) * 100
                        self.voltage_labels[i].setText(f"{voltage:.2f} V ({percent:.1f}%)")
            
            # Read NTC
            data_ntc = self.protocol.read_ntc_temperatures(RS485_ADDR_CONTROLLER_420)
            if data_ntc:
                for i, ch_data in enumerate(data_ntc):
                    if i < len(self.ntc_labels):
                        temp = ch_data['temperature_C']
                        self.ntc_labels[i].setText(f"{temp:.1f} °C")
                        
        except Exception as e:
            QMessageBox.warning(self, "Error", f"Failed to read analog inputs: {e}")

class DigitalOutputWidget(QGroupBox):
    """Widget for digital output control - 56 channels"""
    
    def __init__(self, protocol: RS485Protocol):
        super().__init__("Digital Output Control - 56 Channels")
        self.protocol = protocol
        self.init_ui()
    
    def init_ui(self):
        """Initialize UI"""
        layout = QVBoxLayout()
        
        # Info label
        info_label = QLabel("Controller OUT (Address: 0x03) - RS485 Digital Output Module")
        info_label.setStyleSheet("color: #0066cc; font-weight: bold; padding: 5px;")
        layout.addWidget(info_label)
        
        # Digital Outputs
        do_group = QGroupBox("Digital Outputs (DO0 - DO55)")
        do_scroll = QScrollArea()
        do_scroll.setWidgetResizable(True)
        do_scroll.setMinimumHeight(300)
        do_widget = QWidget()
        do_layout = QGridLayout()
        do_layout.setSpacing(5)
        
        self.do_checkboxes = []
        for i in range(56):
            checkbox = QCheckBox(f"DO{i:02d}")
            checkbox.setMinimumWidth(70)
            checkbox.stateChanged.connect(self.on_output_changed)
            do_layout.addWidget(checkbox, i // 8, i % 8)
            self.do_checkboxes.append(checkbox)
        
        do_widget.setLayout(do_layout)
        do_scroll.setWidget(do_widget)
        do_group_layout = QVBoxLayout()
        do_group_layout.addWidget(do_scroll)
        do_group.setLayout(do_group_layout)
        layout.addWidget(do_group)
        
        # Control buttons
        button_layout = QHBoxLayout()
        
        self.write_do_btn = QPushButton("✓ Write Outputs to Controller")
        self.write_do_btn.setMinimumHeight(40)
        self.write_do_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        self.write_do_btn.clicked.connect(self.write_outputs)
        button_layout.addWidget(self.write_do_btn)
        
        self.read_do_btn = QPushButton("↻ Read Current Output State")
        self.read_do_btn.setMinimumHeight(40)
        self.read_do_btn.setStyleSheet("background-color: #2196F3; color: white; font-weight: bold;")
        self.read_do_btn.clicked.connect(self.read_outputs)
        button_layout.addWidget(self.read_do_btn)
        
        self.all_on_btn = QPushButton("All ON")
        self.all_on_btn.setMinimumHeight(40)
        self.all_on_btn.clicked.connect(self.set_all_on)
        button_layout.addWidget(self.all_on_btn)
        
        self.all_off_btn = QPushButton("All OFF")
        self.all_off_btn.setMinimumHeight(40)
        self.all_off_btn.clicked.connect(self.set_all_off)
        button_layout.addWidget(self.all_off_btn)
        
        layout.addLayout(button_layout)
        
        # Status info
        self.status_label = QLabel("Ready - Select outputs and click 'Write Outputs'")
        self.status_label.setStyleSheet("padding: 5px; background-color: #f0f0f0;")
        layout.addWidget(self.status_label)
        
        self.setLayout(layout)
    
    def write_outputs(self):
        """Write digital outputs (56 channels)"""
        try:
            # Build output byte array (56 outputs = 7 bytes)
            output_bytes = bytearray(7)
            
            for i in range(56):
                if i < len(self.do_checkboxes) and self.do_checkboxes[i].isChecked():
                    byte_idx = i // 8
                    bit_idx = i % 8
                    output_bytes[byte_idx] |= (1 << bit_idx)
            
            # Count active outputs
            active_count = sum(1 for cb in self.do_checkboxes if cb.isChecked())
            
            self.status_label.setText(f"Writing {active_count} active outputs...")
            QApplication.processEvents()
            
            success = self.protocol.write_digital_outputs(RS485_ADDR_CONTROLLER_OUT, bytes(output_bytes))
            
            if success:
                self.status_label.setText(f"✓ Success! {active_count} outputs written to controller")
                self.status_label.setStyleSheet("padding: 5px; background-color: #d4edda; color: #155724;")
            else:
                self.status_label.setText("✗ Error: Failed to write outputs (no response from controller)")
                self.status_label.setStyleSheet("padding: 5px; background-color: #f8d7da; color: #721c24;")
                
        except Exception as e:
            self.status_label.setText(f"✗ Error: {e}")
            self.status_label.setStyleSheet("padding: 5px; background-color: #f8d7da; color: #721c24;")
    
    def read_outputs(self):
        """Read current output state (56 channels)"""
        try:
            self.status_label.setText("Reading output state from controller...")
            QApplication.processEvents()
            
            data = self.protocol.read_digital_outputs(RS485_ADDR_CONTROLLER_OUT)
            if data:
                # Update checkboxes (56 outputs = 7 bytes)
                active_count = 0
                for i in range(56):
                    byte_idx = i // 8
                    bit_idx = i % 8
                    if byte_idx < len(data):
                        state = (data[byte_idx] >> bit_idx) & 0x01
                        if i < len(self.do_checkboxes):
                            self.do_checkboxes[i].setChecked(bool(state))
                            if state:
                                active_count += 1
                
                self.status_label.setText(f"✓ Read complete: {active_count} outputs are active")
                self.status_label.setStyleSheet("padding: 5px; background-color: #d4edda; color: #155724;")
            else:
                self.status_label.setText("✗ Error: Failed to read outputs (no response from controller)")
                self.status_label.setStyleSheet("padding: 5px; background-color: #f8d7da; color: #721c24;")
        except Exception as e:
            self.status_label.setText(f"✗ Error: {e}")
            self.status_label.setStyleSheet("padding: 5px; background-color: #f8d7da; color: #721c24;")
    
    def set_all_on(self):
        """Set all outputs to ON"""
        for checkbox in self.do_checkboxes:
            checkbox.setChecked(True)
        self.status_label.setText("All outputs set to ON (not written yet - click 'Write Outputs')")
        self.status_label.setStyleSheet("padding: 5px; background-color: #fff3cd; color: #856404;")
    
    def set_all_off(self):
        """Set all outputs to OFF"""
        for checkbox in self.do_checkboxes:
            checkbox.setChecked(False)
        self.status_label.setText("All outputs set to OFF (not written yet - click 'Write Outputs')")
        self.status_label.setStyleSheet("padding: 5px; background-color: #fff3cd; color: #856404;")
    
    def on_output_changed(self):
        """Handle output checkbox change"""
        active_count = sum(1 for cb in self.do_checkboxes if cb.isChecked())
        self.status_label.setText(f"{active_count} outputs selected (click 'Write Outputs' to apply)")
        self.status_label.setStyleSheet("padding: 5px; background-color: #f0f0f0;")

class MainWindow(QMainWindow):
    """Main Application Window - Digital OUT Controller Only"""
    
    def __init__(self):
        super().__init__()
        self.protocol = None
        self.health_worker = None
        self.health_thread = None
        
        self.init_ui()
        self.refresh_ports()
    
    def init_ui(self):
        """Initialize user interface"""
        self.setWindowTitle(get_version_string() + " - Digital OUT Controller")
        self.setMinimumSize(800, 600)
        self.resize(800, 600)  # Default window size
        
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)
        
        # Connection panel
        conn_group = QGroupBox("RS485 Connection")
        conn_layout = QHBoxLayout()
        
        conn_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(200)
        conn_layout.addWidget(self.port_combo)
        
        self.refresh_btn = QPushButton("Refresh")
        self.refresh_btn.clicked.connect(self.refresh_ports)
        conn_layout.addWidget(self.refresh_btn)
        
        conn_layout.addWidget(QLabel("Baud Rate:"))
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(["9600", "19200", "38400", "57600", "115200"])
        self.baud_combo.setCurrentText("115200")
        conn_layout.addWidget(self.baud_combo)
        
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.setMinimumHeight(35)
        self.connect_btn.clicked.connect(self.toggle_connection)
        conn_layout.addWidget(self.connect_btn)
        
        conn_layout.addStretch()
        
        conn_group.setLayout(conn_layout)
        main_layout.addWidget(conn_group)
        
        # MCU Status panel - Only Controller OUT
        self.mcu_out_widget = MCUWidget("Controller OUT (Digital Outputs - 56 Channels)", RS485_ADDR_CONTROLLER_OUT)
        main_layout.addWidget(self.mcu_out_widget)
        
        self.mcu_widgets = {
            RS485_ADDR_CONTROLLER_OUT: self.mcu_out_widget
        }
        
        # Digital Output control (will be enabled when connected)
        self.do_widget = None
        
        # Status bar
        self.statusBar().showMessage("Ready - Connect to RS485 to start")
        
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
        scan_action.triggered.connect(self.scan_devices)
        tools_menu.addAction(scan_action)
        
        # Help menu
        help_menu = menubar.addMenu("Help")
        
        about_action = QAction("About", self)
        about_action.triggered.connect(self.show_about)
        help_menu.addAction(about_action)
    
    def refresh_ports(self):
        """Refresh available serial ports"""
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}", port.device)
        
        if self.port_combo.count() == 0:
            self.port_combo.addItem("No ports found")
    
    def toggle_connection(self):
        """Toggle RS485 connection"""
        if self.protocol is None or not self.protocol.is_connected():
            self.connect()
        else:
            self.disconnect()
    
    def connect(self):
        """Connect to RS485"""
        port = self.port_combo.currentData()
        if not port:
            QMessageBox.warning(self, "Error", "No port selected")
            return
        
        try:
            baudrate = int(self.baud_combo.currentText())
            self.protocol = RS485Protocol(port, baudrate)
            
            if self.protocol.connect():
                self.connect_btn.setText("Disconnect")
                self.connect_btn.setStyleSheet("background-color: #ff6b6b; font-weight: bold;")
                self.port_combo.setEnabled(False)
                self.baud_combo.setEnabled(False)
                self.refresh_btn.setEnabled(False)
                
                self.statusBar().showMessage(f"Connected to {port} @ {baudrate} baud")
                
                # Add Digital Output widget only
                if self.do_widget is None:
                    self.do_widget = DigitalOutputWidget(self.protocol)
                    self.centralWidget().layout().addWidget(self.do_widget)
                
                # Start health monitoring
                self.start_health_monitoring()
                
                # Initial device scan - wait longer for MCU to be ready
                QTimer.singleShot(1500, self.scan_devices)
                
            else:
                QMessageBox.critical(self, "Error", "Failed to connect to serial port")
                self.protocol = None
                
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Connection error: {e}")
            self.protocol = None
    
    def disconnect(self):
        """Disconnect from RS485"""
        if self.protocol:
            # Stop health monitoring
            self.stop_health_monitoring()
            
            self.protocol.disconnect()
            self.protocol = None
            
        self.connect_btn.setText("Connect")
        self.connect_btn.setStyleSheet("")
        self.port_combo.setEnabled(True)
        self.baud_combo.setEnabled(True)
        self.refresh_btn.setEnabled(True)
        
        # Update MCU widget
        for widget in self.mcu_widgets.values():
            widget.update_connection(False)
        
        self.statusBar().showMessage("Disconnected")
    
    def start_health_monitoring(self):
        """Start health monitoring thread"""
        if self.protocol:
            self.health_thread = QThread()
            self.health_worker = HealthMonitorWorker(self.protocol)
            self.health_worker.moveToThread(self.health_thread)
            
            # Connect signals
            self.health_thread.started.connect(self.health_worker.start_monitoring)
            self.health_worker.health_updated.connect(self.on_health_updated)
            self.health_worker.connection_lost.connect(self.on_connection_lost)
            
            self.health_thread.start()
    
    def stop_health_monitoring(self):
        """Stop health monitoring thread"""
        if self.health_worker:
            self.health_worker.stop_monitoring()
        
        if self.health_thread:
            self.health_thread.quit()
            self.health_thread.wait()
    
    @pyqtSlot(int, dict)
    def on_health_updated(self, mcu_addr: int, health_data: dict):
        """Handle health update"""
        if mcu_addr in self.mcu_widgets:
            self.mcu_widgets[mcu_addr].update_connection(True)
            self.mcu_widgets[mcu_addr].update_health(health_data)
    
    @pyqtSlot(int)
    def on_connection_lost(self, mcu_addr: int):
        """Handle connection lost"""
        if mcu_addr in self.mcu_widgets:
            self.mcu_widgets[mcu_addr].update_connection(False)
    
    def scan_devices(self):
        """Scan for Controller OUT device"""
        if not self.protocol or not self.protocol.is_connected():
            QMessageBox.warning(self, "Error", "Not connected to RS485")
            return
        
        self.statusBar().showMessage("Scanning for Controller OUT...")
        QApplication.processEvents()
        
        # Ping Controller OUT device
        if self.protocol.ping(RS485_ADDR_CONTROLLER_OUT):
            self.mcu_out_widget.update_connection(True)
            
            # Get version
            version = self.protocol.get_version(RS485_ADDR_CONTROLLER_OUT)
            if version:
                self.mcu_out_widget.update_version(version)
            
            self.statusBar().showMessage("✓ Controller OUT found and connected!")
            QMessageBox.information(self, "Scan Complete", 
                                   "Controller OUT (0x03) detected and ready!\n\n"
                                   "You can now control the 56 digital outputs.")
        else:
            self.mcu_out_widget.update_connection(False)
            self.statusBar().showMessage("✗ Controller OUT not found")
            QMessageBox.warning(self, "Scan Complete", 
                               "Controller OUT (0x03) not detected.\n\n"
                               "Please check:\n"
                               "- RS485 connection\n"
                               "- Controller power\n"
                               "- Baud rate settings\n"
                               "- Controller address (should be 0x03)")
    
    def show_about(self):
        """Show about dialog"""
        QMessageBox.about(self, "About", 
                         f"{get_version_string()}\n\n"
                         f"{APP_DESCRIPTION}\n\n"
                         f"Company: {APP_COMPANY}")
    
    def closeEvent(self, event):
        """Handle window close"""
        if self.protocol:
            self.disconnect()
        event.accept()

def main():
    """Main application entry point"""
    try:
        app = QApplication(sys.argv)
        
        # Set application style
        app.setStyle('Fusion')
        
        # Create and show main window
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

