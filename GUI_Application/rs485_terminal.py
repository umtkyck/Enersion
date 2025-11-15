"""
******************************************************************************
@file           : rs485_terminal.py
@brief          : Simple RS485 Terminal - Send and Receive
******************************************************************************
@attention

Interactive terminal for COM8 (RS485).
Shows everything sent and received on COM8.

******************************************************************************
"""

import sys
import time
import serial
import threading

class RS485Terminal:
    """Simple RS485 Terminal"""
    
    def __init__(self, port, baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = False
        self.rx_thread = None
        self.message_count = 0
    
    def connect(self):
        """Open serial port"""
        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=0.1
            )
            print(f"✓ Connected to {self.port} @ {self.baudrate} baud")
            return True
        except serial.SerialException as e:
            print(f"✗ Failed to open {self.port}: {e}")
            return False
    
    def disconnect(self):
        """Close serial port"""
        self.running = False
        if self.rx_thread:
            self.rx_thread.join(timeout=1)
        if self.ser and self.ser.is_open:
            self.ser.close()
    
    def receive_thread(self):
        """Background thread to receive data"""
        while self.running:
            try:
                if self.ser and self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting)
                    timestamp = time.strftime("%H:%M:%S")
                    
                    # Display as hex
                    hex_str = ' '.join(f'{b:02X}' for b in data)
                    
                    # Try to display as ASCII if printable
                    try:
                        ascii_str = data.decode('ascii', errors='replace')
                        ascii_str = ''.join(c if c.isprintable() or c in '\r\n\t' else '.' for c in ascii_str)
                    except:
                        ascii_str = ''
                    
                    print(f"\n[{timestamp}] RX << {len(data)} bytes")
                    print(f"  HEX: {hex_str}")
                    if ascii_str.strip():
                        print(f"  ASCII: {ascii_str}")
                    print("> ", end='', flush=True)
                
                time.sleep(0.01)
            except Exception as e:
                print(f"\nRX Error: {e}")
                break
    
    def send_data(self, data_str):
        """Send data to serial port"""
        if not self.ser or not self.ser.is_open:
            print("Port not open!")
            return
        
        # Convert string to bytes
        # Support both hex (0x55 0xAA) and ASCII
        if data_str.startswith('0x') or ' 0x' in data_str:
            # Hex format
            try:
                hex_values = data_str.replace('0x', '').split()
                data = bytes([int(h, 16) for h in hex_values])
            except ValueError as e:
                print(f"Invalid hex format: {e}")
                return
        else:
            # ASCII format
            # Support escape sequences
            data_str = data_str.replace('\\r', '\r').replace('\\n', '\n').replace('\\t', '\t')
            data = data_str.encode('ascii', errors='replace')
        
        # Send
        try:
            self.message_count += 1
            bytes_sent = self.ser.write(data)
            timestamp = time.strftime("%H:%M:%S")
            
            hex_str = ' '.join(f'{b:02X}' for b in data)
            
            print(f"[{timestamp}] TX >> {bytes_sent} bytes sent")
            print(f"  HEX: {hex_str}")
            
        except Exception as e:
            print(f"Send error: {e}")
    
    def run_interactive(self):
        """Run interactive terminal"""
        if not self.connect():
            return
        
        # Start receive thread
        self.running = True
        self.rx_thread = threading.Thread(target=self.receive_thread, daemon=True)
        self.rx_thread.start()
        
        print()
        print("=" * 70)
        print("RS485 Interactive Terminal")
        print("=" * 70)
        print("Commands:")
        print("  - Type text and press Enter to send as ASCII")
        print("  - Type hex: 0x55 0xAA 0x01  (send as hex bytes)")
        print("  - Type 'test' to send test message")
        print("  - Type 'hello' to send HELLO WORLD")
        print("  - Type 'exit' or Ctrl+C to quit")
        print("=" * 70)
        print()
        
        try:
            while True:
                user_input = input("> ").strip()
                
                if not user_input:
                    continue
                
                if user_input.lower() == 'exit':
                    break
                elif user_input.lower() == 'test':
                    self.send_data("TEST MESSAGE\\r\\n")
                elif user_input.lower() == 'hello':
                    self.send_data("HELLO WORLD FROM PC\\r\\n")
                else:
                    self.send_data(user_input)
        
        except KeyboardInterrupt:
            print("\n\nExiting...")
        
        finally:
            self.disconnect()
            print("\nDisconnected.")
    
    def run_auto_test(self):
        """Run automatic test with periodic messages"""
        if not self.connect():
            return
        
        # Start receive thread
        self.running = True
        self.rx_thread = threading.Thread(target=self.receive_thread, daemon=True)
        self.rx_thread.start()
        
        print()
        print("=" * 70)
        print("RS485 Automatic Test Mode")
        print("=" * 70)
        print("Sending test messages every 1 second...")
        print("Press Ctrl+C to stop")
        print("=" * 70)
        print()
        
        test_messages = [
            "HELLO WORLD\\r\\n",
            "TEST MESSAGE\\r\\n",
            "0x55 0xAA 0x01 0x02 0x03",
            "RS485 CHECK\\r\\n",
            "PING\\r\\n"
        ]
        
        try:
            count = 0
            while True:
                msg = test_messages[count % len(test_messages)]
                print(f"\n>>> Sending message #{count+1}: ", end='')
                self.send_data(msg)
                count += 1
                time.sleep(1)
        
        except KeyboardInterrupt:
            print("\n\nStopping...")
        
        finally:
            self.disconnect()
            print(f"\nTotal messages sent: {self.message_count}")
            print("Disconnected.")

def main():
    """Main function"""
    print()
    
    if len(sys.argv) > 1:
        port = sys.argv[1]
        mode = sys.argv[2] if len(sys.argv) > 2 else 'interactive'
    else:
        print("Usage:")
        print("  python rs485_terminal.py COM8")
        print("  python rs485_terminal.py COM8 auto")
        print()
        port = input("Enter COM port (e.g., COM8): ").strip().upper()
        if not port:
            print("No port specified!")
            return 1
        
        print("\nSelect mode:")
        print("  1. Interactive (type messages)")
        print("  2. Automatic (send test messages every 1 sec)")
        choice = input("Choice [1]: ").strip()
        mode = 'auto' if choice == '2' else 'interactive'
    
    terminal = RS485Terminal(port)
    
    if mode == 'auto':
        terminal.run_auto_test()
    else:
        terminal.run_interactive()
    
    return 0

if __name__ == '__main__':
    sys.exit(main())

