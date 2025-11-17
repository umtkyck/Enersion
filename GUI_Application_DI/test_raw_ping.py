"""Quick test - Send RAW PING to COM8"""
import serial
import time

# PING packet structure:
# 0xAA (start)
# 0x03 (dest=Controller OUT)  
# 0x10 (src=GUI)
# 0x01 (CMD_PING)
# 0x00 (length=0)
# CRC16 (calculated)
# 0x55 (end)

def calc_crc16(data):
    """Calculate CRC16 (Modbus style)"""
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
dest = 0x03  # Controller OUT
src = 0x10   # GUI
cmd = 0x01   # PING
length = 0x00

# CRC buffer
crc_data = bytes([dest, src, cmd, length])
crc = calc_crc16(crc_data)

# Full packet
packet = bytes([0xAA]) + crc_data + bytes([crc & 0xFF, (crc >> 8) & 0xFF, 0x55])

print("=" * 60)
print("RAW PING Test to Controller OUT")
print("=" * 60)
print(f"Packet: {' '.join(f'{b:02X}' for b in packet)}")
print(f"Length: {len(packet)} bytes")
print()

try:
    ser = serial.Serial('COM8', 115200, timeout=1)
    print("[OK] COM8 opened")
    
    print("\nSending PING...")
    ser.write(packet)
    
    time.sleep(0.5)
    
    if ser.in_waiting > 0:
        response = ser.read(ser.in_waiting)
        print(f"\n[OK] RESPONSE RECEIVED ({len(response)} bytes):")
        print(f"  HEX: {' '.join(f'{b:02X}' for b in response)}")
        
        if len(response) >= 8:
            if response[0] == 0xAA and response[-1] == 0x55:
                print("  [OK] Valid packet structure")
                if response[3] == 0x02:  # PING_RESPONSE
                    print("  [OK] PING RESPONSE - Controller OUT is alive!")
                else:
                    print(f"  [?] Command: 0x{response[3]:02X}")
            else:
                print("  [FAIL] Invalid packet format")
    else:
        print("\n[FAIL] NO RESPONSE - Timeout")
        print("\nPossible causes:")
        print("  1. Code not flashed to MCU")
        print("  2. MCU not running (check COM7 for debug)")
        print("  3. RS485 hardware issue")
        print("  4. Interrupt not enabled (rebuild needed)")
    
    ser.close()
    print("\n[OK] COM8 closed")
    
except Exception as e:
    print(f"\n[ERROR] {e}")

print("=" * 60)

