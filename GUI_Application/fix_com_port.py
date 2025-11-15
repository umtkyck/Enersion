"""
******************************************************************************
@file           : fix_com_port.py
@brief          : COM Port Troubleshooting Tool
******************************************************************************
@attention

This tool helps diagnose and fix COM port access issues.

******************************************************************************
"""

import sys
import serial
import serial.tools.list_ports

def list_all_ports():
    """List all available COM ports with details"""
    print("=" * 70)
    print("Available COM Ports")
    print("=" * 70)
    
    ports = serial.tools.list_ports.comports()
    
    if not ports:
        print("No COM ports found!")
        return []
    
    for i, port in enumerate(ports, 1):
        print(f"\n[{i}] {port.device}")
        print(f"    Description: {port.description}")
        print(f"    Hardware ID: {port.hwid}")
        print(f"    Manufacturer: {port.manufacturer if port.manufacturer else 'Unknown'}")
        
        # Try to open the port to check if it's available
        try:
            ser = serial.Serial(port.device, timeout=0.1)
            ser.close()
            print(f"    Status: ✓ AVAILABLE")
        except serial.SerialException as e:
            if "PermissionError" in str(e) or "Access is denied" in str(e):
                print(f"    Status: ✗ IN USE (locked by another application)")
            else:
                print(f"    Status: ✗ ERROR - {e}")
    
    print("\n" + "=" * 70)
    return ports

def find_locked_processes():
    """Try to identify processes using COM ports (Windows)"""
    print("\n" + "=" * 70)
    print("Checking for processes using COM ports...")
    print("=" * 70)
    
    try:
        import subprocess
        
        # Run PowerShell command to find serial port handles
        cmd = 'Get-CimInstance Win32_PnPEntity | Where-Object {$_.Name -match "COM"} | Select-Object Name, Status'
        result = subprocess.run(['powershell', '-Command', cmd], 
                              capture_output=True, text=True, timeout=5)
        
        if result.stdout:
            print(result.stdout)
        else:
            print("Could not retrieve COM port status from Windows.")
            
    except Exception as e:
        print(f"Could not check processes: {e}")
        print("\nManual check suggestions:")
        print("1. Open Task Manager (Ctrl+Shift+Esc)")
        print("2. Look for applications that might use serial ports:")
        print("   - Arduino IDE")
        print("   - PuTTY / Terminal programs")
        print("   - Other Python scripts")
        print("   - STM32CubeProgrammer")
        print("   - Serial Monitor tools")

def test_port(port_name):
    """Test if a specific port can be opened"""
    print(f"\n" + "=" * 70)
    print(f"Testing {port_name}...")
    print("=" * 70)
    
    try:
        print(f"Attempting to open {port_name}...")
        ser = serial.Serial(port_name, baudrate=115200, timeout=1)
        print(f"✓ SUCCESS: {port_name} opened successfully!")
        ser.close()
        print(f"✓ Port closed properly.")
        return True
    except serial.SerialException as e:
        print(f"✗ FAILED: {e}")
        
        if "PermissionError" in str(e) or "Access is denied" in str(e):
            print("\n🔒 Port is locked by another application!")
            print("\nSolutions:")
            print("1. Close any serial monitor/terminal programs")
            print("2. Close Arduino IDE or other development tools")
            print("3. Unplug and replug the USB device")
            print("4. Restart your computer (if nothing else works)")
            print("\nCommon culprits:")
            print("  - Previous Python script still running")
            print("  - Serial terminal (PuTTY, TeraTerm, etc.)")
            print("  - Device Manager has port open")
        
        return False

def interactive_test():
    """Interactive port testing"""
    print("\n" + "=" * 70)
    print("COM Port Troubleshooting Tool")
    print("=" * 70)
    
    # List all ports
    ports = list_all_ports()
    
    if not ports:
        print("\nNo ports found. Check USB connections.")
        return
    
    # Check for locked processes
    find_locked_processes()
    
    # Ask user which port to test
    print("\n" + "=" * 70)
    print("Port Testing")
    print("=" * 70)
    
    port_to_test = input("\nEnter COM port to test (e.g., COM7) or press Enter to skip: ").strip().upper()
    
    if port_to_test:
        test_port(port_to_test)
    
    # Recommendations
    print("\n" + "=" * 70)
    print("Recommendations")
    print("=" * 70)
    print("\nIf COM7 is locked:")
    print("1. Run this command in PowerShell (as Administrator):")
    print("   Get-Process | Where-Object {$_.MainWindowTitle -match 'COM'}")
    print("\n2. Or try closing these common programs:")
    print("   - Any serial terminal (PuTTY, TeraTerm, RealTerm)")
    print("   - Arduino IDE / Serial Monitor")
    print("   - Other Python scripts accessing serial ports")
    print("   - STM32CubeProgrammer or STM32CubeIDE")
    print("\n3. Try different baud rates if available")
    print("\n4. Use a different COM port if possible")
    print("\n5. Restart the USB device:")
    print("   - Unplug USB cable")
    print("   - Wait 5 seconds")
    print("   - Plug back in")

def main():
    """Main function"""
    try:
        interactive_test()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user.")
    except Exception as e:
        print(f"\nError: {e}")

if __name__ == '__main__':
    main()


