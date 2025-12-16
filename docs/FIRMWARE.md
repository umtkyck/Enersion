# Enersion Firmware Documentation

## Table of Contents

1. [Overview](#overview)
2. [Firmware Architecture](#firmware-architecture)
3. [DI Controller Firmware](#di-controller-firmware)
4. [DO Controller Firmware](#do-controller-firmware)
5. [ANA Controller Firmware](#ana-controller-firmware)
6. [Communication Protocol](#communication-protocol)
7. [Build & Flash](#build--flash)
8. [Testing](#testing)

---

## Overview

The Enersion system uses STM32H753ZIT6 microcontrollers for all peripheral controllers. Each controller runs dedicated firmware to manage its specific function (DI, DO, or Analog).

### Firmware Summary

| Controller | Folder | Address | Function | Status |
|------------|--------|---------|----------|--------|
| Digital Input | `SW_Controller_DI/` | 0x02 | 64 input monitoring | ✅ Complete |
| Digital Output | `SW_Controller_OUT/` | 0x03 | 64 output control | ✅ Complete |
| Analog Input | `SW_Controller_ANA/` | 0x01 | 16 analog channels | 🔄 In Progress |

---

## Firmware Architecture

### Layered Design

```
┌─────────────────────────────────────────────────────────┐
│                   Application Layer                      │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │  DI Logic   │  │  DO Logic   │  │  ANA Logic  │     │
│  └─────────────┘  └─────────────┘  └─────────────┘     │
├─────────────────────────────────────────────────────────┤
│                   Protocol Layer                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │          Enersion Protocol (CRC, Packets)          │  │
│  └───────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────┤
│                    Driver Layer                          │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐  │
│  │  GPIO    │  │  UART    │  │  SPI     │  │  Timer │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────┘  │
├─────────────────────────────────────────────────────────┤
│                    HAL Layer                             │
│  ┌───────────────────────────────────────────────────┐  │
│  │              STM32 HAL Library                     │  │
│  └───────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────┤
│                    Hardware                              │
│  ┌───────────────────────────────────────────────────┐  │
│  │              STM32H753ZIT6 MCU                     │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Common Components

All controllers share:
- RS485 communication module
- Enersion protocol stack
- CRC-16 calculation
- Watchdog timer
- Status LED management

---

## DI Controller Firmware

**Location:** `SW_Controller_DI/`

### File Structure

```
SW_Controller_DI/
├── Core/
│   ├── Inc/
│   │   ├── main.h              # Main declarations
│   │   ├── gpio.h              # GPIO configuration
│   │   ├── usart.h             # UART/RS485 config
│   │   ├── protocol.h          # Protocol definitions
│   │   ├── di_manager.h        # DI logic
│   │   └── stm32h7xx_it.h      # Interrupt handlers
│   └── Src/
│       ├── main.c              # Main loop
│       ├── gpio.c              # GPIO initialization
│       ├── usart.c             # RS485 UART driver
│       ├── protocol.c          # Protocol handler
│       ├── di_manager.c        # DI reading logic
│       ├── stm32h7xx_it.c      # ISR implementations
│       └── system_stm32h7xx.c  # System config
├── Drivers/
│   └── STM32H7xx_HAL_Driver/   # HAL library
├── SW_Controller_DI.ioc        # STM32CubeMX project
└── Debug/
    └── SW_Controller_DI.elf    # Compiled binary
```

### Main Loop Flow

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();              // Initialize 64 DI pins
    MX_USART3_UART_Init();       // RS485 UART
    MX_TIM2_Init();              // Debounce timer
    
    Protocol_Init(ADDR_CTRL_DIO); // Address 0x02
    DI_Manager_Init();
    
    while (1)
    {
        DI_Manager_Update();      // Read all inputs
        Protocol_Process();       // Handle RS485 commands
        HAL_IWDG_Refresh();       // Feed watchdog
    }
}
```

### GPIO Configuration (64 Inputs)

```c
// di_manager.c

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} DI_Channel_t;

static const DI_Channel_t di_channels[64] = {
    // Bank 0: PA0-PA15 (DI 0-15)
    {GPIOA, GPIO_PIN_0},  {GPIOA, GPIO_PIN_1},  /* ... */
    // Bank 1: PB0-PB15 (DI 16-31)
    {GPIOB, GPIO_PIN_0},  {GPIOB, GPIO_PIN_1},  /* ... */
    // Bank 2: PC0-PC15 (DI 32-47)
    {GPIOC, GPIO_PIN_0},  {GPIOC, GPIO_PIN_1},  /* ... */
    // Bank 3: PD0-PD15 (DI 48-63)
    {GPIOD, GPIO_PIN_0},  {GPIOD, GPIO_PIN_1},  /* ... */
};

// Read all 64 inputs into 8-byte array
void DI_Manager_ReadAll(uint8_t* buffer)
{
    for (int i = 0; i < 64; i++) {
        uint8_t state = HAL_GPIO_ReadPin(
            di_channels[i].port, 
            di_channels[i].pin
        );
        
        // Pack into bytes (8 bits per byte)
        if (state == GPIO_PIN_SET) {
            buffer[i / 8] |= (1 << (i % 8));
        } else {
            buffer[i / 8] &= ~(1 << (i % 8));
        }
    }
}
```

### Debounce Implementation

```c
// di_manager.c

#define DEBOUNCE_COUNT  3      // Number of stable reads
#define DEBOUNCE_TIME   10     // ms between reads

static uint8_t raw_state[8];
static uint8_t debounced_state[8];
static uint8_t debounce_counter[64];

void DI_Manager_Debounce(void)
{
    uint8_t current[8];
    DI_Manager_ReadRaw(current);
    
    for (int i = 0; i < 64; i++) {
        uint8_t bit = (current[i/8] >> (i%8)) & 0x01;
        uint8_t stable = (debounced_state[i/8] >> (i%8)) & 0x01;
        
        if (bit != stable) {
            debounce_counter[i]++;
            if (debounce_counter[i] >= DEBOUNCE_COUNT) {
                // State changed, update debounced
                if (bit) {
                    debounced_state[i/8] |= (1 << (i%8));
                } else {
                    debounced_state[i/8] &= ~(1 << (i%8));
                }
                debounce_counter[i] = 0;
            }
        } else {
            debounce_counter[i] = 0;
        }
    }
}
```

### Command Handlers

```c
// protocol.c

void Protocol_HandleCommand(EnersionPacket_t* packet)
{
    switch (packet->cmd) {
        case CMD_PING:
            Protocol_SendPingResponse();
            break;
            
        case CMD_GET_VERSION:
            Protocol_SendVersion(FW_VERSION_MAJOR, 
                                 FW_VERSION_MINOR,
                                 FW_VERSION_PATCH);
            break;
            
        case CMD_GET_STATUS:
            Protocol_SendStatus(GetSystemStatus());
            break;
            
        case CMD_READ_DI:
            uint8_t states[8];
            DI_Manager_GetDebounced(states);
            Protocol_SendDIResponse(states, 8);
            break;
            
        default:
            Protocol_SendError(ERR_UNKNOWN_CMD);
            break;
    }
}
```

---

## DO Controller Firmware

**Location:** `SW_Controller_OUT/`

### File Structure

```
SW_Controller_OUT/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── gpio.h
│   │   ├── usart.h
│   │   ├── protocol.h
│   │   ├── do_manager.h        # DO control logic
│   │   └── pwm_controller.h    # PWM for channels 0-15
│   └── Src/
│       ├── main.c
│       ├── gpio.c
│       ├── usart.c
│       ├── protocol.c
│       ├── do_manager.c
│       ├── pwm_controller.c
│       └── stm32h7xx_it.c
├── Drivers/
├── SW_Controller_OUT.ioc
└── Debug/
    └── SW_Controller_OUT.elf
```

### Main Loop Flow

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();              // Initialize 64 DO pins
    MX_USART3_UART_Init();       // RS485 UART
    MX_TIM1_Init();              // PWM timer
    
    Protocol_Init(ADDR_CTRL_OUT); // Address 0x03
    DO_Manager_Init();
    DO_Manager_SetSafeState();    // All outputs OFF
    
    while (1)
    {
        Protocol_Process();       // Handle RS485 commands
        DO_Manager_Update();      // Apply pending changes
        
        // Watchdog - if no communication, safe state
        if (Protocol_GetIdleTime() > WATCHDOG_TIMEOUT) {
            DO_Manager_SetSafeState();
        }
        
        HAL_IWDG_Refresh();
    }
}
```

### GPIO Configuration (64 Outputs)

```c
// do_manager.c

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t pwm_capable;
} DO_Channel_t;

static const DO_Channel_t do_channels[64] = {
    // Bank 0: PE0-PE15 (DO 0-15) - PWM capable
    {GPIOE, GPIO_PIN_0, 1},  {GPIOE, GPIO_PIN_1, 1},  /* ... */
    // Bank 1: PF0-PF15 (DO 16-31)
    {GPIOF, GPIO_PIN_0, 0},  {GPIOF, GPIO_PIN_1, 0},  /* ... */
    // Bank 2: PG0-PG15 (DO 32-47)
    {GPIOG, GPIO_PIN_0, 0},  {GPIOG, GPIO_PIN_1, 0},  /* ... */
    // Bank 3: PH0-PH15 (DO 48-63)
    {GPIOH, GPIO_PIN_0, 0},  {GPIOH, GPIO_PIN_1, 0},  /* ... */
};

static uint8_t output_state[8];

// Write all 64 outputs from 8-byte array
void DO_Manager_WriteAll(const uint8_t* states)
{
    memcpy(output_state, states, 8);
    
    for (int i = 0; i < 64; i++) {
        uint8_t bit = (states[i / 8] >> (i % 8)) & 0x01;
        
        HAL_GPIO_WritePin(
            do_channels[i].port,
            do_channels[i].pin,
            bit ? GPIO_PIN_SET : GPIO_PIN_RESET
        );
    }
}

// Set single output
void DO_Manager_SetChannel(uint8_t channel, uint8_t state)
{
    if (channel >= 64) return;
    
    if (state) {
        output_state[channel/8] |= (1 << (channel%8));
    } else {
        output_state[channel/8] &= ~(1 << (channel%8));
    }
    
    HAL_GPIO_WritePin(
        do_channels[channel].port,
        do_channels[channel].pin,
        state ? GPIO_PIN_SET : GPIO_PIN_RESET
    );
}
```

### Failsafe Mode

```c
// do_manager.c

#define FAILSAFE_TIMEOUT_MS  5000  // 5 seconds no comms

static uint32_t last_command_time = 0;

void DO_Manager_Update(void)
{
    uint32_t now = HAL_GetTick();
    
    // Check for communication timeout
    if ((now - last_command_time) > FAILSAFE_TIMEOUT_MS) {
        DO_Manager_SetSafeState();
    }
}

void DO_Manager_SetSafeState(void)
{
    // Turn off all outputs
    memset(output_state, 0, sizeof(output_state));
    
    for (int i = 0; i < 64; i++) {
        HAL_GPIO_WritePin(
            do_channels[i].port,
            do_channels[i].pin,
            GPIO_PIN_RESET
        );
    }
}

void DO_Manager_OnCommandReceived(void)
{
    last_command_time = HAL_GetTick();
}
```

### Command Handlers

```c
// protocol.c

void Protocol_HandleCommand(EnersionPacket_t* packet)
{
    switch (packet->cmd) {
        case CMD_PING:
            Protocol_SendPingResponse();
            DO_Manager_OnCommandReceived();
            break;
            
        case CMD_WRITE_DO:
            if (packet->len == 8) {
                DO_Manager_WriteAll(packet->data);
                Protocol_SendDOResponse(DO_SUCCESS);
            } else {
                Protocol_SendError(ERR_INVALID_LENGTH);
            }
            DO_Manager_OnCommandReceived();
            break;
            
        case CMD_READ_DO:
            uint8_t states[8];
            DO_Manager_GetStates(states);
            Protocol_SendDOReadResponse(states, 8);
            break;
            
        case CMD_SET_SINGLE_DO:
            // Data: [channel, state]
            if (packet->len == 2) {
                DO_Manager_SetChannel(packet->data[0], 
                                      packet->data[1]);
                Protocol_SendDOResponse(DO_SUCCESS);
            }
            DO_Manager_OnCommandReceived();
            break;
            
        default:
            Protocol_SendError(ERR_UNKNOWN_CMD);
            break;
    }
}
```

---

## ANA Controller Firmware

**Location:** `SW_Controller_ANA/`

### File Structure

```
SW_Controller_ANA/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── spi.h               # SPI for AD4114
│   │   ├── usart.h
│   │   ├── protocol.h
│   │   ├── ad4114.h            # ADC driver
│   │   └── ana_manager.h       # Analog processing
│   └── Src/
│       ├── main.c
│       ├── spi.c
│       ├── usart.c
│       ├── protocol.c
│       ├── ad4114.c
│       ├── ana_manager.c
│       └── stm32h7xx_it.c
├── Drivers/
├── SW_Controller_ANA.ioc
└── Debug/
    └── SW_Controller_ANA.elf
```

### AD4114 ADC Interface

```c
// ad4114.h

#define AD4114_NUM_CHANNELS    16

typedef struct {
    int32_t raw_value;          // 24-bit ADC reading
    float voltage;              // Calculated voltage
    float current_ma;           // Calculated current (4-20mA)
    float scaled_value;         // User-scaled value
    uint8_t status;             // Channel status
} AD4114_Channel_t;

// Initialize AD4114
HAL_StatusTypeDef AD4114_Init(SPI_HandleTypeDef* hspi);

// Read single channel
HAL_StatusTypeDef AD4114_ReadChannel(uint8_t channel, 
                                     AD4114_Channel_t* result);

// Read all channels
HAL_StatusTypeDef AD4114_ReadAll(AD4114_Channel_t* results);

// Configure sample rate
HAL_StatusTypeDef AD4114_SetSampleRate(uint16_t rate_sps);
```

### AD4114 Driver Implementation

```c
// ad4114.c

static SPI_HandleTypeDef* ad4114_spi;

HAL_StatusTypeDef AD4114_Init(SPI_HandleTypeDef* hspi)
{
    ad4114_spi = hspi;
    
    // Reset AD4114
    AD4114_Reset();
    HAL_Delay(10);
    
    // Verify communication
    uint8_t id = AD4114_ReadID();
    if (id != AD4114_DEVICE_ID) {
        return HAL_ERROR;
    }
    
    // Configure channels
    for (int ch = 0; ch < AD4114_NUM_CHANNELS; ch++) {
        AD4114_ConfigChannel(ch, AD4114_GAIN_1, AD4114_REF_INT);
    }
    
    // Set sample rate (default 100 SPS)
    AD4114_SetSampleRate(100);
    
    return HAL_OK;
}

HAL_StatusTypeDef AD4114_ReadChannel(uint8_t channel, 
                                     AD4114_Channel_t* result)
{
    // Select channel
    AD4114_SelectChannel(channel);
    
    // Wait for conversion
    while (!AD4114_IsReady()) {
        // Timeout handling
    }
    
    // Read 24-bit value
    result->raw_value = AD4114_ReadData();
    
    // Convert to voltage (assuming 2.5V reference)
    result->voltage = (float)result->raw_value * 2.5f / 8388607.0f;
    
    // Convert to current (250Ω shunt)
    result->current_ma = result->voltage / 250.0f * 1000.0f;
    
    return HAL_OK;
}
```

### Analog Processing

```c
// ana_manager.c

#define FILTER_SAMPLES  8

typedef struct {
    AD4114_Channel_t current;
    int32_t filter_buffer[FILTER_SAMPLES];
    uint8_t filter_index;
    float scale_min;
    float scale_max;
    float scale_unit;
} ANA_Channel_t;

static ANA_Channel_t channels[16];

void ANA_Manager_Update(void)
{
    for (int i = 0; i < 16; i++) {
        AD4114_Channel_t reading;
        AD4114_ReadChannel(i, &reading);
        
        // Moving average filter
        channels[i].filter_buffer[channels[i].filter_index] = 
            reading.raw_value;
        channels[i].filter_index = 
            (channels[i].filter_index + 1) % FILTER_SAMPLES;
        
        int32_t sum = 0;
        for (int j = 0; j < FILTER_SAMPLES; j++) {
            sum += channels[i].filter_buffer[j];
        }
        
        reading.raw_value = sum / FILTER_SAMPLES;
        channels[i].current = reading;
        
        // Apply user scaling
        // 4mA = scale_min, 20mA = scale_max
        float percent = (reading.current_ma - 4.0f) / 16.0f;
        channels[i].current.scaled_value = 
            channels[i].scale_min + 
            percent * (channels[i].scale_max - channels[i].scale_min);
    }
}
```

---

## Communication Protocol

### Packet Format

```
┌───────┬──────┬─────┬─────┬─────┬────────────┬───────┬──────┐
│ START │ DEST │ SRC │ CMD │ LEN │   DATA     │ CRC16 │ END  │
│ 0xAA  │  1B  │ 1B  │ 1B  │ 1B  │ 0-250 bytes│  2B   │ 0x55 │
└───────┴──────┴─────┴─────┴─────┴────────────┴───────┴──────┘
```

### CRC-16 Implementation

```c
// enersion_crc.c

uint16_t Enersion_CRC16(const uint8_t* data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return crc;
}
```

### RS485 Direction Control

```c
// rs485.c

#define RS485_DE_PIN    GPIO_PIN_0
#define RS485_DE_PORT   GPIOE

void RS485_EnableTX(void)
{
    HAL_GPIO_WritePin(RS485_DE_PORT, RS485_DE_PIN, GPIO_PIN_SET);
}

void RS485_EnableRX(void)
{
    HAL_GPIO_WritePin(RS485_DE_PORT, RS485_DE_PIN, GPIO_PIN_RESET);
}

void RS485_Send(const uint8_t* data, uint16_t length)
{
    RS485_EnableTX();
    HAL_UART_Transmit(&huart3, (uint8_t*)data, length, 1000);
    RS485_EnableRX();
}
```

### Command Reference

| Code | Name | Direction | Data Format |
|------|------|-----------|-------------|
| 0x01 | PING | M→S | None |
| 0x02 | PING_RESP | S→M | None |
| 0x03 | GET_VERSION | M→S | None |
| 0x04 | VERSION_RESP | S→M | [Major, Minor, Patch] |
| 0x10 | GET_STATUS | M→S | None |
| 0x11 | STATUS_RESP | S→M | [Status bytes] |
| 0x20 | READ_DI | M→S | None |
| 0x21 | DI_RESP | S→M | [8 bytes - 64 bits] |
| 0x30 | WRITE_DO | M→S | [8 bytes - 64 bits] |
| 0x31 | DO_RESP | S→M | [Result code] |
| 0x32 | READ_DO | M→S | None |
| 0x33 | DO_READ_RESP | S→M | [8 bytes - 64 bits] |
| 0x40 | READ_ANA | M→S | None |
| 0x41 | ANA_RESP | S→M | [16 x 4 bytes = 64 bytes] |
| 0xFF | ERROR | S→M | [Error code] |

---

## Build & Flash

### Using STM32CubeIDE

1. **Open Project:**
   ```
   File → Import → Existing Projects into Workspace
   Select: SW_Controller_DI (or OUT, ANA)
   ```

2. **Build:**
   ```
   Project → Build Project (Ctrl+B)
   ```

3. **Flash:**
   ```
   Run → Debug (F11)
   Or: Run → Run (Ctrl+F11)
   ```

### Using Command Line

```bash
# Navigate to project
cd SW_Controller_DI

# Build
make clean
make all

# Flash using ST-Link
st-flash write Debug/SW_Controller_DI.elf 0x08000000

# Or using OpenOCD
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
  -c "program Debug/SW_Controller_DI.elf verify reset exit"
```

### Build Options

```makefile
# In Makefile or .cproject

# Optimization
OPT = -O2

# Debug symbols
DEBUG = -g3

# Warnings
WARNINGS = -Wall -Werror -Wextra

# MISRA compliance (use PC-lint or similar)
# MISRA_CHECK = 1
```

---

## Testing

### Unit Test Structure

```
tests/
├── test_protocol.c      # Protocol layer tests
├── test_di_manager.c    # DI logic tests
├── test_do_manager.c    # DO logic tests
├── test_ad4114.c        # ADC driver tests
└── test_runner.c        # Unity test runner
```

### Example Protocol Test

```c
// test_protocol.c

#include "unity.h"
#include "protocol.h"

void test_CRC16_KnownValues(void)
{
    uint8_t data[] = {0xAA, 0x02, 0x10, 0x01, 0x00};
    uint16_t crc = Enersion_CRC16(data, 5);
    TEST_ASSERT_EQUAL_HEX16(0x1234, crc); // Expected CRC
}

void test_PacketParse_Valid(void)
{
    uint8_t raw[] = {0xAA, 0x02, 0x10, 0x01, 0x00, 0x12, 0x34, 0x55};
    EnersionPacket_t packet;
    
    ParseResult_t result = Protocol_Parse(raw, 8, &packet);
    
    TEST_ASSERT_EQUAL(PARSE_OK, result);
    TEST_ASSERT_EQUAL(0x02, packet.dest);
    TEST_ASSERT_EQUAL(0x01, packet.cmd);
}

void test_PacketParse_InvalidCRC(void)
{
    uint8_t raw[] = {0xAA, 0x02, 0x10, 0x01, 0x00, 0xFF, 0xFF, 0x55};
    EnersionPacket_t packet;
    
    ParseResult_t result = Protocol_Parse(raw, 8, &packet);
    
    TEST_ASSERT_EQUAL(PARSE_CRC_ERROR, result);
}
```

### Hardware-in-Loop Testing

```bash
# On MYIR board, run test script
./test_rs485_myir.sh

# Expected output:
# Ping DI Controller (0x02)... OK
# Ping DO Controller (0x03)... OK
# Read DI states... OK [00 00 00 00 00 00 00 00]
# Write DO test pattern... OK
# Read DO states... OK [AA 55 AA 55 AA 55 AA 55]
```

---

## Troubleshooting

### Common Issues

| Issue | Possible Cause | Solution |
|-------|----------------|----------|
| No RS485 response | Wrong address | Verify slave address |
| CRC errors | Baud rate mismatch | Check 115200 8N1 |
| DI always HIGH | Input voltage too low | Check >10V input |
| DO not switching | Driver issue | Check ULN2803 |
| ADC reading stuck | SPI clock issue | Verify SPI config |

### Debug UART

All controllers have debug output on USART1:

```c
// Debug print
printf("DI State: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
       state[0], state[1], state[2], state[3],
       state[4], state[5], state[6], state[7]);
```

Connect USART1 TX to USB-UART converter for debugging.

---

*Document Version: 1.0*  
*Last Updated: December 2024*

