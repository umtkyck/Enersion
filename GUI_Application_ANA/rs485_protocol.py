"""
******************************************************************************
@file           : rs485_protocol.py
@brief          : RS485 Communication Protocol Layer
******************************************************************************
@attention

Protocol Layer for RS485 communication
Handles packet encoding/decoding, CRC calculation, and command handling

******************************************************************************
"""

import struct
import serial
import threading
import time
from enum import IntEnum
from typing import Optional, Callable, Dict
from dataclasses import dataclass

# Protocol Constants
RS485_START_BYTE = 0xAA
RS485_END_BYTE = 0x55
RS485_TIMEOUT_MS = 100
RS485_MAX_PACKET_SIZE = 256

# MCU Address Definitions
RS485_ADDR_BROADCAST = 0x00
RS485_ADDR_CONTROLLER_420 = 0x01
RS485_ADDR_CONTROLLER_DIO = 0x02
RS485_ADDR_CONTROLLER_OUT = 0x03
RS485_ADDR_GUI = 0x10

# MCU Names
MCU_NAMES = {
    RS485_ADDR_CONTROLLER_420: "Controller 420",
    RS485_ADDR_CONTROLLER_DIO: "Controller DIO",
    RS485_ADDR_CONTROLLER_OUT: "Controller OUT"
}

class RS485Command(IntEnum):
    """RS485 Command Codes"""
    CMD_PING = 0x01
    CMD_PING_RESPONSE = 0x02
    CMD_GET_VERSION = 0x03
    CMD_VERSION_RESPONSE = 0x04
    CMD_HEARTBEAT = 0x05
    CMD_HEARTBEAT_RESPONSE = 0x06
    CMD_GET_STATUS = 0x10
    CMD_STATUS_RESPONSE = 0x11
    CMD_READ_DI = 0x20
    CMD_DI_RESPONSE = 0x21
    CMD_WRITE_DO = 0x30
    CMD_DO_RESPONSE = 0x31
    CMD_READ_DO = 0x32
    CMD_READ_ANALOG_420 = 0x40
    CMD_ANALOG_420_RESPONSE = 0x41
    CMD_READ_ANALOG_VOLTAGE = 0x42
    CMD_ANALOG_VOLTAGE_RESPONSE = 0x43
    CMD_READ_NTC = 0x44
    CMD_NTC_RESPONSE = 0x45
    CMD_READ_ALL_ANALOG = 0x46
    CMD_ALL_ANALOG_RESPONSE = 0x47
    CMD_ERROR_RESPONSE = 0xFF

class RS485Error(IntEnum):
    """RS485 Error Codes"""
    ERR_NONE = 0x00
    ERR_INVALID_CHECKSUM = 0x01
    ERR_INVALID_ADDRESS = 0x02
    ERR_INVALID_COMMAND = 0x03
    ERR_INVALID_LENGTH = 0x04
    ERR_TIMEOUT = 0x05
    ERR_BUSY = 0x06

@dataclass
class RS485Packet:
    """RS485 Packet Structure"""
    dest_addr: int
    src_addr: int
    command: int
    data: bytes
    
    def __post_init__(self):
        if len(self.data) > 250:
            raise ValueError("Data length must be <= 250 bytes")

@dataclass
class MCUStatus:
    """MCU Status Information"""
    mcu_id: int
    health: int
    uptime: int
    error_count: int
    rx_packet_count: int
    tx_packet_count: int
    
    @classmethod
    def from_bytes(cls, data: bytes):
        """Parse status from bytes"""
        if len(data) < 16:
            raise ValueError("Invalid status data length")
        
        mcu_id, health = struct.unpack('BB', data[0:2])
        uptime, error_count, rx_count = struct.unpack('<III', data[2:14])
        tx_count = struct.unpack('<H', data[14:16])[0]
        
        return cls(mcu_id, health, uptime, error_count, rx_count, tx_count)

