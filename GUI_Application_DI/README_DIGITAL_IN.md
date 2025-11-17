# Digital IN Controller - GUI Application

RS485-based Digital Input Monitoring Application for STM32H7 Controller DIO.

## Overview

This application monitors 56 digital inputs via RS485 communication protocol with real-time status display and auto-refresh capabilities.

## Features

- **56 Digital Input Channels**: Monitor DI0 to DI55
- **Real-time Monitoring**: Auto-refresh mode (1 second interval)
- **Visual Indicators**: Color-coded HIGH/LOW states
- **Health Monitoring**: Continuous MCU health check
- **RS485 Communication**: Robust protocol with CRC checking
- **Version Display**: Shows firmware version information

## Hardware Requirements

- Windows PC with Python 3.8+
- RS485 USB Adapter
- STM32H7 Controller DIO with firmware flashed
- USB cable for debug console (optional)

## Hardware Connections

### RS485 Connection (Data)
- **COM Port**: COM8 (or your RS485 adapter port)
- **Baud Rate**: 115200
- **MCU Address**: 0x02 (Controller DIO)
- **Pins**: PD4 (COM), PD5 (TX), PD6 (RX)

### Debug Console (Optional)
- **COM Port**: COM7 (or your debug UART port)
- **Baud Rate**: 115200
- **Purpose**: View MCU debug messages

## Installation

### 1. Install Python Dependencies

```bash
cd GUI_Application_DI
pip install -r requirements.txt
```

### 2. Flash Firmware

Flash the `SW_Controller_DI` firmware to your STM32H7 microcontroller using STM32CubeIDE.

**Important Firmware Features**:
- UART2 FIFO enabled (critical for reliable communication)
- RS485 half-duplex control on PD4
- USART2 interrupt enabled
- Address set to 0x02 (RS485_ADDR_CONTROLLER_DIO)

### 3. Connect Hardware

1. Connect RS485 adapter to PC (usually appears as COM8)
2. Connect RS485 A/B lines to MCU
3. Power on the MCU
4. (Optional) Connect debug UART to view MCU messages

## Usage

### GUI Application

```bash
cd GUI_Application_DI
python main_gui.py
```

**Or use the batch file**:
```bash
run.bat
```

### GUI Operation

1. **Connect**:
   - Select COM port from dropdown
   - Click "Connect"
   - Wait for "Controller DIO detected" message

2. **Monitor Inputs**:
   - Click "Read Inputs Now" for single read
   - Click "Start Auto-Refresh (1s)" for continuous monitoring
   - Active inputs show as GREEN "HIGH"
   - Inactive inputs show as GRAY "LOW"

3. **Monitor Health**:
   - MCU status panel shows connection state
   - Health percentage (should be 100%)
   - Packet statistics (TX/RX/Errors)

### Test Script

```bash
cd GUI_Application_DI
python test_controller_di.py
```

Enter COM port when prompted (e.g., COM8).

**Tests performed**:
- PING (connectivity check)
- GET_VERSION (firmware version)
- HEARTBEAT (health status)
- GET_STATUS (detailed statistics)
- READ_DI (digital input values)

## RS485 Protocol

### Commands

| Command | ID | Description |
|---------|----|----|
| PING | 0x01 | Test connectivity |
| GET_VERSION | 0x03 | Get firmware version |
| HEARTBEAT | 0x04 | Get health status |
| GET_STATUS | 0x05 | Get detailed stats |
| READ_DI | 0x0B | Read digital inputs |

### Packet Format

```
[START] [DEST] [SRC] [CMD] [LEN] [DATA...] [CRC_L] [CRC_H] [END]
  0xAA   1byte  1byte  1byte  1byte   0-250   2bytes      0x55
```

### Digital Input Data Format

Response to READ_DI command contains 7 bytes:
- **Byte 0**: DI0-DI7 (bit 0 = DI0, bit 7 = DI7)
- **Byte 1**: DI8-DI15
- **Byte 2**: DI16-DI23
- **Byte 3**: DI24-DI31
- **Byte 4**: DI32-DI39
- **Byte 5**: DI40-DI47
- **Byte 6**: DI48-DI55

Each bit: 1 = HIGH, 0 = LOW

## Troubleshooting

### "Controller DIO not detected"

**Causes**:
- RS485 wiring incorrect
- MCU not powered
- Firmware not flashed
- Wrong COM port selected

**Solutions**:
1. Verify RS485 connections (A/B polarity)
2. Check MCU power LED
3. Re-flash firmware from SW_Controller_DI
4. Verify COM port in Device Manager

### "Failed to open COM port"

**Cause**: Another program is using the COM port

**Solution**:
1. Close PuTTY, Tera Term, or other serial programs
2. Check for background Python processes
3. Use Task Manager to end stuck python.exe processes

### "No response from controller"

**Causes**:
- UART FIFO disabled (CRITICAL)
- USART2 interrupt not enabled
- RS485 direction pin (PD4) not configured
- Wrong MCU address

**Solutions**:
1. Ensure `HAL_UARTEx_EnableFifoMode(&huart2)` is called
2. Verify USART2_IRQHandler exists in stm32h7xx_it.c
3. Check PD4 GPIO is OUTPUT mode
4. Confirm address is 0x02 in rs485_protocol.c

### Inputs Not Updating

**Causes**:
- Input signals not connected
- GPIO pins not configured as inputs
- Input voltage levels incorrect

**Solutions**:
1. Verify GPIO pins are configured as INPUT
2. Check input signals with multimeter
3. Ensure input voltage is 0V (LOW) or 3.3V (HIGH)
4. Review digital_input_handler.c implementation

## File Structure

```
GUI_Application_DI/
├── main_gui.py                    # Main GUI application
├── rs485_protocol.py              # RS485 communication protocol
├── version.py                     # Version information
├── requirements.txt               # Python dependencies
├── run.bat                        # Windows launcher
├── test_controller_di.py          # Test script
└── README_DIGITAL_IN.md           # This file
```

## Firmware Files

```
SW_Controller_DI/
├── Core/
│   ├── Src/
│   │   ├── main.c                  # Main application (UART FIFO enabled)
│   │   ├── rs485_protocol.c        # RS485 protocol (address 0x02)
│   │   ├── digital_input_handler.c # Digital input handling
│   │   ├── stm32h7xx_it.c          # Interrupt handlers (USART2_IRQHandler)
│   │   └── stm32h7xx_hal_msp.c     # HAL MSP (NVIC enable)
│   └── Inc/
│       ├── main.h                  # Pin definitions
│       ├── rs485_protocol.h        # Protocol definitions
│       └── digital_input_handler.h # Input handler header
```

## Version History

### v1.0 (2025-11-17)
- Initial release
- 56 digital input channels
- RS485 protocol with UART FIFO
- Real-time monitoring
- Auto-refresh mode
- Health monitoring

## Support

For issues or questions:
- Check firmware is compiled and flashed
- Verify all hardware connections
- Review debug console (COM7) for MCU messages
- Test with test_controller_di.py script

## License

Internal project - All rights reserved

---

**Digital IN Controller v1.0** | RS485 Digital Input Monitoring

