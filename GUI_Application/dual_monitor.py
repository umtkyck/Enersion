"""
Dual COM Monitor - COM7 (Debug) and COM8 (RS485) together
"""
import serial
import threading
import time

class DualMonitor:
    def __init__(self):
        self.running = True
        self.com7 = None
        self.com8 = None
        
    def monitor_com7(self):
        """Monitor COM7 - Debug UART"""
        try:
            self.com7 = serial.Serial('COM7', 115200, timeout=0.1)
            print("[COM7] Opened - Debug UART")
            
            while self.running:
                if self.com7.in_waiting > 0:
                    data = self.com7.read(self.com7.in_waiting)
                    try:
                        text = data.decode('ascii', errors='replace')
                        print(f"[COM7] {text}", end='')
                    except:
                        print(f"[COM7] HEX: {data.hex()}")
                time.sleep(0.01)
        except Exception as e:
            print(f"[COM7] Error: {e}")
        finally:
            if self.com7:
                self.com7.close()
    
    def monitor_com8(self):
        """Monitor COM8 - RS485"""
        try:
            self.com8 = serial.Serial('COM8', 115200, timeout=0.1)
            print("[COM8] Opened - RS485")
            
            while self.running:
                if self.com8.in_waiting > 0:
                    data = self.com8.read(self.com8.in_waiting)
                    timestamp = time.strftime("%H:%M:%S")
                    print(f"\n[COM8 RX {timestamp}] {len(data)} bytes: {' '.join(f'{b:02X}' for b in data)}")
                time.sleep(0.01)
        except Exception as e:
            print(f"[COM8] Error: {e}")
        finally:
            if self.com8:
                self.com8.close()
    
    def send_test_ping(self):
        """Send test PING every 2 seconds"""
        time.sleep(2)  # Wait for COM ports to open
        
        # PING packet
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
        
        dest = 0x03
        src = 0x10
        cmd = 0x01
        length = 0x00
        
        crc_data = bytes([dest, src, cmd, length])
        crc = calc_crc16(crc_data)
        packet = bytes([0xAA]) + crc_data + bytes([crc & 0xFF, (crc >> 8) & 0xFF, 0x55])
        
        counter = 0
        while self.running:
            try:
                if self.com8 and self.com8.is_open:
                    counter += 1
                    timestamp = time.strftime("%H:%M:%S")
                    print(f"\n[COM8 TX {timestamp}] PING #{counter}: {' '.join(f'{b:02X}' for b in packet)}")
                    self.com8.write(packet)
                time.sleep(2)
            except Exception as e:
                print(f"[COM8 TX] Error: {e}")
                break
    
    def run(self):
        print("=" * 70)
        print("Dual COM Monitor - COM7 (Debug) + COM8 (RS485)")
        print("=" * 70)
        print("Starting monitors...")
        print()
        
        # Start threads
        t7 = threading.Thread(target=self.monitor_com7, daemon=True)
        t8 = threading.Thread(target=self.monitor_com8, daemon=True)
        tx = threading.Thread(target=self.send_test_ping, daemon=True)
        
        t7.start()
        t8.start()
        tx.start()
        
        try:
            print("Press Ctrl+C to stop...\n")
            while True:
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\n\nStopping...")
            self.running = False
            time.sleep(1)
            print("Done.")

if __name__ == '__main__':
    monitor = DualMonitor()
    monitor.run()


