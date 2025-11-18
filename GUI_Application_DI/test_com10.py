"""
Quick test to send PING to COM10 and see if MCU receives it
Watch COM7 for RX messages!
"""
import serial
import time
import struct

def calculate_crc16(data):
    """Calculate CRC-16-CCITT"""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc = crc << 1
            crc &= 0xFFFF
    return crc

def send_ping():
    """Send PING packet to address 0x02 via COM10"""
    try:
        ser = serial.Serial('COM10', 115200, timeout=2)
        print(f"✓ Opened COM10 (115200 baud)")
        time.sleep(0.1)
        
        # Build PING packet
        # Format: START | DEST | SRC | CMD | LEN | CRC_H | CRC_L | END
        dest = 0x02  # Controller DI
        src = 0x10   # GUI
        cmd = 0x01   # PING
        data_len = 0
        
        packet = bytearray([0xAA, dest, src, cmd, data_len])
        crc = calculate_crc16(packet[1:5])
        packet.extend([crc >> 8, crc & 0xFF, 0x55])
        
        print(f"\n📤 Sending PING packet to 0x{dest:02X}:")
        print(f"   Bytes: {' '.join([f'{b:02X}' for b in packet])}")
        print(f"   Length: {len(packet)} bytes")
        
        # Send packet
        ser.write(packet)
        ser.flush()
        print(f"✓ Packet sent!")
        
        # Wait for response
        print(f"\n⏳ Waiting for response (2 seconds)...")
        response = ser.read(100)
        
        if len(response) > 0:
            print(f"\n✅ Received {len(response)} bytes:")
            print(f"   {' '.join([f'{b:02X}' for b in response])}")
        else:
            print(f"\n❌ No response received")
            print(f"\n🔍 CHECK COM7 PuTTY:")
            print(f"   - Look for 'RX: 0xAA' messages")
            print(f"   - If you see RX messages, RS485 is working!")
            print(f"   - If no RX, check RS485 wiring")
        
        ser.close()
        
    except Exception as e:
        print(f"❌ Error: {e}")

if __name__ == "__main__":
    print("=" * 60)
    print("  COM10 → MCU PING TEST")
    print("  (Watch COM7 PuTTY for RX messages!)")
    print("=" * 60)
    send_ping()
    print("\n" + "=" * 60)
    input("\nPress Enter to exit...")

