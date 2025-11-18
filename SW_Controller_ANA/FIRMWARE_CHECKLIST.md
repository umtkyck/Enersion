# STM32 Firmware Development Checklist
## Proven Patterns from SW_Controller_OUT & SW_Controller_DI

This checklist ensures consistent, error-free firmware development across all controller projects.

---

## ✅ PERIPHERAL CONFIGURATION

### UART Configuration
- [ ] **USART1 (Debug Console)**
  - Baud rate: 115200
  - Word length: 8 bits
  - Stop bits: 1
  - Parity: None
  - GPIO: TX/RX pins configured
  - **NO interrupt needed** (polling for debug)

- [ ] **USART2 (RS485)**
  - Baud rate: 115200
  - Word length: 8 bits
  - Stop bits: 1
  - Parity: None
  - GPIO: TX/RX pins configured
  - **Interrupt REQUIRED:** `USART2_IRQn` enabled
  - FIFO: **DISABLED** (`HAL_UARTEx_DisableFifoMode`)
  - COM pin: GPIO output for DE/RE control

### SPI Configuration (for ADC chips)
- [ ] **SPI1 & SPI4**
  - Mode: Master
  - Data size: 8-bit or 16-bit
  - Clock polarity: Check datasheet
  - Clock phase: Check datasheet
  - NSS: GPIO output (software control)
  - DMA: Optional for high-speed ADC

---

## ✅ HEADER FILE INCLUDES

### Standard C Libraries
```c
#include <string.h>    // For memcpy, memset
#include <stdint.h>    // For uint8_t, uint16_t, etc.
#include <math.h>      // For NTC calculations
```

### HAL & Application Headers
```c
#include "main.h"
#include "version.h"
#include "debug_uart.h"
#include "rs485_protocol.h"
#include "analog_input_handler.h"  // or digital_input_handler.h
```

### Critical Rule
❌ **NEVER use DEBUG_INFO/ERROR inside interrupt handlers**
✅ Only use debug in main loop or command handlers

---

## ✅ RS485 PROTOCOL IMPLEMENTATION

### File: `rs485_protocol.c`

#### 1. Includes & Variables
```c
#include "rs485_protocol.h"
#include "debug_uart.h"
#include "version.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

static uint8_t myAddress = RS485_ADDR_CONTROLLER_XXX;  // Set correct address
static volatile uint8_t txInProgress = 0;  // Loopback prevention flag
```

#### 2. Initialization
```c
void RS485_Init(uint8_t myAddr)
{
    myAddress = myAddr;
    memset(&status, 0, sizeof(status));
    
    // CRITICAL: Disable FIFO to prevent overrun
    HAL_UARTEx_DisableFifoMode(&huart2);
    
    // Initialize COM pin to RX mode (LOW)
    HAL_GPIO_WritePin(RS485_XXX_COM_GPIO_Port, RS485_XXX_COM_Pin, GPIO_PIN_RESET);
    
    // Register default handlers
    RS485_RegisterCommandHandler(CMD_PING, RS485_HandlePing);
    RS485_RegisterCommandHandler(CMD_GET_VERSION, RS485_HandleGetVersion);
    RS485_RegisterCommandHandler(CMD_HEARTBEAT, RS485_HandleHeartbeat);
    RS485_RegisterCommandHandler(CMD_GET_STATUS, RS485_HandleGetStatus);
    
    // Start RX interrupt
    HAL_UART_Receive_IT(&huart2, rxBuffer, 1);
    
    DEBUG_INFO("RS485 Protocol initialized, Address: 0x%02X", myAddress);
}
```

