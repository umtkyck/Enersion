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
    """Worker thread for health monitoring"""
    
    health_updated = pyqtSignal(int, dict)  # mcu_id, health_data
    connection_lost = pyqtSignal(int)  # mcu_id
    
    def __init__(self, protocol: RS485Protocol):
        super().__init__()
        self.protocol = protocol
        self.running = False
        self.mcu_addresses = [
            RS485_ADDR_CONTROLLER_420,
            RS485_ADDR_CONTROLLER_DIO,
            RS485_ADDR_CONTROLLER_OUT
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

class DigitalIOWidget(QGroupBox):
    """Widget for digital I/O control"""
    
    def __init__(self, protocol: RS485Protocol):
        super().__init__("Digital I/O Control")
        self.protocol = protocol
        self.init_ui()
    
    def init_ui(self):
        """Initialize UI"""
        layout = QVBoxLayout()
        
        # Digital Inputs
        di_group = QGroupBox("Digital Inputs (DIO Controller) - 56 channels")
        di_scroll = QScrollArea()
        di_scroll.setWidgetResizable(True)
        di_scroll.setMinimumHeight(200)
        di_scroll.setMaximumHeight(300)
        di_widget = QWidget()
        di_layout = QGridLayout()
        di_layout.setSpacing(3)
        
        self.di_indicators = []
        for i in range(56):
            label = QLabel(f"DI{i}:")
            label.setMinimumWidth(35)
            indicator = QLabel("○")
            indicator.setStyleSheet("font-size: 16px;")
            di_layout.addWidget(label, i // 8, (i % 8) * 2)
            di_layout.addWidget(indicator, i // 8, (i % 8) * 2 + 1)
            self.di_indicators.append(indicator)
        
        di_widget.setLayout(di_layout)
        di_scroll.setWidget(di_widget)
        di_group_layout = QVBoxLayout()
        di_group_layout.addWidget(di_scroll)
        di_group.setLayout(di_group_layout)
        layout.addWidget(di_group)
        
        # Digital Outputs
        do_group = QGroupBox("Digital Outputs (OUT Controller) - 56 channels")
        do_scroll = QScrollArea()
        do_scroll.setWidgetResizable(True)
        do_scroll.setMinimumHeight(200)
        do_scroll.setMaximumHeight(300)
        do_widget = QWidget()
        do_layout = QGridLayout()
        do_layout.setSpacing(3)
        
        self.do_checkboxes = []
        for i in range(56):
            checkbox = QCheckBox(f"DO{i}")
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
        
        self.read_di_btn = QPushButton("Read Inputs")
        self.read_di_btn.clicked.connect(self.read_inputs)
        button_layout.addWidget(self.read_di_btn)
        
        self.write_do_btn = QPushButton("Write Outputs")
        self.write_do_btn.clicked.connect(self.write_outputs)
        button_layout.addWidget(self.write_do_btn)
        
        self.read_do_btn = QPushButton("Read Outputs")
        self.read_do_btn.clicked.connect(self.read_outputs)
        button_layout.addWidget(self.read_do_btn)
        
        layout.addLayout(button_layout)
        
        self.setLayout(layout)
    
    def read_inputs(self):
        """Read digital inputs (56 channels)"""
        try:
            data = self.protocol.read_digital_inputs(RS485_ADDR_CONTROLLER_DIO)
            if data:
                # Update indicators (56 inputs = 7 bytes)
                for i in range(56):
                    byte_idx = i // 8
                    bit_idx = i % 8
                    if byte_idx < len(data):
                        state = (data[byte_idx] >> bit_idx) & 0x01
                        indicator = self.di_indicators[i]
                        if state:
                            indicator.setText("●")
                            indicator.setStyleSheet("color: green; font-size: 16px;")
                        else:
                            indicator.setText("○")
                            indicator.setStyleSheet("color: gray; font-size: 16px;")
        except Exception as e:
            QMessageBox.warning(self, "Error", f"Failed to read inputs: {e}")
    
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
            
            success = self.protocol.write_digital_outputs(RS485_ADDR_CONTROLLER_OUT, bytes(output_bytes))
            
            if success:
                QMessageBox.information(self, "Success", "Outputs written successfully (56 channels)")
            else:
                QMessageBox.warning(self, "Error", "Failed to write outputs")
                
        except Exception as e:
            QMessageBox.warning(self, "Error", f"Failed to write outputs: {e}")
    
    def read_outputs(self):
        """Read current output state (56 channels)"""
        try:
            data = self.protocol.read_digital_outputs(RS485_ADDR_CONTROLLER_OUT)
            if data:
                # Update checkboxes (56 outputs = 7 bytes)
                for i in range(56):
                    byte_idx = i // 8
                    bit_idx = i % 8
                    if byte_idx < len(data):
                        state = (data[byte_idx] >> bit_idx) & 0x01
                        if i < len(self.do_checkboxes):
                            self.do_checkboxes[i].setChecked(bool(state))
        except Exception as e:
            QMessageBox.warning(self, "Error", f"Failed to read outputs: {e}")
    
    def on_output_changed(self):
        """Handle output checkbox change"""
        # Could auto-write on change if desired
        pass

class MainWindow(QMainWindow):
    """Main Application Window"""
    
    def __init__(self):
        super().__init__()
        self.protocol = None
        self.health_worker = None
        self.health_thread = None
        
        self.init_ui()
        self.refresh_ports()
    
    def init_ui(self):
        """Initialize user interface"""
        self.setWindowTitle(get_version_string())
        self.setMinimumSize(1200, 900)  # Increased for better visibility of tabs
        
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)
        
        # Connection panel
        conn_group = QGroupBox("Connection")
        conn_layout = QHBoxLayout()
        
        conn_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(150)
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
        self.connect_btn.clicked.connect(self.toggle_connection)
        conn_layout.addWidget(self.connect_btn)
        
        conn_layout.addStretch()
        
        conn_group.setLayout(conn_layout)
        main_layout.addWidget(conn_group)
        
        # MCU Status panels
        mcu_layout = QHBoxLayout()
        
        self.mcu_420_widget = MCUWidget("Controller 420 (4-20mA)", RS485_ADDR_CONTROLLER_420)
        mcu_layout.addWidget(self.mcu_420_widget)
        
        self.mcu_dio_widget = MCUWidget("Controller DIO (Inputs)", RS485_ADDR_CONTROLLER_DIO)
        mcu_layout.addWidget(self.mcu_dio_widget)
        
        self.mcu_out_widget = MCUWidget("Controller OUT (Outputs)", RS485_ADDR_CONTROLLER_OUT)
        mcu_layout.addWidget(self.mcu_out_widget)
        
        self.mcu_widgets = {
            RS485_ADDR_CONTROLLER_420: self.mcu_420_widget,
            RS485_ADDR_CONTROLLER_DIO: self.mcu_dio_widget,
            RS485_ADDR_CONTROLLER_OUT: self.mcu_out_widget
        }
        
        main_layout.addLayout(mcu_layout)
        
        # Analog input display (will be enabled when connected)
        self.analog_widget = None
        
        # Digital I/O control (will be enabled when connected)
        self.dio_widget = None
        
        # Status bar
        self.statusBar().showMessage("Ready")
        
        # Menu bar
        menubar = self.menuBar()
        
        # File menu
        file_menu = menubar.addMenu("File")
        
        exit_action = QAction("Exit", self)
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)
        
        # Tools menu
        tools_menu = menubar.addMenu("Tools")
        
        scan_action = QAction("Scan Devices", self)
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
                self.port_combo.setEnabled(False)
                self.baud_combo.setEnabled(False)
                self.refresh_btn.setEnabled(False)
                
                self.statusBar().showMessage(f"Connected to {port}")
                
                # Add Analog Input widget
                if self.analog_widget is None:
                    self.analog_widget = AnalogInputWidget(self.protocol)
                    self.centralWidget().layout().addWidget(self.analog_widget)
                
                # Add Digital I/O widget
                if self.dio_widget is None:
                    self.dio_widget = DigitalIOWidget(self.protocol)
                    self.centralWidget().layout().addWidget(self.dio_widget)
                
                # Start health monitoring
                self.start_health_monitoring()
                
                # Initial device scan
                QTimer.singleShot(500, self.scan_devices)
                
            else:
                QMessageBox.critical(self, "Error", "Failed to connect")
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
        self.port_combo.setEnabled(True)
        self.baud_combo.setEnabled(True)
        self.refresh_btn.setEnabled(True)
        
        # Update MCU widgets
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
        """Scan for connected devices"""
        if not self.protocol or not self.protocol.is_connected():
            QMessageBox.warning(self, "Error", "Not connected")
            return
        
        self.statusBar().showMessage("Scanning devices...")
        QApplication.processEvents()
        
        found_count = 0
        
        for mcu_addr, widget in self.mcu_widgets.items():
            # Ping device
            if self.protocol.ping(mcu_addr):
                widget.update_connection(True)
                found_count += 1
                
                # Get version
                version = self.protocol.get_version(mcu_addr)
                if version:
                    widget.update_version(version)
            else:
                widget.update_connection(False)
        
        self.statusBar().showMessage(f"Scan complete. Found {found_count} device(s)")
    
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
    app = QApplication(sys.argv)
    
    # Set application style
    app.setStyle('Fusion')
    
    # Create and show main window
    window = MainWindow()
    window.show()
    
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()

