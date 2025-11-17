"""Listen to COM8 continuously"""
import serial
import time

print("=" * 70)
print("COM8 Listener - Waiting for ANY data...")
print("=" * 70)
print()

try:
    ser = serial.Serial('COM8', 115200, timeout=0.1)
    print("COM8 opened, listening...\n")
    
    byte_count = 0
    last_data_time = time.time()
    
    while True:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            byte_count += len(data)
            last_data_time = time.time()
            
            hex_str = ' '.join(f'{b:02X}' for b in data)
            print(f"[{time.time():.3f}] RX ({len(data)} bytes): {hex_str}")
        
        # Print status every 5 seconds
        if time.time() - last_data_time > 5 and byte_count > 0:
            print(f"[IDLE] Total received: {byte_count} bytes")
            last_data_time = time.time()
        
        time.sleep(0.01)
        
except KeyboardInterrupt:
    print("\n\nStopped by user")
    print(f"Total bytes received: {byte_count}")
    ser.close()
except Exception as e:
    print(f"[ERROR] {e}")

print("=" * 70)


