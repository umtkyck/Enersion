"""
******************************************************************************
@file           : rs485_simple_test.py
@brief          : Ultra-simple RS485 test - raw bytes
******************************************************************************
@attention

Sends RAW test bytes over RS485 every 1 second.
This is the simplest possible test - no protocol, just raw data.
Watch COM7 (serial console) to see if MCU receives anything.

******************************************************************************
"""

import sys
import time
import serial

def send_simple_bytes(port, baudrate=115200):
    """Send simple test bytes periodically"""
    
    print("=" * 70)
    print("RS485 Simple Byte Test")
    print("=" * 70)
    print(f"Port: {port}")
    print(f"Baud Rate: {baudrate}")
    print()
    print("Sending test pattern every 1 second:")
    print("  Pattern: 0x55 0xAA 0x01 0x02 0x03 (5 bytes)")
    print()
    print("Watch COM7 (PuTTY) to see if MCU receives data!")
    print("Press Ctrl+C to stop")
    print("=" * 70)
    print()
    
    # Open serial port
    try:
        ser = serial.Serial(port, baudrate, timeout=0.1)
        print(f"✓ Opened {port} @ {baudrate} baud")
        print()
        print("Starting transmission...")
        print("-" * 70)
    except serial.SerialException as e:
        print(f"✗ Failed to open port: {e}")
        return
    
    # Test patterns
    patterns = [
        (b'\x55\xAA\x01\x02\x03', "Standard test pattern"),
        (b'TEST\r\n', "ASCII TEST message"),
        (b'\xFF\x00\xFF\x00', "Alternating pattern"),
        (b'HELLO_MCU\r\n', "Hello message"),
    ]
    
    message_count = 0
    
    try:
        while True:
            message_count += 1
            timestamp = time.strftime("%H:%M:%S")
            
            # Cycle through patterns
            pattern_idx = (message_count - 1) % len(patterns)
            data, description = patterns[pattern_idx]
            
            print(f"[{timestamp}] #{message_count:04d} Sending: {description}")
            print(f"           Bytes: {' '.join(f'0x{b:02X}' for b in data)} ({len(data)} bytes)")
            
            # Send data
            bytes_written = ser.write(data)
            print(f"           Sent: {bytes_written} bytes")
            
            # Try to read response (if any)
            time.sleep(0.1)  # Give MCU time to respond
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"           ✓ Response: {' '.join(f'0x{b:02X}' for b in response)}")
            else:
                print(f"           (no response)")
            
            print()
            
            # Wait 1 second
            time.sleep(1)
            
    except KeyboardInterrupt:
        print()
        print("=" * 70)
        print("Test stopped by user")
        print("=" * 70)
        print(f"Total messages sent: {message_count}")
        print()
    
    finally:
        ser.close()
        print("Port closed.")

def main():
    """Main function"""
    print()
    
    # Check command line argument
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        print("Enter COM port for RS485 (e.g., COM8):")
        port = input("> ").strip().upper()
        
        if not port:
            print("No port specified!")
            return 1
    
    # Send simple bytes
    send_simple_bytes(port)
    
    return 0

if __name__ == '__main__':
    sys.exit(main())



