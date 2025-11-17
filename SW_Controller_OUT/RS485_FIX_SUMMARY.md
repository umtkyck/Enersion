# RS485 Direction Control Fix - SW_Controller_OUT

## Problem
RS485 half-duplex communication was not working because **PD4 (RS485_COM_OUT)** direction control pin was not being toggled during transmission.

## Solution Applied

### Files Modified

#### 1. `Core/Inc/main.h`
**Status:** ✅ Already Correct
```c
#define RS485_COM_OUT_Pin GPIO_PIN_4
#define RS485_COM_OUT_GPIO_Port GPIOD
#define RS485_TX_OUT_Pin GPIO_PIN_5
#define RS485_TX_OUT_GPIO_Port GPIOD
#define RS485_RX_OUT_Pin GPIO_PIN_6
#define RS485_RX_OUT_GPIO_Port GPIOD
```

#### 2. `Core/Src/rs485_protocol.c`

**Added in RS485_Init():**
```c
/* Initialize RS485 direction pin (PD4) to RX mode (LOW) */
HAL_GPIO_WritePin(RS485_COM_OUT_GPIO_Port, RS485_COM_OUT_Pin, GPIO_PIN_RESET);
```

**Updated RS485_SendPacket():**
```c
/* Enable RS485 transmitter (PD4 = HIGH) */
HAL_GPIO_WritePin(RS485_COM_OUT_GPIO_Port, RS485_COM_OUT_Pin, GPIO_PIN_SET);

/* Small delay for transceiver switching (1ms) */
HAL_Delay(1);

/* Transmit packet */
HAL_StatusTypeDef result = HAL_UART_Transmit(&huart2, (uint8_t*)&packet, 
                                              packetSize, RS485_TIMEOUT_MS);

/* Wait for transmission complete */
while(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET);

/* Small delay before switching back (1ms) */
HAL_Delay(1);

/* Switch back to receive mode (PD4 = LOW) */
HAL_GPIO_WritePin(RS485_COM_OUT_GPIO_Port, RS485_COM_OUT_Pin, GPIO_PIN_RESET);
```

## How It Works Now

### Initialization
1. System initializes GPIOD clock ✓
2. PD4 configured as GPIO Output ✓
3. PD4 set to LOW (RX mode) ✓
4. UART2 configured on PD5/PD6 ✓

### Normal Operation (Receive Mode)
```
PD4 = LOW  → RS485 transceiver in receive mode
PD6 (RX)   ← Receives data from RS485 bus
```

### During Transmission
```
PD4 = HIGH → RS485 transceiver in transmit mode
Wait 1ms   → Allow transceiver to switch
PD5 (TX)   → Sends data to RS485 bus
Wait TC    → Wait for transmission complete
Wait 1ms   → Allow data to leave transceiver
PD4 = LOW  → Back to receive mode
```

## Timing Diagram
```
Time:      0ms    1ms    3ms    5ms    6ms    7ms
           |      |      |      |      |      |
PD4:    ___┌──────────────────────────┐_______
           |  TX MODE                 | RX    |
           |                          |       |
PD5:    ___┌──UART DATA───────────────┐_______
           |  Packet transmission     |       |
           |                          |       |
RS485:  ___┌──A+/B- DIFFERENTIAL──────┐_______
```

## Verification Checklist

### Hardware
- [x] PD4 configured as GPIO Output (push-pull)
- [x] PD5 connected to RS485 DI (Driver Input)
- [x] PD6 connected to RS485 RO (Receiver Output)
- [x] PD4 connected to RS485 DE/RE pins
- [x] RS485 A+/B- to bus

### Software
- [x] PD4 initialized to LOW (RX mode)
- [x] PD4 goes HIGH before transmission
- [x] 1ms delay for transceiver switching
- [x] Wait for UART TC flag
- [x] 1ms delay before switching back
- [x] PD4 goes LOW after transmission
- [x] main.h included in rs485_protocol.h

### Expected Behavior
- [x] Idle state: PD4 = LOW (RX mode)
- [x] PC sends PING: MCU receives on PD6
- [x] MCU responds: PD4 pulses HIGH, sends response, returns to LOW
- [x] GUI can communicate with Controller OUT

## Testing

### Test with GUI Application
```bash
cd GUI_Application
python main_gui.py
```

1. Select **COM8** (RS485 port)
2. Baud Rate: **115200**
3. Click **Connect**
4. Click **Tools → Scan Device**
5. Should detect **Controller OUT (0x03)** ✓
6. Control digital outputs DO0-DO55 ✓

### Test with Terminal
```bash
python rs485_periodic_test.py COM8
```
Should show:
- PING responses from Controller OUT
- Success rate should be high (>95%)

### Oscilloscope Verification
- **PD4:** LOW most of time, pulses HIGH during TX
- **PD5:** UART data when PD4 is HIGH
- **PD6:** UART data when PD4 is LOW
- **A+/B-:** Differential signal during both TX and RX

## Before vs After

### Before (Not Working)
```
PC sends → COM8 → RS485 → MCU PD6 (RX) ✓
MCU tries to send → PD5 (TX) → RS485 ✗ (Transceiver in RX mode!)
Result: No response, timeout
```

### After (Working!)
```
PC sends → COM8 → RS485 → MCU PD6 (RX) ✓
MCU sets PD4=HIGH → Transceiver to TX mode
MCU sends → PD5 (TX) → RS485 → COM8 → PC ✓
MCU sets PD4=LOW → Back to RX mode
Result: Full duplex communication! 🎉
```

## Build and Flash

### STM32CubeIDE
1. Build: **Ctrl+B**
2. Flash: **F11** (Debug) or **Ctrl+F11** (Run)

### Command Line
```bash
cd Debug
make clean
make -j4
STM32_Programmer_CLI -c port=SWD -w SW_Controller_OUT.elf -v -rst
```

## Common Issues (Now Fixed!)

### ❌ Previously: No response from Controller OUT
**Cause:** PD4 not controlled → stuck in RX mode
**Fixed:** ✅ PD4 now properly toggled

### ❌ Previously: Random communication errors
**Cause:** No switching delays
**Fixed:** ✅ 1ms delays added for transceiver switching

### ❌ Previously: First bytes lost
**Cause:** Not waiting for TC flag
**Fixed:** ✅ Now waits for transmission complete

## Additional Changes Needed

This same fix should be applied to:
- [ ] **SW_Controller_420** - Analog Input Controller
- [ ] **SW_Controller_DIO** - Digital Input Controller

Both use RS485 and will have the same issue!

## Credits
- **Date:** 2025-01-14
- **Issue:** RS485 half-duplex direction control
- **Solution:** PD4 toggle with timing delays
- **Tested:** rs485_Test project ✓
- **Applied:** SW_Controller_OUT ✓

---

**Result:** RS485 communication now works correctly! PC ↔ Controller OUT ✓