#### 3. Transmit Function
```c
HAL_StatusTypeDef RS485_SendPacket(...)
{
    // Set TX in progress flag
    txInProgress = 1;
    
    // Disable RX interrupt during TX
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);
    
    // Set COM pin HIGH (TX mode)
    HAL_GPIO_WritePin(RS485_XXX_COM_GPIO_Port, RS485_XXX_COM_Pin, GPIO_PIN_SET);
    
    // Busy-wait delay (NOT HAL_Delay in interrupt context!)
    for(volatile uint32_t i = 0; i < 240000; i++) { __NOP(); }
    
    // Transmit
    HAL_StatusTypeDef result = HAL_UART_Transmit(&huart2, txBuffer, packetSize, RS485_TIMEOUT_MS);
    
    // Wait for TC flag
    while(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET);
    
    // Busy-wait delay
    for(volatile uint32_t i = 0; i < 240000; i++) { __NOP(); }
    
    // Set COM pin LOW (RX mode)
    HAL_GPIO_WritePin(RS485_XXX_COM_GPIO_Port, RS485_XXX_COM_Pin, GPIO_PIN_RESET);
    
    // Re-enable RX interrupt
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    
    // Clear TX in progress flag
    txInProgress = 0;
    
    return result;
}
```

#### 4. Interrupt Handler
```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        // Ignore RX during TX (loopback prevention)
        if (txInProgress) {
            HAL_UART_Receive_IT(&huart2, rxBuffer, 1);
            return;
        }
        
        // NO DEBUG HERE!
        RS485_ProcessReceivedByte(rxBuffer[0]);
        HAL_UART_Receive_IT(&huart2, rxBuffer, 1);
    }
}
```

#### 5. Packet Processing
```c
static void RS485_ProcessReceivedByte(uint8_t byte)
{
    // NO DEBUG_INFO HERE!
    // Only update static variables and buffers
    // Packet timeout: 500ms between bytes
    
    static uint32_t lastByteTime = 0;
    uint32_t now = HAL_GetTick();
    
    if (now - lastByteTime > 500 && packetIndex > 0) {
        packetIndex = 0;
        expectedLength = 0;
    }
    lastByteTime = now;
    
    // Parse packet logic...
}
```

---

## ✅ INTERRUPT CONFIGURATION

### File: `stm32h7xx_it.c`

Add USART2 interrupt handler:
```c
/* USER CODE BEGIN 1 */

extern UART_HandleTypeDef huart2;

void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}

/* USER CODE END 1 */
```

### File: `stm32h7xx_hal_msp.c`

Enable NVIC for USART2:
```c
/* USER CODE BEGIN USART2_MspInit 1 */

/* USART2 interrupt Init (RS485) */
HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(USART2_IRQn);

/* USER CODE END USART2_MspInit 1 */
```

---

## ✅ VERSION INFORMATION

### File: `version.h`
```c
#ifndef VERSION_H
#define VERSION_H

#include <stdint.h>  // CRITICAL: Must include this!

#define FW_VERSION_MAJOR        1
#define FW_VERSION_MINOR        0
#define FW_VERSION_PATCH        0
#define FW_BUILD_NUMBER         1

#define MCU_ID                  0xXX  // Unique per controller
#define MCU_NAME                "CONTROLLER_XXX"

void Version_GetString(char* buffer, uint32_t size);
uint32_t Version_GetFirmwareVersion(void);

#endif
```

---

## ✅ MAIN APPLICATION

### File: `main.c`

#### Includes
```c
#include <string.h>  // CRITICAL: For memcpy
#include "version.h"
#include "debug_uart.h"
#include "rs485_protocol.h"
#include "xxx_handler.h"
```

#### Initialization Sequence
```c
void main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    // Initialize peripherals
    MX_GPIO_Init();
    MX_USART1_UART_Init();  // Debug
    MX_USART2_UART_Init();  // RS485
    MX_SPI1_Init();         // ADC (if needed)
    MX_SPI4_Init();         // ADC (if needed)
    
    // Initialize application layers
    Debug_Init();
    
    // Print startup banner
    Version_GetString(versionString, VERSION_STRING_SIZE);
    DEBUG_INFO("===========================================");
    DEBUG_INFO("  %s", versionString);
    DEBUG_INFO("===========================================");
    
    // Initialize handlers
    XXX_Handler_Init();
    
    // Initialize RS485 with correct address
    RS485_Init(RS485_ADDR_CONTROLLER_XXX);
    
    // Register command handlers
    RS485_RegisterCommandHandler(CMD_XXX, HandleXXX);
    
    DEBUG_INFO("System initialization complete");
    
    // Main loop
    while (1) {
        RS485_Process();
        XXX_Handler_Update();
        
        HAL_Delay(1);
    }
}
```

