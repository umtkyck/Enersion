# RS485 Test Application

## Overview
Simple RS485 test firmware for STM32H753ZI that sends periodic "HELLO WORLD" messages every 1 second.

## Hardware Configuration

### Pins (Already Configured)
```
PD4 - RS485_COM_OUT  (Direction Enable - DE/RE control)
PD5 - RS485_TX_OUT   (UART2 TX → RS485 DI)
PD6 - RS485_RX_OUT   (UART2 RX ← RS485 RO)
```

### UART Settings
- **Peripheral:** USART2
- **Baud Rate:** 115200
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Mode:** TX + RX

## Functionality

### What It Does
1. **On Startup:**
   - Initializes RS485 in receive mode (PD4 = LOW)
   - Sends startup message: `=== RS485 Test Started ===`

2. **Main Loop:**
   - Every 1 second, sends test message:
     ```
     RS485 TEST #0001 - HELLO WORLD
     RS485 TEST #0002 - HELLO WORLD
     RS485 TEST #0003 - HELLO WORLD
     ...
     ```

3. **RS485 Transmission:**
   - Sets PD4 HIGH (enable transmitter)
   - Waits 1ms for transceiver switching
   - Transmits data via UART2
   - Waits for transmission complete
   - Sets PD4 LOW (back to receive mode)

## Code Structure

### RS485_Transmit() Function
```c
void RS485_Transmit(uint8_t *data, uint16_t len)
{
    // Enable transmitter (PD4 = HIGH)
    HAL_GPIO_WritePin(RS485_COM_OUT_GPIO_Port, RS485_COM_OUT_Pin, GPIO_PIN_SET);
    HAL_Delay(1);  // Switching delay
    
    // Transmit
    HAL_UART_Transmit(&huart2, data, len, 100);
    while(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET);
    
    // Disable transmitter (PD4 = LOW)
    HAL_Delay(1);
    HAL_GPIO_WritePin(RS485_COM_OUT_GPIO_Port, RS485_COM_OUT_Pin, GPIO_PIN_RESET);
}
```

### Send_Test_Message() Function
```c
void Send_Test_Message(void)
{
    char message[100];
    message_counter++;
    sprintf(message, "RS485 TEST #%04lu - HELLO WORLD\r\n", message_counter);
    RS485_Transmit((uint8_t*)message, strlen(message));
}
```

## Building and Flashing

### Using STM32CubeIDE
1. Open the project in STM32CubeIDE
2. Build: **Project → Build Project** (Ctrl+B)
3. Flash: **Run → Debug** (F11) or **Run → Run** (Ctrl+F11)

### Using Command Line
```bash
# Navigate to Debug folder
cd Debug

# Build using make
make clean
make -j4

# Flash using STM32_Programmer_CLI
STM32_Programmer_CLI -c port=SWD -w SW_Controller_OUT.elf -v -rst
```

## Testing

### Test Setup
```
STM32 Board          RS485 Transceiver          PC
  PD5 (TX) -------> DI                      
  PD6 (RX) <------- RO                      
  PD4 (DE) -------> DE/RE                   
                         A+ <------------> USB-RS485 (COM8)
                         B- <------------> USB-RS485 (COM8)
```

### Expected Output
Connect a serial terminal to the RS485 port (e.g., COM8 @ 115200 baud) and you should see:

```
=== RS485 Test Started ===
RS485 TEST #0001 - HELLO WORLD
RS485 TEST #0002 - HELLO WORLD
RS485 TEST #0003 - HELLO WORLD
...
```

### Using Python Test Script
From `GUI_Application` folder:
```bash
# Simple terminal monitor
python rs485_terminal.py COM8

# Or use PuTTY
# Port: COM8
# Baud: 115200
# Data bits: 8, Stop: 1, Parity: None
```

## Pin Verification

### Check with Oscilloscope/Logic Analyzer
1. **PD4 (COM_OUT):**
   - Should be LOW most of the time (RX mode)
   - Pulses HIGH for ~3-5ms every 1 second (TX mode)

2. **PD5 (TX):**
   - Should show UART data when PD4 is HIGH
   - UART frame: START + 8 data bits + STOP

3. **A+/B- Differential:**
   - Should show RS485 differential signal during TX

## Troubleshooting

### No Output on RS485
**Check:**
- [ ] PD4 configured as output (should be automatic from .ioc)
- [ ] UART2 enabled and configured correctly
- [ ] RS485 transceiver powered
- [ ] A+/B- wired correctly (not swapped)
- [ ] Baud rate matches (115200)

### Messages Garbled
**Check:**
- [ ] Baud rate mismatch
- [ ] RS485 termination (120Ω resistors at both ends)
- [ ] Cable quality and length
- [ ] Ground connection

### PD4 Not Toggling
**Check:**
- [ ] GPIO clock enabled (GPIOD)
- [ ] Pin not used by another peripheral
- [ ] Code compiled and flashed correctly

## Modifications

### Change Message Interval
In `main.c`, line ~165:
```c
if ((HAL_GetTick() - last_send_time) >= 1000)  // 1000ms = 1 second
```
Change 1000 to desired interval in milliseconds.

### Change Message Content
In `main.c`, function `Send_Test_Message()`:
```c
sprintf(message, "YOUR MESSAGE HERE #%04lu\r\n", message_counter);
```

### Change Baud Rate
In STM32CubeMX (.ioc file):
1. Open in CubeMX
2. Connectivity → USART2
3. Change "Baud Rate" parameter
4. Re-generate code

## Notes

- All code is in `USER CODE BEGIN/END` sections
- Safe to regenerate with STM32CubeMX
- Uses HAL library
- No interrupts or DMA (simple polling)
- Minimal CPU load (sleeps in loop)

## Version
- **Date:** 2025-01-14
- **Author:** Enersion
- **MCU:** STM32H753ZITx
- **Purpose:** RS485 Hardware Verification Test

---

**Quick Start:**
1. Build and flash firmware
2. Connect RS485 to COM8
3. Open PuTTY @ COM8, 115200 baud
4. Watch "HELLO WORLD" messages appear every 1 second! 🚀


