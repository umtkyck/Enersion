#!/usr/bin/env python3
"""
RS485 Hardware Abstraction Layer for MYIR STM32MP257

This module provides RS485 serial communication with GPIO direction control.
Device: /dev/ttySTM9
GPIO: PI10 (138) for TX/RX direction

Author: Enersion
Version: 1.0.0
"""

import os
import serial
import time
import logging
from typing import Optional, Tuple
from dataclasses import dataclass
from enum import Enum

logger = logging.getLogger(__name__)


class RS485Error(Exception):
    """RS485 communication error"""
    pass


class Parity(Enum):
    """Serial parity options"""
    NONE = 'N'
    EVEN = 'E'
    ODD = 'O'


@dataclass
class RS485Config:
    """RS485 configuration parameters"""
    device: str = "/dev/ttySTM9"
    baudrate: int = 115200
    data_bits: int = 8
    parity: Parity = Parity.NONE
    stop_bits: int = 1
    timeout: float = 0.5
    gpio_pin: int = 138  # PI10 for direction control


class RS485GPIO:
    """
    GPIO control for RS485 direction (TX/RX)
    
    MYIR STM32MP257:
    - GPIO 138 (PI10) = nRTS
    - LOW (0) = Receive mode
    - HIGH (1) = Transmit mode
    """
    
    GPIO_EXPORT = "/sys/class/gpio/export"
    GPIO_UNEXPORT = "/sys/class/gpio/unexport"
    GPIO_BASE = "/sys/class/gpio"
    
    def __init__(self, pin: int = 138):
        self.pin = pin
        self.gpio_path = f"{self.GPIO_BASE}/gpio{pin}"
        self.alt_gpio_path = f"{self.GPIO_BASE}/PI10"
        self._initialized = False
        
    def _get_gpio_path(self) -> str:
        """Get the correct GPIO path (gpio138 or PI10)"""
        if os.path.exists(f"{self.gpio_path}/value"):
            return self.gpio_path
        elif os.path.exists(f"{self.alt_gpio_path}/value"):
            return self.alt_gpio_path
        return self.gpio_path
    
    def _write_file(self, path: str, value: str) -> bool:
        """Write value to sysfs file"""
        try:
            with open(path, 'w') as f:
                f.write(value)
            return True
        except (IOError, PermissionError) as e:
            logger.warning(f"Failed to write {value} to {path}: {e}")
            return False
    
    def init(self) -> bool:
        """Initialize GPIO for RS485 direction control"""
        if self._initialized:
            return True
        
        # Check if already exported
        gpio_path = self._get_gpio_path()
        if not os.path.exists(f"{gpio_path}/value"):
            # Export GPIO
            if not self._write_file(self.GPIO_EXPORT, str(self.pin)):
                # May already be exported
                time.sleep(0.1)
                gpio_path = self._get_gpio_path()
                if not os.path.exists(f"{gpio_path}/value"):
                    logger.error(f"Failed to export GPIO {self.pin}")
                    return False
            time.sleep(0.1)  # Wait for sysfs
        
        # Set direction to output
        gpio_path = self._get_gpio_path()
        if not self._write_file(f"{gpio_path}/direction", "out"):
            logger.error("Failed to set GPIO direction")
            return False
        
        # Default to receive mode
        self.rx_enable()
        
        self._initialized = True
        logger.info(f"RS485 GPIO {self.pin} initialized")
        return True
    
    def deinit(self) -> None:
        """Deinitialize GPIO"""
        if not self._initialized:
            return
        
        self.rx_enable()
        self._write_file(self.GPIO_UNEXPORT, str(self.pin))
        self._initialized = False
    
    def tx_enable(self) -> bool:
        """Enable transmit mode (GPIO HIGH)"""
        gpio_path = self._get_gpio_path()
        return self._write_file(f"{gpio_path}/value", "1")
    
    def rx_enable(self) -> bool:
        """Enable receive mode (GPIO LOW)"""
        gpio_path = self._get_gpio_path()
        return self._write_file(f"{gpio_path}/value", "0")


