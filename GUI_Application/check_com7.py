"""Check COM7 - Debug UART"""
import serial
import time

print("=" * 70)
print("COM7 Debug Monitor")
print("=" * 70)
print("Reading COM7 for 10 seconds...")
print("MCU should show boot messages and RS485 initialization")
print("-" * 70)

try:
    ser = serial.Serial('COM7', 115200, timeout=0.1)
    print("[COM7 OPENED]\n")
    
    start_time = time.time()
    data_received = False
    
    while (time.time() - start_time) < 10:
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            try:
                text = data.decode('ascii', errors='replace')
                print(text, end='', flush=True)
                data_received = True
            except:
                print(f"[HEX: {data.hex()}]", flush=True)
        time.sleep(0.1)
    
    ser.close()
    
    print("\n" + "-" * 70)
    if not data_received:
        print("[WARNING] NO DATA from COM7!")
        print("Possible issues:")
        print("  - MCU not running")
        print("  - Debug UART not configured")
        print("  - Wrong COM port")
    else:
        print("[OK] Data received from COM7")
    
except Exception as e:
    print(f"[ERROR] {e}")

print("=" * 70)

