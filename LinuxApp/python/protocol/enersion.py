#!/usr/bin/env python3
"""
Enersion Protocol Implementation

Custom protocol for communication with DI/DO controllers.
- Controller DIO (0x02): 64 Digital Inputs
- Controller OUT (0x03): 64 Digital Outputs

Packet Format:
[START(0xAA)][DEST][SRC][CMD][LEN][DATA...][CRC16][END(0x55)]

Author: Enersion
Version: 1.0.0
"""

import struct
import time
import logging
from typing import Optional, Tuple, List
from dataclasses import dataclass, field
from enum import IntEnum

logger = logging.getLogger(__name__)


# =============================================================================
# Constants
# =============================================================================

START_BYTE = 0xAA
END_BYTE = 0x55
MAX_DATA_LEN = 250
HEADER_SIZE = 5  # Start + Dest + Src + Cmd + Len
CRC_SIZE = 2
FOOTER_SIZE = 1

DIO_CHANNEL_COUNT = 64
DIO_BYTE_COUNT = 8  # 64 bits = 8 bytes


# =============================================================================
# Device Addresses
# =============================================================================

class Address(IntEnum):
    """Device addresses on the RS485 bus"""
    BROADCAST = 0x00
    CTRL_420 = 0x01      # 4-20mA Controller
    CTRL_DIO = 0x02      # Digital Input Controller
    CTRL_OUT = 0x03      # Digital Output Controller
    MASTER = 0x10        # Master (Linux/GUI)


# =============================================================================
# Commands
# =============================================================================

class Command(IntEnum):
    """Protocol command codes"""
    # System Commands
    PING = 0x01
    PING_RESPONSE = 0x02
    GET_VERSION = 0x03
    VERSION_RESPONSE = 0x04
    HEARTBEAT = 0x05
    HEARTBEAT_RESPONSE = 0x06
    
    # Status Commands
    GET_STATUS = 0x10
    STATUS_RESPONSE = 0x11
    
    # Digital Input Commands
    READ_DI = 0x20
    DI_RESPONSE = 0x21
    
    # Digital Output Commands
    WRITE_DO = 0x30
    DO_RESPONSE = 0x31
    READ_DO = 0x32
    
    # Error Response
    ERROR_RESPONSE = 0xFF


# =============================================================================
# CRC-16 MODBUS
# =============================================================================

CRC_TABLE = [
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
]


def crc16(data: bytes) -> int:
    """Calculate CRC-16/MODBUS checksum"""
    crc = 0xFFFF
    for byte in data:
        crc = (crc >> 8) ^ CRC_TABLE[(crc ^ byte) & 0xFF]
    return crc


def verify_crc(data: bytes) -> bool:
    """Verify CRC-16 checksum in data"""
    if len(data) < 3:
        return False
    
    payload = data[:-2]
    received_crc = struct.unpack('<H', data[-2:])[0]
    calculated_crc = crc16(payload)
    
    return received_crc == calculated_crc


# =============================================================================
# Data Structures
# =============================================================================

@dataclass
class Packet:
    """Protocol packet structure"""
    dest_addr: int = 0
    src_addr: int = Address.MASTER
    command: int = 0
    data: bytes = b''
    
    def encode(self) -> bytes:
        """Encode packet to wire format"""
        # Header
        header = bytes([
            START_BYTE,
            self.dest_addr,
            self.src_addr,
            self.command,
            len(self.data)
        ])
        
        # CRC over header (excluding start) + data
        crc_data = header[1:] + self.data
        crc = crc16(crc_data)
        
        # Complete packet
        packet = header + self.data + struct.pack('<H', crc) + bytes([END_BYTE])
        return packet
    
    @classmethod
    def decode(cls, data: bytes) -> Optional['Packet']:
        """Decode packet from wire format"""
        if len(data) < HEADER_SIZE + CRC_SIZE + FOOTER_SIZE:
            logger.warning(f"Packet too short: {len(data)} bytes")
            return None
        
        if data[0] != START_BYTE:
            logger.warning(f"Invalid start byte: 0x{data[0]:02X}")
            return None
        
        if data[-1] != END_BYTE:
            logger.warning(f"Invalid end byte: 0x{data[-1]:02X}")
            return None
        
        # Parse header
        dest_addr = data[1]
        src_addr = data[2]
        command = data[3]
        data_len = data[4]
        
        # Validate length
        expected_len = HEADER_SIZE + data_len + CRC_SIZE + FOOTER_SIZE
        if len(data) != expected_len:
            logger.warning(f"Length mismatch: got {len(data)}, expected {expected_len}")
            return None
        
        # Verify CRC
        crc_start = HEADER_SIZE + data_len
        received_crc = struct.unpack('<H', data[crc_start:crc_start+2])[0]
        calculated_crc = crc16(data[1:crc_start])
        
        if received_crc != calculated_crc:
            logger.warning(f"CRC mismatch: got 0x{received_crc:04X}, expected 0x{calculated_crc:04X}")
            return None
        
        # Extract data
        payload = data[HEADER_SIZE:HEADER_SIZE + data_len]
        
        return cls(
            dest_addr=dest_addr,
            src_addr=src_addr,
            command=command,
            data=payload
        )