#### Command Handlers
```c
void HandleReadXXX(const RS485_Packet_t* packet)
{
    DEBUG_INFO("READ_XXX command from 0x%02X", packet->srcAddr);
    
    // Get data as floats (little-endian)
    uint8_t data[NUM_CHANNELS * 4];
    
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        float value = XXX_GetValue(i);
        memcpy(&data[i * 4], &value, 4);
        
        if (i < 5) {
            DEBUG_DEBUG("Ch%d: %.2f", i, value);
        }
    }
    
    RS485_SendResponse(packet->srcAddr, CMD_XXX_RESPONSE, data, sizeof(data));
    
    DEBUG_INFO("Data sent (%d bytes)", sizeof(data));
}
```

---

## ✅ GUI APPLICATION CHECKLIST

### File Structure
```
GUI_Application_XXX/
  ├── main_gui.py
  ├── rs485_protocol.py  (copy from DO)
  ├── version.py
  ├── requirements.txt
  └── README.md
```

### File: `version.py`
```python
APP_VERSION_MAJOR = 1
APP_VERSION_MINOR = 0
APP_VERSION_PATCH = 0
APP_BUILD_NUMBER = 1

APP_NAME = "XXX Controller"
APP_DESCRIPTION = "..."
APP_COMPANY = "Enersion"

# Compatibility aliases
VERSION_MAJOR = APP_VERSION_MAJOR
VERSION_MINOR = APP_VERSION_MINOR
VERSION_PATCH = APP_VERSION_PATCH
VERSION_BUILD = APP_BUILD_NUMBER
VERSION_NAME = APP_NAME
```

### File: `main_gui.py`

#### Imports
```python
import sys
import struct
import traceback
from PyQt5.QtWidgets import (...)
from PyQt5.QtCore import Qt, QTimer
import serial.tools.list_ports

from rs485_protocol import RS485Protocol, RS485_ADDR_CONTROLLER_XXX
from version import get_version_string, VERSION_NAME, ...
```

#### Connection Handling
```python
def connect(self):
    try:
        # Close existing protocol if any
        if self.protocol:
            try:
                self.protocol.disconnect()
            except:
                pass
        
        baudrate = int(self.baud_combo.currentText())
        self.protocol = RS485Protocol(port, baudrate)
        
        if self.protocol.connect():
            # Wait 1.5 seconds for MCU to be ready
            QTimer.singleShot(1500, self.scan_device_after_connect)
        else:
            QMessageBox.critical(self, "Error", "Failed to connect...")
            
    except Exception as e:
        QMessageBox.critical(self, "Error", f"Connection error: {e}\n\n{traceback.format_exc()}")
```

#### Data Parsing
```python
# Unpack floats from response (little-endian)
if response and len(response) >= expected_bytes:
    for i in range(num_channels):
        offset = i * 4
        value = struct.unpack('<f', response[offset:offset+4])[0]
        # Update GUI with value
```

---

## ✅ COMMON ERRORS TO AVOID

### ❌ ERROR 1: Missing includes
```c
// BAD: No string.h
memcpy(&data[i], &value, 4);  // Implicit declaration warning

// GOOD:
#include <string.h>
memcpy(&data[i], &value, 4);  // OK
```

### ❌ ERROR 2: Debug in interrupt
```c
// BAD:
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    DEBUG_INFO("RX: 0x%02X", data);  // TOO SLOW!
}

// GOOD:
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // NO DEBUG HERE - just process quickly
    RS485_ProcessReceivedByte(data);
}
```

### ❌ ERROR 3: FIFO enabled
```c
// BAD: FIFO can cause overrun
// (default from CubeMX)

// GOOD: Explicitly disable
HAL_UARTEx_DisableFifoMode(&huart2);
```