class RS485:
    """
    RS485 Serial Communication Handler
    
    Provides half-duplex RS485 communication with automatic
    direction control via GPIO.
    
    Usage:
        rs485 = RS485(RS485Config())
        rs485.open()
        rs485.write(b'\\xAA\\x02\\x10\\x01...')
        response = rs485.read(expected_length=10)
        rs485.close()
    """
    
    def __init__(self, config: Optional[RS485Config] = None):
        self.config = config or RS485Config()
        self._serial: Optional[serial.Serial] = None
        self._gpio = RS485GPIO(self.config.gpio_pin)
        self._is_open = False
        
        # Statistics
        self.bytes_sent = 0
        self.bytes_received = 0
        self.tx_count = 0
        self.rx_count = 0
        self.error_count = 0
        self.timeout_count = 0
    
    @property
    def is_open(self) -> bool:
        """Check if port is open"""
        return self._is_open and self._serial is not None and self._serial.is_open
    
    def open(self) -> bool:
        """Open RS485 port"""
        if self.is_open:
            return True
        
        try:
            # Initialize GPIO
            if not self._gpio.init():
                logger.warning("GPIO init failed, continuing without direction control")
            
            # Open serial port
            self._serial = serial.Serial(
                port=self.config.device,
                baudrate=self.config.baudrate,
                bytesize=self.config.data_bits,
                parity=self.config.parity.value,
                stopbits=self.config.stop_bits,
                timeout=self.config.timeout,
                write_timeout=self.config.timeout
            )
            
            # Flush buffers
            self._serial.reset_input_buffer()
            self._serial.reset_output_buffer()
            
            self._is_open = True
            logger.info(f"RS485 opened: {self.config.device} @ {self.config.baudrate}")
            return True
            
        except serial.SerialException as e:
            logger.error(f"Failed to open RS485: {e}")
            self.error_count += 1
            return False
    
    def close(self) -> None:
        """Close RS485 port"""
        if self._serial:
            self._gpio.rx_enable()
            self._serial.close()
            self._serial = None
        
        self._gpio.deinit()
        self._is_open = False
        logger.info("RS485 closed")
    
    def write(self, data: bytes) -> int:
        """
        Write data to RS485 port
        
        Args:
            data: Bytes to send
            
        Returns:
            Number of bytes written
        """
        if not self.is_open:
            raise RS485Error("Port not open")
        
        try:
            # Enable TX mode
            self._gpio.tx_enable()
            time.sleep(0.0001)  # 100us settle time
            
            # Write data
            written = self._serial.write(data)
            
            # Wait for transmission
            self._serial.flush()
            
            # Switch back to RX mode
            time.sleep(0.0001)
            self._gpio.rx_enable()
            
            # Update stats
            self.bytes_sent += written
            self.tx_count += 1
            
            logger.debug(f"TX [{written}]: {data.hex()}")
            return written
            
        except serial.SerialException as e:
            self._gpio.rx_enable()
            self.error_count += 1
            raise RS485Error(f"Write failed: {e}")
    
    def read(self, size: int = 256, timeout: Optional[float] = None) -> bytes:
        """
        Read data from RS485 port
        
        Args:
            size: Maximum bytes to read
            timeout: Read timeout (None = use default)
            
        Returns:
            Received bytes
        """
        if not self.is_open:
            raise RS485Error("Port not open")
        
        try:
            # Set timeout if specified
            if timeout is not None:
                old_timeout = self._serial.timeout
                self._serial.timeout = timeout
            
            # Read data
            data = self._serial.read(size)
            
            # Restore timeout
            if timeout is not None:
                self._serial.timeout = old_timeout
            
            # Update stats
            if data:
                self.bytes_received += len(data)
                self.rx_count += 1
                logger.debug(f"RX [{len(data)}]: {data.hex()}")
            else:
                self.timeout_count += 1
            
            return data
            
        except serial.SerialException as e:
            self.error_count += 1
            raise RS485Error(f"Read failed: {e}")
    
    def read_until(self, expected: bytes, size: int = 256, 
                   timeout: Optional[float] = None) -> bytes:
        """Read until expected byte sequence or timeout"""
        if not self.is_open:
            raise RS485Error("Port not open")
        
        if timeout is not None:
            old_timeout = self._serial.timeout
            self._serial.timeout = timeout
        
        try:
            data = self._serial.read_until(expected, size)
            if data:
                self.bytes_received += len(data)
                self.rx_count += 1
            return data
        finally:
            if timeout is not None:
                self._serial.timeout = old_timeout
    
    def flush(self) -> None:
        """Flush input and output buffers"""
        if self._serial:
            self._serial.reset_input_buffer()
            self._serial.reset_output_buffer()
    
    def in_waiting(self) -> int:
        """Get number of bytes waiting to be read"""
        if self._serial:
            return self._serial.in_waiting
        return 0
    
    def reset_stats(self) -> None:
        """Reset communication statistics"""
        self.bytes_sent = 0
        self.bytes_received = 0
        self.tx_count = 0
        self.rx_count = 0
        self.error_count = 0
        self.timeout_count = 0


def list_serial_ports() -> list:
    """List available serial ports"""
    ports = []
    
    # Check common Linux serial devices
    dev_patterns = [
        "/dev/ttySTM",
        "/dev/ttyUSB",
        "/dev/ttyACM",
        "/dev/ttyS"
    ]
    
    for pattern in dev_patterns:
        for i in range(10):
            port = f"{pattern}{i}"
            if os.path.exists(port):
                ports.append(port)
    
    # Always include default RS485 port
    if "/dev/ttySTM9" not in ports:
        ports.insert(0, "/dev/ttySTM9")
    
    return ports


# Test code
if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    
    print("RS485 Test")
    print("=" * 40)
    
    config = RS485Config(
        device="/dev/ttySTM9",
        baudrate=115200
    )
    
    rs485 = RS485(config)
    
    if rs485.open():
        print("Port opened successfully")
        
        # Send test data
        test_data = b'\xAA\x02\x10\x01\x00\x55'
        rs485.write(test_data)
        print(f"Sent: {test_data.hex()}")
        
        # Read response
        response = rs485.read(32, timeout=1.0)
        print(f"Received: {response.hex() if response else 'timeout'}")
        
        rs485.close()
    else:
        print("Failed to open port")