@dataclass
class Version:
    """Device version information"""
    major: int = 0
    minor: int = 0
    patch: int = 0
    build: int = 0
    mcu_id: int = 0
    
    def __str__(self) -> str:
        return f"v{self.major}.{self.minor}.{self.patch}.{self.build}"


@dataclass
class Status:
    """Device status information"""
    mcu_id: int = 0
    health: int = 0
    uptime: int = 0
    error_count: int = 0
    rx_packet_count: int = 0
    tx_packet_count: int = 0


@dataclass
class DIOState:
    """64-bit Digital I/O state"""
    state: bytearray = field(default_factory=lambda: bytearray(8))
    
    def get_bit(self, channel: int) -> bool:
        """Get state of a single channel (0-63)"""
        if 0 <= channel < 64:
            byte_idx = channel // 8
            bit_idx = channel % 8
            return bool(self.state[byte_idx] & (1 << bit_idx))
        return False
    
    def set_bit(self, channel: int, value: bool) -> None:
        """Set state of a single channel (0-63)"""
        if 0 <= channel < 64:
            byte_idx = channel // 8
            bit_idx = channel % 8
            if value:
                self.state[byte_idx] |= (1 << bit_idx)
            else:
                self.state[byte_idx] &= ~(1 << bit_idx)
    
    def set_all(self, value: bool) -> None:
        """Set all channels to same value"""
        fill = 0xFF if value else 0x00
        for i in range(8):
            self.state[i] = fill
    
    def get_active_count(self) -> int:
        """Count number of active (ON) channels"""
        count = 0
        for byte in self.state:
            count += bin(byte).count('1')
        return count
    
    def to_list(self) -> List[bool]:
        """Convert to list of 64 booleans"""
        return [self.get_bit(i) for i in range(64)]
    
    def from_list(self, values: List[bool]) -> None:
        """Set from list of booleans"""
        for i, value in enumerate(values[:64]):
            self.set_bit(i, value)
    
    def __str__(self) -> str:
        """String representation as binary"""
        result = ""
        for i in range(64):
            if i > 0 and i % 8 == 0:
                result += " "
            result += "1" if self.get_bit(i) else "0"
        return result


# =============================================================================
# Protocol Handler
# =============================================================================