### ❌ ERROR 4: Missing interrupt handler
```c
// BAD: Forgot to add USART2_IRQHandler
// Result: No RX interrupts!

// GOOD: Add to stm32h7xx_it.c
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}
```

### ❌ ERROR 5: Wrong COM pin control
```c
// BAD: Using wrong pin name
HAL_GPIO_WritePin(RS485_COM_OUT_GPIO_Port, RS485_COM_OUT_Pin, ...);  // In DI controller!

// GOOD: Use correct pin for each controller
// DO:  RS485_COM_OUT_GPIO_Port, RS485_COM_OUT_Pin
// DI:  RS485_DI_COM_GPIO_Port, RS485_DI_COM_Pin
// ANA: RS485_ANA_COM_GPIO_Port, RS485_ANA_COM_Pin
```

### ❌ ERROR 6: Short connection timeout
```python
# BAD: Only 500ms wait
QTimer.singleShot(500, self.scan_devices)

# GOOD: 1500ms for MCU to stabilize
QTimer.singleShot(1500, self.scan_devices)
```

### ❌ ERROR 7: Missing version aliases
```python
# BAD: Only APP_VERSION_MAJOR defined
# main_gui.py tries to use VERSION_MAJOR → NameError!

# GOOD: Add compatibility aliases
VERSION_MAJOR = APP_VERSION_MAJOR
VERSION_MINOR = APP_VERSION_MINOR
VERSION_NAME = APP_NAME
```

---

## ✅ BUILD & TEST CHECKLIST

### Before Committing
- [ ] Code compiles with **0 errors, 0 warnings**
- [ ] All includes present (`string.h`, `stdint.h`)
- [ ] Version numbers match (firmware & GUI)
- [ ] RS485 address correct for this controller
- [ ] COM pin names correct for this controller
- [ ] FIFO disabled for UART2
- [ ] Interrupt handlers added (stm32h7xx_it.c)
- [ ] NVIC enabled (stm32h7xx_hal_msp.c)
- [ ] No DEBUG in interrupt context
- [ ] GUI has all compatibility aliases
- [ ] README.md updated

### Testing
1. Build firmware → Flash to MCU
2. Open debug UART → See startup banner
3. Run GUI → Connect via RS485
4. Test PING → Should respond
5. Test GET_VERSION → Should return version
6. Test data commands → Should return values

---

## 📋 PROJECT-SPECIFIC CHECKLIST

### SW_Controller_OUT (Digital Outputs)
- Address: `RS485_ADDR_CONTROLLER_OUT` (0x03)
- COM pin: `RS485_COM_OUT_GPIO_Port`, `RS485_COM_OUT_Pin`
- Commands: WRITE_DO, READ_DO
- Data: 56 outputs (7 bytes)

### SW_Controller_DI (Digital Inputs)
- Address: `RS485_ADDR_CONTROLLER_DIO` (0x02)
- COM pin: `RS485_DI_COM_GPIO_Port`, `RS485_DI_COM_Pin`
- Commands: READ_DI
- Data: 56 inputs (7 bytes)

### SW_Controller_ANA (Analog Inputs)
- Address: `RS485_ADDR_CONTROLLER_420` (0x01)
- COM pin: `RS485_ANA_COM_GPIO_Port`, `RS485_ANA_COM_Pin`
- Commands: READ_ANALOG_420, READ_ANALOG_VOLTAGE, READ_NTC
- Data: 26 floats (104 bytes), 6 floats (24 bytes), 4 floats (16 bytes)

---

## 🎯 SUCCESS CRITERIA

✅ **Firmware compiles without errors**
✅ **GUI connects to controller**
✅ **PING responds correctly**
✅ **Version matches between firmware & GUI**
✅ **Data commands return expected format**
✅ **No crashes or restarts**
✅ **Clean debug console output**

---

**Use this checklist for all future STM32 + RS485 + GUI projects!**

