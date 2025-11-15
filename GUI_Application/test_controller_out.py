"""
******************************************************************************
@file           : test_controller_out.py
@brief          : Simple command-line test script for Controller OUT
******************************************************************************
@attention

Quick test script to verify RS485 communication with Controller OUT
without GUI. Useful for debugging and automated testing.

******************************************************************************
"""

import sys
import time
from rs485_protocol import *

def print_header():
    """Print test header"""
    print("=" * 60)
    print("Controller OUT Test Script")
    print("RS485 Digital Output Module (Address: 0x03)")
    print("=" * 60)
    print()

def list_ports():
    """List available serial ports"""
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    
    if not ports:
        print("No serial ports found!")
        return None
    
    print("Available COM Ports:")
    for i, port in enumerate(ports, 1):
        print(f"  {i}. {port.device} - {port.description}")
    print()
    
    return ports

def connect_to_controller(port, baudrate=115200):
    """Connect to Controller OUT"""
    print(f"Connecting to {port} @ {baudrate} baud...")
    
    protocol = RS485Protocol(port, baudrate)
    
    if protocol.connect():
        print("✓ Serial port opened successfully")
        time.sleep(0.5)  # Give it time to stabilize
        return protocol
    else:
        print("✗ Failed to open serial port")
        return None

def test_ping(protocol):
    """Test PING command"""
    print("\n[TEST 1] PING Controller OUT (0x03)")
    print("-" * 40)
    
    result = protocol.ping(RS485_ADDR_CONTROLLER_OUT)
    
    if result:
        print("✓ Controller OUT responded to PING")
        return True
    else:
        print("✗ No response from Controller OUT")
        return False

def test_get_version(protocol):
    """Test GET_VERSION command"""
    print("\n[TEST 2] Get Firmware Version")
    print("-" * 40)
    
    version = protocol.get_version(RS485_ADDR_CONTROLLER_OUT)
    
    if version:
        print(f"✓ Version: {version}")
        print(f"  MCU ID: {version.mcu_id}")
        return True
    else:
        print("✗ Failed to read version")
        return False

def test_heartbeat(protocol):
    """Test HEARTBEAT command"""
    print("\n[TEST 3] Heartbeat Test")
    print("-" * 40)
    
    result = protocol.heartbeat(RS485_ADDR_CONTROLLER_OUT)
    
    if result:
        mcu_id, health = result
        print(f"✓ Heartbeat OK")
        print(f"  MCU ID: {mcu_id}")
        print(f"  Health: {health}%")
        return True
    else:
        print("✗ Heartbeat failed")
        return False

def test_get_status(protocol):
    """Test GET_STATUS command"""
    print("\n[TEST 4] Get Controller Status")
    print("-" * 40)
    
    status = protocol.get_status(RS485_ADDR_CONTROLLER_OUT)
    
    if status:
        print(f"✓ Status retrieved:")
        print(f"  Health: {status.health}%")
        print(f"  Uptime: {status.uptime}s ({status.uptime//3600}h {(status.uptime%3600)//60}m)")
        print(f"  RX Packets: {status.rx_packet_count}")
        print(f"  TX Packets: {status.tx_packet_count}")
        print(f"  Errors: {status.error_count}")
        return True
    else:
        print("✗ Failed to get status")
        return False

def test_write_outputs(protocol):
    """Test WRITE_DO command"""
    print("\n[TEST 5] Write Digital Outputs")
    print("-" * 40)
    
    # Test pattern 1: Turn on every other output (DO0, DO2, DO4, ...)
    print("Pattern 1: Alternating outputs (DO0, DO2, DO4, ...)")
    pattern1 = bytes([0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55])
    
    success = protocol.write_digital_outputs(RS485_ADDR_CONTROLLER_OUT, pattern1)
    
    if success:
        print("✓ Pattern 1 written successfully")
    else:
        print("✗ Failed to write pattern 1")
        return False
    
    time.sleep(2)
    
    # Test pattern 2: Turn on first 8 outputs
    print("\nPattern 2: First 8 outputs ON (DO0-DO7)")
    pattern2 = bytes([0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    
    success = protocol.write_digital_outputs(RS485_ADDR_CONTROLLER_OUT, pattern2)
    
    if success:
        print("✓ Pattern 2 written successfully")
    else:
        print("✗ Failed to write pattern 2")
        return False
    
    time.sleep(2)
    
    # Test pattern 3: All OFF
    print("\nPattern 3: All outputs OFF")
    pattern3 = bytes([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    
    success = protocol.write_digital_outputs(RS485_ADDR_CONTROLLER_OUT, pattern3)
    
    if success:
        print("✓ Pattern 3 written successfully (all OFF)")
        return True
    else:
        print("✗ Failed to write pattern 3")
        return False

def test_read_outputs(protocol):
    """Test READ_DO command"""
    print("\n[TEST 6] Read Digital Outputs")
    print("-" * 40)
    
    # First write a known pattern
    test_pattern = bytes([0x0F, 0xF0, 0x33, 0xCC, 0x55, 0xAA, 0x00])
    print("Writing test pattern: 0x0F 0xF0 0x33 0xCC 0x55 0xAA 0x00")
    
    protocol.write_digital_outputs(RS485_ADDR_CONTROLLER_OUT, test_pattern)
    time.sleep(0.5)
    
    # Read it back
    data = protocol.read_digital_outputs(RS485_ADDR_CONTROLLER_OUT)
    
    if data:
        print(f"✓ Read {len(data)} bytes:")
        print(f"  Data: {' '.join(f'0x{b:02X}' for b in data)}")
        
        # Display active outputs
        active = []
        for i in range(56):
            byte_idx = i // 8
            bit_idx = i % 8
            if byte_idx < len(data):
                if (data[byte_idx] >> bit_idx) & 0x01:
                    active.append(i)
        
        print(f"  Active outputs ({len(active)}): {active if len(active) <= 20 else f'{active[:20]}...'}")
        return True
    else:
        print("✗ Failed to read outputs")
        return False

def main():
    """Main test function"""
    print_header()
    
    # List ports
    ports = list_ports()
    if not ports:
        return 1
    
    # Select port
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        try:
            choice = int(input("Select port number: "))
            port = ports[choice - 1].device
        except (ValueError, IndexError):
            print("Invalid selection!")
            return 1
    
    # Connect
    protocol = connect_to_controller(port)
    if not protocol:
        return 1
    
    try:
        # Run tests
        tests = [
            test_ping,
            test_get_version,
            test_heartbeat,
            test_get_status,
            test_write_outputs,
            test_read_outputs
        ]
        
        passed = 0
        failed = 0
        
        for test in tests:
            try:
                if test(protocol):
                    passed += 1
                else:
                    failed += 1
            except Exception as e:
                print(f"✗ Test exception: {e}")
                failed += 1
            
            time.sleep(0.5)
        
        # Summary
        print("\n" + "=" * 60)
        print("Test Summary")
        print("=" * 60)
        print(f"Total Tests: {passed + failed}")
        print(f"Passed: {passed}")
        print(f"Failed: {failed}")
        print(f"Success Rate: {(passed/(passed+failed)*100):.1f}%")
        print()
        
        if failed == 0:
            print("✓ All tests passed! Controller OUT is working correctly.")
            return 0
        else:
            print(f"✗ {failed} test(s) failed. Check hardware and connections.")
            return 1
        
    finally:
        # Cleanup
        print("\nDisconnecting...")
        protocol.disconnect()
        print("Done.")

if __name__ == '__main__':
    sys.exit(main())


