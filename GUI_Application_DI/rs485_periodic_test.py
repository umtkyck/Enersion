"""
******************************************************************************
@file           : rs485_periodic_test.py
@brief          : Simple RS485 periodic message sender for testing
******************************************************************************
@attention

Sends simple test messages over RS485 every 1 second.
Use this to verify RS485 hardware is working.
Watch COM7 (serial console) to see MCU responses.

******************************************************************************
"""

import sys
import time
import serial
from rs485_protocol import *

def send_periodic_messages(port, baudrate=115200):
    """Send periodic test messages over RS485"""
    
    print("=" * 70)
    print("RS485 Periodic Test - Simple Message Sender")
    print("=" * 70)
    print(f"Port: {port}")
    print(f"Baud Rate: {baudrate}")
    print(f"Target: Controller OUT (Address 0x03)")
    print(f"Interval: 1 second")
    print()
    print("Watch COM7 (PuTTY) to see MCU responses!")
    print("Press Ctrl+C to stop")
    print("=" * 70)
    print()
    
    # Create protocol instance
    protocol = RS485Protocol(port, baudrate)
    
    # Connect
    print("Connecting to RS485...")
    if not protocol.connect():
        print("✗ Failed to open port!")
        return
    
    print(f"✓ Connected to {port}")
    print()
    print("Starting periodic messages...")
    print("-" * 70)
    
    message_count = 0
    success_count = 0
    fail_count = 0
    
    try:
        while True:
            message_count += 1
            timestamp = time.strftime("%H:%M:%S")
            
            print(f"[{timestamp}] Message #{message_count:04d} ", end='', flush=True)
            
            # Send PING command to Controller OUT
            result = protocol.ping(RS485_ADDR_CONTROLLER_OUT)
            
            if result:
                success_count += 1
                print(f"✓ PING OK | Success: {success_count} | Fail: {fail_count}")
            else:
                fail_count += 1
                print(f"✗ No response | Success: {success_count} | Fail: {fail_count}")
            
            # Print stats every 10 messages
            if message_count % 10 == 0:
                success_rate = (success_count / message_count) * 100 if message_count > 0 else 0
                print(f"    Stats: {success_count}/{message_count} ({success_rate:.1f}% success)")
            
            # Wait 1 second
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\n")
        print("=" * 70)
        print("Test stopped by user")
        print("=" * 70)
        print(f"Total Messages: {message_count}")
        print(f"Successful: {success_count}")
        print(f"Failed: {fail_count}")
        if message_count > 0:
            print(f"Success Rate: {(success_count/message_count)*100:.1f}%")
        print()
    
    finally:
        print("Disconnecting...")
        protocol.disconnect()
        print("Done.")

def list_ports():
    """List available COM ports"""
    import serial.tools.list_ports
    
    ports = serial.tools.list_ports.comports()
    
    if not ports:
        print("No COM ports found!")
        return None
    
    print("Available COM Ports:")
    for i, port in enumerate(ports, 1):
        print(f"  {i}. {port.device} - {port.description}")
    print()
    
    return ports

def main():
    """Main function"""
    print()
    
    # Check command line argument
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        # List ports
        ports = list_ports()
        if not ports:
            return 1
        
        print("Enter COM port for RS485 (e.g., COM8):")
        port = input("> ").strip().upper()
        
        if not port:
            print("No port specified!")
            return 1
    
    # Send periodic messages
    send_periodic_messages(port)
    
    return 0

if __name__ == '__main__':
    sys.exit(main())



