===============================================
  Digital OUT Controller v1.1.0
  RS485 Digital Output Control Application
===============================================

QUICK START:
------------
1. Double-click "DigitalOUT_Controller.exe" to run
2. Connect your RS485 adapter (COM port)
3. Click "Connect" and select your COM port
4. Control 56 digital outputs from the GUI

REQUIREMENTS:
-------------
- Windows 10/11 (64-bit)
- RS485 USB adapter connected to PC
- Controller OUT hardware (STM32H7 with RS485)
- USB cable for serial console (optional)

HARDWARE CONNECTIONS:
---------------------
RS485 Connection:
- COM8 (or your RS485 COM port) → RS485 A/B lines
- Baud rate: 115200
- Address: 0x03 (Controller OUT)

Serial Console (Debug):
- COM7 (optional) for MCU debug messages
- Baud rate: 115200

FEATURES:
---------
✓ 56 Digital Outputs (8 groups of 7 channels)
✓ Real-time status monitoring
✓ Individual channel control (HIGH/LOW)
✓ Toggle All / Clear All functions
✓ Health check & version info
✓ COM port auto-detection
✓ Packet statistics

TROUBLESHOOTING:
----------------
Problem: "Failed to open COM port"
Solution: Close any other programs using the COM port (PuTTY, etc.)

Problem: "Controller OUT not detected"
Solution: 
- Check RS485 connections
- Verify MCU is powered and firmware is flashed
- Try reconnecting the COM port

Problem: "No response from controller"
Solution:
- Check RS485 A/B wiring polarity
- Ensure PD4 (RS485_COM_OUT) is properly configured
- Verify UART2 FIFO is enabled in firmware
- Check USART2_IRQHandler is present in firmware

Problem: "Failed to write outputs"
Solution:
- Increase timeout in rs485_protocol.py if needed
- Check output pin mappings DO0-DO55
- Verify output drivers are powered

FILE SIZE: ~38 MB (includes all Python dependencies)
STARTUP TIME: 3-5 seconds

VERSION HISTORY:
----------------
v1.1.0.2 - Critical RS485 fixes:
  • Fixed UART2 FIFO disabled bug
  • Added proper NVIC interrupt setup
  • Corrected pin mappings for DO0-DO55
  • Fixed DO49 (PG8) and DO50 (PC6) assignments
  • Removed debug messages for production

v1.0.0 - Initial release

SUPPORT:
--------
For issues or updates, check:
https://github.com/umtkyck/Enersion

===============================================
Created with PyInstaller 6.16.0 | PyQt5
© 2025 Digital OUT Controller
===============================================
