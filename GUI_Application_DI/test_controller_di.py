"""
Simple test script for Controller DI (Digital Input)
Tests RS485 communication with DI controller
"""

import serial
import time
from rs485_protocol import RS485Protocol, RS485_ADDR_CONTROLLER_DIO, RS485_ADDR_GUI

def print_separator():
    print("=" * 60)

def test_ping(protocol):
    """Test PING command"""
    print("\n1. Testing PING...")
    result = protocol.ping(RS485_ADDR_CONTROLLER_DIO)
    if result:
        print("   PASS - Controller DI responded to PING")
    else:
        print("   FAIL - No response from Controller DI")
    return result

def test_get_version(protocol):
    """Test GET_VERSION command"""
    print("\n2. Testing GET_VERSION...")
    version = protocol.get_version(RS485_ADDR_CONTROLLER_DIO)
    if version:
        print(f"   PASS - Version: {version['major']}.{version['minor']}.{version['patch']}.{version['build']}")
    else:
        print("   FAIL - No version response")
    return version is not None

def test_heartbeat(protocol):
    """Test HEARTBEAT command"""
    print("\n3. Testing HEARTBEAT...")
    result = protocol.heartbeat(RS485_ADDR_CONTROLLER_DIO)
    if result:
        mcu_id, health = result
        print(f"   PASS - MCU ID: 0x{mcu_id:02X}, Health: {health}%")
    else:
        print("   FAIL - No heartbeat response")
    return result is not None

def test_get_status(protocol):
    """Test GET_STATUS command"""
    print("\n4. Testing GET_STATUS...")
    status = protocol.get_status(RS485_ADDR_CONTROLLER_DIO)
    if status:
        print(f"   PASS - Status:")
        print(f"      Uptime: {status.get('uptime', 0)} seconds")
        print(f"      TX Packets: {status.get('tx_packets', 0)}")
        print(f"      RX Packets: {status.get('rx_packets', 0)}")
        print(f"      Errors: {status.get('errors', 0)}")
    else:
        print("   FAIL - No status response")
    return status is not None

def test_read_di(protocol):
    """Test READ_DI command"""
    print("\n5. Testing READ_DI (Digital Inputs)...")
    data = protocol.read_digital_inputs(RS485_ADDR_CONTROLLER_DIO)
    if data:
        print(f"   PASS - Read {len(data)} bytes of digital input data")
        
        # Display input states
        active_count = 0
        active_inputs = []
        
        for i in range(56):
            byte_idx = i // 8
            bit_idx = i % 8
            if byte_idx < len(data):
                state = (data[byte_idx] >> bit_idx) & 0x01
                if state:
                    active_count += 1
                    active_inputs.append(i)
        
        print(f"   Active inputs ({active_count}/56): {active_inputs if active_inputs else 'None'}")
        
        # Display byte values
        print(f"   Raw data (hex): {' '.join(f'{b:02X}' for b in data)}")
    else:
        print("   FAIL - No digital input data received")
    return data is not None

def main():
    print_separator()
    print("Controller DI Test Script")
    print("Digital Input Controller (Address: 0x02)")
    print_separator()
    
    # Get COM port
    port = input("\nEnter COM port (e.g., COM8): ").strip()
    
    if not port:
        print("Error: No COM port specified")
        return
    
    try:
        print(f"\nOpening {port}...")
        protocol = RS485Protocol(port, RS485_ADDR_GUI)
        print("Connection established")
        
        # Run tests
        print_separator()
        print("Running Tests...")
        print_separator()
        
        results = []
        results.append(("PING", test_ping(protocol)))
        time.sleep(0.5)
        
        results.append(("GET_VERSION", test_get_version(protocol)))
        time.sleep(0.5)
        
        results.append(("HEARTBEAT", test_heartbeat(protocol)))
        time.sleep(0.5)
        
        results.append(("GET_STATUS", test_get_status(protocol)))
        time.sleep(0.5)
        
        results.append(("READ_DI", test_read_di(protocol)))
        
        # Summary
        print_separator()
        print("Test Summary:")
        print_separator()
        
        passed = sum(1 for _, result in results if result)
        total = len(results)
        
        for test_name, result in results:
            status = "PASS" if result else "FAIL"
            print(f"   {test_name:15s}: {status}")
        
        print(f"\nTotal: {passed}/{total} tests passed ({passed*100//total}%)")
        
        if passed == total:
            print("\nALL TESTS PASSED")
        else:
            print("\nSOME TESTS FAILED - Check connections and firmware")
        
        print_separator()
        
        # Close connection
        protocol.close()
        print("\nConnection closed")
        
    except serial.SerialException as e:
        print(f"\nSerial Error: {e}")
        print("Make sure the COM port is correct and not in use by another program")
    except Exception as e:
        print(f"\nError: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()

