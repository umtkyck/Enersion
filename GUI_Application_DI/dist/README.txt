===============================================
  Digital IN Controller v1.1.0
  RS485 Digital Input Monitor Application
===============================================

QUICK START:
------------
1. Double-click "DigitalIN_Controller.exe" to run
2. Connect your RS485 adapter (COM port)
3. Click "Connect" and select your COM port
4. Monitor 56 digital inputs in real-time

REQUIREMENTS:
-------------
- Windows 10/11 (64-bit)
- RS485 USB adapter connected to PC
- Controller DI hardware (STM32H7 with RS485)
- USB cable for serial console (optional)

HARDWARE CONNECTIONS:
---------------------
RS485 Connection:
- COM8 (or your RS485 COM port) → RS485 A/B lines
- Baud rate: 115200
- Address: 0x02 (Controller DI)

Serial Console (Debug):
- COM7 (optional) for MCU debug messages
- Baud rate: 115200

FEATURES:
---------
✓ 56 Digital Inputs (8 groups of 7 channels)
✓ Real-time input state monitoring (HIGH/LOW)
✓ Auto-refresh every 100ms
✓ Health check & version info
✓ COM port auto-detection
✓ Packet statistics
✓ Input debouncing in firmware

TROUBLESHOOTING:
----------------
Problem: "Failed to open COM port"
Solution: Close any other programs using the COM port (PuTTY, etc.)

Problem: "Controller DI not detected"
Solution: 
- Check RS485 connections
- Verify MCU is powered and firmware is flashed
- Try reconnecting the COM port

Problem: "No response from controller"
Solution:
- Check RS485 A/B wiring polarity
- Ensure PD4 (RS485_DI_COM) is properly configured
- Verify UART2 FIFO is enabled in firmware
- Check USART2_IRQHandler is present in firmware

Problem: "Input readings not updating"
Solution:
- Verify input pins DI0-DI55 are correctly mapped
- Check pull-up/pull-down resistors on input pins
- Ensure input voltage levels are correct (0V/3.3V)

FILE SIZE: ~38 MB (includes all Python dependencies)
STARTUP TIME: 3-5 seconds

VERSION HISTORY:
----------------
v1.1.0.2 - Critical RS485 fixes:
  • Fixed UART2 FIFO disabled bug
  • Added missing USART2_IRQHandler
  • Updated pin mappings for DI0-DI55
  • Removed debug messages for production

v1.0.0 - Initial release

SUPPORT:
--------
For issues or updates, check:
https://github.com/umtkyck/Enersion

===============================================
Created with PyInstaller 6.16.0 | PyQt5
© 2025 Digital IN Controller
===============================================
