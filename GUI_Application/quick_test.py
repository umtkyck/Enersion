"""Quick RS485 Test with Output"""
import serial
import time
import sys

print("=" * 70)
print("RS485 Quick Test - COM8")
print("=" * 70)

# CRC calculation
def calc_crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

# Build PING packet
dest = 0x03
src = 0x10
cmd = 0x01
length = 0x00

crc_data = bytes([dest, src, cmd, length])
crc = calc_crc16(crc_data)
packet = bytes([0xAA]) + crc_data + bytes([crc & 0xFF, (crc >> 8) & 0xFF, 0x55])

print(f"\nPING Packet: {' '.join(f'{b:02X}' for b in packet)}")
print(f"Length: {len(packet)} bytes\n")

try:
    # Open COM8
    print("Opening COM8...")
    ser = serial.Serial('COM8', 115200, timeout=2)
    print("COM8 opened OK\n")
    
    # Send 5 PINGs
    for i in range(5):
        print(f"[{i+1}] Sending PING...", end='', flush=True)
        ser.write(packet)
        ser.flush()  # CRITICAL: Force OS to send all bytes NOW!
        
        # Wait for response
        time.sleep(1.0)  # Increased to 1 second for MCU processing
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f" RESPONSE! {len(response)} bytes")
            print(f"    HEX: {' '.join(f'{b:02X}' for b in response)}")
            
            # Check if valid response
            if len(response) >= 8 and response[0] == 0xAA and response[-1] == 0x55:
                if response[3] == 0x02:  # PING_RESPONSE
                    print("    [SUCCESS] Valid PING response!")
                else:
                    print(f"    [?] Command: 0x{response[3]:02X}")
        else:
            print(" NO RESPONSE (timeout)")
        
        print()
        time.sleep(1)
    
    ser.close()
    print("\nCOM8 closed")
    
except Exception as e:
    print(f"\n[ERROR] {e}")
    sys.exit(1)

print("\n" + "=" * 70)
print("Test Complete")
print("=" * 70)