class EnersionProtocol:
    """
    Enersion Protocol Handler
    
    Manages communication with DI/DO controllers via RS485.
    
    Usage:
        from hal import RS485, RS485Config
        from protocol import EnersionProtocol
        
        rs485 = RS485(RS485Config())
        rs485.open()
        
        protocol = EnersionProtocol(rs485)
        
        # Ping controller
        if protocol.ping(Address.CTRL_DIO):
            print("DI Controller online")
        
        # Read inputs
        state = protocol.read_digital_inputs()
        print(f"Active inputs: {state.get_active_count()}")
        
        # Write outputs
        output_state = DIOState()
        output_state.set_bit(0, True)
        protocol.write_digital_outputs(output_state)
    """
    
    DEFAULT_TIMEOUT = 0.5
    DEFAULT_RETRIES = 3
    
    def __init__(self, rs485, my_address: int = Address.MASTER):
        self.rs485 = rs485
        self.my_address = my_address
        self.timeout = self.DEFAULT_TIMEOUT
        self.retries = self.DEFAULT_RETRIES
    
    def set_timeout(self, timeout: float) -> None:
        """Set response timeout in seconds"""
        self.timeout = timeout
    
    def set_retries(self, retries: int) -> None:
        """Set retry count"""
        self.retries = max(0, retries)
    
    def _transaction(self, request: Packet) -> Optional[Packet]:
        """Send request and wait for response"""
        for attempt in range(self.retries + 1):
            try:
                # Flush input buffer
                self.rs485.flush()
                
                # Send request
                tx_data = request.encode()
                self.rs485.write(tx_data)
                
                logger.debug(f"TX: {tx_data.hex()}")
                
                # Wait for response
                time.sleep(0.01)  # Small delay before reading
                
                # Read response
                response_data = bytearray()
                start_time = time.time()
                
                while time.time() - start_time < self.timeout:
                    if self.rs485.in_waiting() > 0:
                        chunk = self.rs485.read(256)
                        response_data.extend(chunk)
                        
                        # Check if we have a complete packet
                        if len(response_data) >= HEADER_SIZE:
                            expected_len = HEADER_SIZE + response_data[4] + CRC_SIZE + FOOTER_SIZE
                            if len(response_data) >= expected_len:
                                break
                    else:
                        time.sleep(0.01)
                
                if not response_data:
                    logger.debug(f"Timeout (attempt {attempt + 1})")
                    continue
                
                logger.debug(f"RX: {bytes(response_data).hex()}")
                
                # Decode response
                response = Packet.decode(bytes(response_data))
                if response:
                    return response
                
            except Exception as e:
                logger.warning(f"Transaction error (attempt {attempt + 1}): {e}")
        
        return None
    
    # =========================================================================
    # System Commands
    # =========================================================================
    
    def ping(self, address: int) -> bool:
        """Ping device and check if online"""
        request = Packet(
            dest_addr=address,
            src_addr=self.my_address,
            command=Command.PING,
            data=b''
        )
        
        response = self._transaction(request)
        return response is not None and response.command == Command.PING_RESPONSE
    
    def get_version(self, address: int) -> Optional[Version]:
        """Get device firmware version"""
        request = Packet(
            dest_addr=address,
            src_addr=self.my_address,
            command=Command.GET_VERSION,
            data=b''
        )
        
        response = self._transaction(request)
        
        if response and response.command == Command.VERSION_RESPONSE and len(response.data) >= 5:
            return Version(
                major=response.data[0],
                minor=response.data[1],
                patch=response.data[2],
                build=response.data[3],
                mcu_id=response.data[4]
            )
        
        return None
    
    def get_status(self, address: int) -> Optional[Status]:
        """Get device status"""
        request = Packet(
            dest_addr=address,
            src_addr=self.my_address,
            command=Command.GET_STATUS,
            data=b''
        )
        
        response = self._transaction(request)
        
        if response and response.command == Command.STATUS_RESPONSE and len(response.data) >= 16:
            return Status(
                mcu_id=response.data[0],
                health=response.data[1],
                uptime=struct.unpack('<I', response.data[2:6])[0],
                error_count=struct.unpack('<I', response.data[6:10])[0],
                rx_packet_count=struct.unpack('<I', response.data[10:14])[0],
                tx_packet_count=struct.unpack('<H', response.data[14:16])[0]
            )
        
        return None
    
    def heartbeat(self, address: int) -> Optional[int]:
        """Send heartbeat and get health percentage"""
        request = Packet(
            dest_addr=address,
            src_addr=self.my_address,
            command=Command.HEARTBEAT,
            data=b''
        )
        
        response = self._transaction(request)
        
        if response and response.command == Command.HEARTBEAT_RESPONSE and len(response.data) >= 2:
            return response.data[1]  # Health percentage
        
        return None
    
    # =========================================================================
    # Digital Input Commands
    # =========================================================================
    
    def read_digital_inputs(self) -> Optional[DIOState]:
        """Read all 64 digital inputs"""
        request = Packet(
            dest_addr=Address.CTRL_DIO,
            src_addr=self.my_address,
            command=Command.READ_DI,
            data=b''
        )
        
        response = self._transaction(request)
        
        if response and response.command == Command.DI_RESPONSE and len(response.data) >= 8:
            state = DIOState()
            state.state = bytearray(response.data[:8])
            return state
        
        return None
    
    # =========================================================================
    # Digital Output Commands
    # =========================================================================
    
    def write_digital_outputs(self, state: DIOState) -> bool:
        """Write all 64 digital outputs"""
        request = Packet(
            dest_addr=Address.CTRL_OUT,
            src_addr=self.my_address,
            command=Command.WRITE_DO,
            data=bytes(state.state)
        )
        
        response = self._transaction(request)
        return response is not None and response.command == Command.DO_RESPONSE
    
    def read_digital_outputs(self) -> Optional[DIOState]:
        """Read current digital output states"""
        request = Packet(
            dest_addr=Address.CTRL_OUT,
            src_addr=self.my_address,
            command=Command.READ_DO,
            data=b''
        )
        
        response = self._transaction(request)
        
        if response and response.command == Command.DO_RESPONSE and len(response.data) >= 8:
            state = DIOState()
            state.state = bytearray(response.data[:8])
            return state
        
        return None


# Test code
if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    
    print("Enersion Protocol Test")
    print("=" * 40)
    
    # Test packet encoding
    packet = Packet(
        dest_addr=Address.CTRL_DIO,
        src_addr=Address.MASTER,
        command=Command.PING,
        data=b''
    )
    
    encoded = packet.encode()
    print(f"Encoded PING: {encoded.hex()}")
    
    # Test packet decoding
    decoded = Packet.decode(encoded)
    if decoded:
        print(f"Decoded: dest=0x{decoded.dest_addr:02X}, cmd=0x{decoded.command:02X}")
    
    # Test DIO state
    state = DIOState()
    state.set_bit(0, True)
    state.set_bit(7, True)
    state.set_bit(63, True)
    print(f"DIO State: {state}")
    print(f"Active count: {state.get_active_count()}")

