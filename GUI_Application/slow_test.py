"""Slow RS485 Test - Send bytes one by one"""
import serial
import time

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

print("=" * 70)
print("Slow RS485 Test - Byte by byte")
print("=" * 70)
print(f"Packet: {' '.join(f'{b:02X}' for b in packet)}")
print()

try:
    ser = serial.Serial('COM8', 115200, timeout=2)
    print("COM8 opened\n")
    
    # Send packet byte by byte with delay
    print("Sending PING byte by byte (10ms delay)...")
    for i, byte in enumerate(packet):
        print(f"  Byte {i+1}/8: 0x{byte:02X}")
        ser.write(bytes([byte]))
        time.sleep(0.01)  # 10ms delay between bytes
    
    print("\nWaiting for response...")
    time.sleep(1)
    
    if ser.in_waiting > 0:
        response = ser.read(ser.in_waiting)
        print(f"\n[SUCCESS] Response: {' '.join(f'{b:02X}' for b in response)}")
    else:
        print("\n[FAIL] No response")
    
    ser.close()
    
except Exception as e:
    print(f"[ERROR] {e}")

print("=" * 70)