@dataclass
class MCUVersion:
    """MCU Version Information"""
    major: int
    minor: int
    patch: int
    build: int
    mcu_id: int
    
    def __str__(self):
        return f"v{self.major}.{self.minor}.{self.patch}.{self.build}"
    
    @classmethod
    def from_bytes(cls, data: bytes):
        """Parse version from bytes"""
        if len(data) < 8:
            raise ValueError("Invalid version data length")
        
        major, minor, patch, build, mcu_id = struct.unpack('BBBBB', data[0:5])
        return cls(major, minor, patch, build, mcu_id)

class RS485Protocol:
    """
    RS485 Protocol Handler
    
    Manages RS485 communication including:
    - Packet encoding/decoding
    - CRC calculation
    - Command handling
    - Asynchronous communication
    """
    
    def __init__(self, port: str, baudrate: int = 115200):
        """
        Initialize RS485 protocol
        
        Args:
            port: Serial port name (e.g., 'COM3', '/dev/ttyUSB0')
            baudrate: Communication baud rate
        """
        self.port = port
        self.baudrate = baudrate
        self.serial: Optional[serial.Serial] = None
        self.my_address = RS485_ADDR_GUI
        
        # Communication statistics
        self.tx_count = 0
        self.rx_count = 0
        self.error_count = 0
        
        # Threading
        self.running = False
        self.rx_thread: Optional[threading.Thread] = None
        self.lock = threading.Lock()
        
        # Response handling
        self.response_handlers: Dict[int, Callable] = {}
        self.pending_responses: Dict[int, Optional[RS485Packet]] = {}
        
    def connect(self) -> bool:
        """
        Connect to RS485 port
        
        Returns:
            bool: True if connected successfully
        """
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.1
            )
            
            # Start receive thread
            self.running = True
            self.rx_thread = threading.Thread(target=self._receive_thread, daemon=True)
            self.rx_thread.start()
            
            return True
            
        except Exception as e:
            print(f"Connection error: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from RS485 port"""
        self.running = False
        
        if self.rx_thread:
            self.rx_thread.join(timeout=1.0)
        
        if self.serial and self.serial.is_open:
            self.serial.close()
    
    def is_connected(self) -> bool:
        """Check if connected"""
        return self.serial is not None and self.serial.is_open
    
    @staticmethod
    def calculate_crc(data: bytes) -> int:
        """
        Calculate CRC16 checksum
        
        Args:
            data: Data bytes
            
        Returns:
            int: CRC16 value
        """
        crc = 0xFFFF
        
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 0x0001:
                    crc = (crc >> 1) ^ 0xA001
                else:
                    crc >>= 1
        
        return crc
    
    def encode_packet(self, packet: RS485Packet) -> bytes:
        """
        Encode packet to bytes
        
        Args:
            packet: Packet to encode
            
        Returns:
            bytes: Encoded packet
        """
        # Build CRC buffer
        crc_buffer = struct.pack('BBBB', 
                                 packet.dest_addr,
                                 packet.src_addr,
                                 packet.command,
                                 len(packet.data))
        crc_buffer += packet.data
        
        # Calculate CRC
        crc = self.calculate_crc(crc_buffer)
        
        # Build complete packet
        encoded = struct.pack('BBBBB',
                             RS485_START_BYTE,
                             packet.dest_addr,
                             packet.src_addr,
                             packet.command,
                             len(packet.data))
        encoded += packet.data
        encoded += struct.pack('<HB', crc, RS485_END_BYTE)
        
        return encoded
    
    def decode_packet(self, data: bytes) -> Optional[RS485Packet]:
        """
        Decode packet from bytes
        
        Args:
            data: Raw packet bytes
            
        Returns:
            RS485Packet or None if invalid
        """
        if len(data) < 8:
            return None
        
        # Check start and end bytes
        if data[0] != RS485_START_BYTE or data[-1] != RS485_END_BYTE:
            return None
        
        # Parse header
        dest_addr, src_addr, command, length = struct.unpack('BBBB', data[1:5])
        
        # Extract data
        if len(data) < (5 + length + 3):
            return None
        
        payload = data[5:5+length]
        
        # Extract and verify CRC
        crc_received = struct.unpack('<H', data[5+length:5+length+2])[0]
        
        # Calculate CRC
        crc_buffer = data[1:5+length]
        crc_calculated = self.calculate_crc(crc_buffer)
        
        if crc_received != crc_calculated:
            self.error_count += 1
            print(f"CRC Error: Expected 0x{crc_calculated:04X}, Got 0x{crc_received:04X}")
            return None
        
        return RS485Packet(dest_addr, src_addr, command, payload)
    
    def send_packet(self, dest_addr: int, command: RS485Command, 
                   data: bytes = b'') -> bool:
        """
        Send RS485 packet
        
        Args:
            dest_addr: Destination address
            command: Command code
            data: Data payload
            
        Returns:
            bool: True if sent successfully
        """
        if not self.is_connected():
            return False
        
        try:
            packet = RS485Packet(dest_addr, self.my_address, command, data)
            encoded = self.encode_packet(packet)
            
            with self.lock:
                self.serial.write(encoded)
                self.serial.flush()  # Force immediate transmission
                time.sleep(0.02)  # 20ms delay for MCU processing
                self.tx_count += 1
            
            return True
            
        except Exception as e:
            print(f"Send error: {e}")
            self.error_count += 1
            return False
    
    def send_command_and_wait(self, dest_addr: int, command: RS485Command,
                             data: bytes = b'', timeout: float = 1.0) -> Optional[RS485Packet]:
        """
        Send command and wait for response
        
        Args:
            dest_addr: Destination address
            command: Command code
            data: Data payload
            timeout: Timeout in seconds
            
        Returns:
            RS485Packet or None if timeout
        """
        # Clear pending response
        self.pending_responses[dest_addr] = None
        
        # Send packet
        if not self.send_packet(dest_addr, command, data):
            return None
        
        # Wait for response
        start_time = time.time()
        while (time.time() - start_time) < timeout:
            if self.pending_responses.get(dest_addr) is not None:
                response = self.pending_responses[dest_addr]
                self.pending_responses[dest_addr] = None
                return response
            time.sleep(0.01)
        
        return None
    
    def _receive_thread(self):
        """Background receive thread"""
        rx_buffer = bytearray()
        
        while self.running:
            try:
                if self.serial and self.serial.in_waiting > 0:
                    byte = self.serial.read(1)
                    
                    if not byte:
                        continue
                    
                    # Look for start byte
                    if len(rx_buffer) == 0:
                        if byte[0] == RS485_START_BYTE:
                            rx_buffer.append(byte[0])
                    else:
                        rx_buffer.append(byte[0])
                        
                        # Check if we have length field
                        if len(rx_buffer) == 5:
                            expected_length = rx_buffer[4]
                            
                        # Check if packet complete
                        if len(rx_buffer) >= 8:
                            expected_length = rx_buffer[4]
                            expected_total = 5 + expected_length + 3
                            
                            if len(rx_buffer) >= expected_total:
                                # Try to decode packet
                                packet = self.decode_packet(bytes(rx_buffer))
                                
                                if packet:
                                    self.rx_count += 1
                                    self._handle_received_packet(packet)
                                
                                rx_buffer.clear()
                        
                        # Prevent buffer overflow
                        if len(rx_buffer) > RS485_MAX_PACKET_SIZE:
                            rx_buffer.clear()
                            
                else:
                    time.sleep(0.001)
                    
            except Exception as e:
                print(f"Receive error: {e}")
                self.error_count += 1
                rx_buffer.clear()
    
    def _handle_received_packet(self, packet: RS485Packet):
        """Handle received packet"""
        # Check if packet is for us
        if packet.dest_addr != self.my_address and packet.dest_addr != RS485_ADDR_BROADCAST:
            return
        
        # Store response for waiting commands
        self.pending_responses[packet.src_addr] = packet
        
        # Call registered handler
        if packet.command in self.response_handlers:
            self.response_handlers[packet.command](packet)
    
    def register_handler(self, command: RS485Command, handler: Callable):
        """
        Register command handler
        
        Args:
            command: Command code
            handler: Handler function
        """
        self.response_handlers[command] = handler
    
    # High-level command functions
    
    def ping(self, dest_addr: int) -> bool:
        """Ping MCU"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_PING)
        return response is not None and response.command == RS485Command.CMD_PING_RESPONSE
    
    def get_version(self, dest_addr: int) -> Optional[MCUVersion]:
        """Get MCU version"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_GET_VERSION)
        
        if response and response.command == RS485Command.CMD_VERSION_RESPONSE:
            try:
                return MCUVersion.from_bytes(response.data)
            except Exception as e:
                print(f"Version parse error: {e}")
        
        return None
    
    def get_status(self, dest_addr: int) -> Optional[MCUStatus]:
        """Get MCU status"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_GET_STATUS)
        
        if response and response.command == RS485Command.CMD_STATUS_RESPONSE:
            try:
                return MCUStatus.from_bytes(response.data)
            except Exception as e:
                print(f"Status parse error: {e}")
        
        return None
    
    def heartbeat(self, dest_addr: int) -> Optional[tuple]:
        """Send heartbeat"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_HEARTBEAT)
        
        if response and response.command == RS485Command.CMD_HEARTBEAT_RESPONSE:
            if len(response.data) >= 2:
                mcu_id, health = struct.unpack('BB', response.data[0:2])
                return (mcu_id, health)
        
        return None
    
    def read_digital_inputs(self, dest_addr: int) -> Optional[bytes]:
        """Read digital inputs"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_READ_DI)
        
        if response and response.command == RS485Command.CMD_DI_RESPONSE:
            return response.data
        
        return None
    
    def write_digital_outputs(self, dest_addr: int, outputs: bytes) -> bool:
        """Write digital outputs"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_WRITE_DO, outputs, timeout=2.0)
        return response is not None and response.command == RS485Command.CMD_DO_RESPONSE
    
    def read_digital_outputs(self, dest_addr: int) -> Optional[bytes]:
        """Read current digital output state"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_READ_DO)
        
        if response and response.command == RS485Command.CMD_DO_RESPONSE:
            return response.data
        
        return None
    
    def read_analog_420mA(self, dest_addr: int) -> Optional[list]:
        """Read 26x 4-20mA analog inputs"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_READ_ANALOG_420)
        
        if response and response.command == RS485Command.CMD_ANALOG_420_RESPONSE:
            # Parse 26 channels * 4 bytes each (4 float)
            channels = []
            for i in range(26):
                offset = i * 4
                if offset + 4 <= len(response.data):
                    current = struct.unpack('<f', response.data[offset:offset+4])[0]
                    channels.append({'raw': 0, 'current_mA': current})
            return channels
        
        return None
    
    def read_analog_voltage(self, dest_addr: int) -> Optional[list]:
        """Read 6x 0-10V analog inputs"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_READ_ANALOG_VOLTAGE)
        
        if response and response.command == RS485Command.CMD_ANALOG_VOLTAGE_RESPONSE:
            # Parse 6 channels * 4 bytes each
            channels = []
            for i in range(6):
                offset = i * 4
                if offset + 4 <= len(response.data):
                    voltage = struct.unpack('<f', response.data[offset:offset+4])[0]
                    channels.append({'raw': 0, 'voltage_V': voltage})
            return channels
        
        return None
    
    def read_ntc_temperatures(self, dest_addr: int) -> Optional[list]:
        """Read 4x NTC temperature sensors"""
        response = self.send_command_and_wait(dest_addr, RS485Command.CMD_READ_NTC)
        
        if response and response.command == RS485Command.CMD_NTC_RESPONSE:
            # Parse 4 channels * 4 bytes each
            channels = []
            for i in range(4):
                offset = i * 4
                if offset + 4 <= len(response.data):
                    temp = struct.unpack('<f', response.data[offset:offset+4])[0]
                    channels.append({'raw': 0, 'temperature_C': temp})
            return channels
        
        return None

