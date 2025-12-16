# Enersion Hardware Documentation

## Table of Contents

1. [System Overview](#system-overview)
2. [Controller Boards](#controller-boards)
3. [Peripheral Connections](#peripheral-connections)
4. [Communication Interfaces](#communication-interfaces)
5. [Power Requirements](#power-requirements)
6. [Wiring Diagrams](#wiring-diagrams)

---

## System Overview

The Enersion system consists of multiple controller boards interconnected via RS485 industrial bus.

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        ENERSION CONTROL SYSTEM                          │
│                                                                         │
│  ┌────────────────────────────────────────────────────────────────┐    │
│  │                    MYIR STM32MP257 HMI                         │    │
│  │                   ┌─────────────────┐                          │    │
│  │    ┌─────────┐   │   Linux OS      │   ┌─────────────────┐    │    │
│  │    │ HDMI    │   │ ├─ Web Server   │   │    Ethernet     │    │    │
│  │    │ Touch   │◄──┤ ├─ Qt GUI       │   │  192.168.0.10   │    │    │
│  │    │ Display │   │ └─ CLI Tools    │   └─────────────────┘    │    │
│  │    └─────────┘   └────────┬────────┘                          │    │
│  │                           │ RS485 (/dev/ttySTM9)              │    │
│  └───────────────────────────┼────────────────────────────────────┘    │
│                              │                                         │
│         ┌────────────────────┼────────────────────┐                    │
│         │                    │                    │                    │
│         ▼                    ▼                    ▼                    │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐             │
│  │ DI Controller│    │ DO Controller│    │ ANA Controller│            │
│  │  STM32H753   │    │  STM32H753   │    │  STM32H753   │             │
│  │  Addr: 0x02  │    │  Addr: 0x03  │    │  Addr: 0x01  │             │
│  │              │    │              │    │              │             │
│  │  64x DI      │    │  64x DO      │    │  16x AI      │             │
│  │  (24VDC)     │    │  (50V/500mA) │    │  (4-20mA)    │             │
│  └──────────────┘    └──────────────┘    └──────────────┘             │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Controller Boards

### 1. Digital Input Controller (DI)

**MCU:** STM32H753ZIT6  
**Address:** 0x02  
**Channels:** 64 isolated digital inputs

#### Specifications

| Parameter | Value |
|-----------|-------|
| Input Voltage | 24VDC nominal (12-30VDC range) |
| Isolation | 2500V optocoupler |
| Input Current | ~5mA per channel |
| Response Time | <1ms |
| Debounce | Software configurable (1-100ms) |

#### Pin Mapping

```
STM32H753 GPIO → Optocoupler → Terminal Block
─────────────────────────────────────────────

Bank 0 (DI 0-15):   PA0-PA15
Bank 1 (DI 16-31):  PB0-PB15
Bank 2 (DI 32-47):  PC0-PC15
Bank 3 (DI 48-63):  PD0-PD15

RS485:
  TX: USART3_TX (PD8)
  RX: USART3_RX (PD9)
  DE: GPIO (PE0)
```

#### Input Circuit

```
    Field Device                    STM32H753
    ────────────────────────────────────────────
         ┌─────────┐    ┌─────────────┐
    (+)──┤ 2.2kΩ   ├────┤ Optocoupler ├──── GPIO (Input)
         └─────────┘    │  PC817      │
                        │             │
    (-)─────────────────┤   GND       ├──── GND
                        └─────────────┘
    
    Input Characteristic:
    - ON Threshold:  >10VDC
    - OFF Threshold: <5VDC
    - Max Voltage:   30VDC
```

---

### 2. Digital Output Controller (DO)

**MCU:** STM32H753ZIT6  
**Address:** 0x03  
**Channels:** 64 digital outputs

#### Specifications

| Parameter | Value |
|-----------|-------|
| Output Type | Open-drain (ULN2803A) |
| Max Voltage | 50VDC per channel |
| Max Current | 500mA per channel |
| Response Time | <100μs |
| PWM Capable | Channels 0-15 |

#### Pin Mapping

```
STM32H753 GPIO → ULN2803 → Terminal Block
──────────────────────────────────────────

Bank 0 (DO 0-15):   PE0-PE15  (PWM capable)
Bank 1 (DO 16-31):  PF0-PF15
Bank 2 (DO 32-47):  PG0-PG15
Bank 3 (DO 48-63):  PH0-PH15

RS485:
  TX: USART3_TX (PD8)
  RX: USART3_RX (PD9)
  DE: GPIO (PD10)
```

#### Output Circuit

```
    STM32H753          ULN2803A           Load
    ─────────────────────────────────────────────
                      ┌──────────┐
    GPIO ────────────►│ IN   OUT ├─────┬───► (+) Load
    (Output)          │          │     │
                      │    COM ──┼─────┴── (+) Supply
                      │          │         (up to 50V)
    GND ─────────────►│ GND      │
                      └──────────┘         (-) Load ─► GND
    
    Note: Flyback diode integrated in ULN2803
```

---

### 3. Analog Input Controller (ANA)

**MCU:** STM32H753ZIT6  
**Address:** 0x01  
**Channels:** 16 analog inputs (4-20mA)

#### Specifications

| Parameter | Value |
|-----------|-------|
| Input Type | 4-20mA current loop |
| ADC | AD4114 (24-bit Σ-Δ) |
| Resolution | 24-bit |
| Sample Rate | 10 SPS - 31.25 kSPS |
| Accuracy | ±0.01% |
| Isolation | Channel-to-channel |

#### Pin Mapping

```
STM32H753 SPI → AD4114 → Input Terminals
─────────────────────────────────────────

SPI2:
  MOSI: PB15
  MISO: PB14
  SCK:  PB13
  CS:   PB12
  
AD4114 Channels:
  AIN0-AIN15: 16 differential inputs

RS485:
  TX: USART3_TX (PD8)
  RX: USART3_RX (PD9)
  DE: GPIO (PD10)
```

#### Input Circuit

```
    4-20mA Sensor                   AD4114
    ────────────────────────────────────────
           ┌─────────┐
    (+)────┤         ├──── AIN+ (Channel)
           │  250Ω   │
           │  0.1%   ├──── 1V - 5V
           │         │
    (-)────┤  Shunt  ├──── AIN- (Channel)
           └─────────┘
    
    Voltage = Current × 250Ω
    4mA  = 1.0V
    20mA = 5.0V
```

---

### 4. MYIR STM32MP257 HMI Board

**Model:** MYD-LD25X  
**CPU:** STM32MP257 (Cortex-A35 + Cortex-M33)  
**OS:** Linux (OpenSTLinux)

#### Specifications

| Parameter | Value |
|-----------|-------|
| Processor | STM32MP257 dual Cortex-A35 @ 1.5GHz |
| Co-processor | Cortex-M33 @ 250MHz |
| RAM | 1GB DDR4 |
| Storage | 8GB eMMC + microSD |
| Display | HDMI (up to 1920x1080) |
| Touch | USB HID touch support |
| Ethernet | Gigabit (eth0) |
| RS485 | /dev/ttySTM9 (J2 connector) |

#### Connector J2 (RS485)

```
    J2 Connector (MY-WIREDCOM module)
    ───────────────────────────────────
    
    Pin 5: RS485_A (+)
    Pin 6: RS485_B (-)
    
    UART9 → /dev/ttySTM9
    Direction GPIO: PI10 (GPIO 138)
```

---

## Peripheral Connections

### Complete System Wiring

```
┌──────────────────────────────────────────────────────────────────────────┐
│                          POWER DISTRIBUTION                               │
│                                                                          │
│      ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│      │  24VDC PSU  │     │  5VDC PSU   │     │  3.3V REG   │            │
│      │  (Main)     │     │  (Logic)    │     │  (MCU)      │            │
│      └──────┬──────┘     └──────┬──────┘     └──────┬──────┘            │
│             │                   │                   │                    │
│      ┌──────┴──────┐     ┌──────┴──────┐     ┌──────┴──────┐            │
│      │ Field Power │     │ Output Drv  │     │ STM32 MCUs  │            │
│      │ (DI, DO)    │     │ (ULN2803)   │     │ (All boards)│            │
│      └─────────────┘     └─────────────┘     └─────────────┘            │
└──────────────────────────────────────────────────────────────────────────┘
```

### RS485 Bus Termination

```
                     RS485 BUS (A/B)
    ────────────────────────────────────────────────────
         │           │           │           │
    ┌────┴────┐ ┌────┴────┐ ┌────┴────┐ ┌────┴────┐
    │ 120Ω   │ │         │ │         │ │  120Ω  │
    │ Term   │ │  DI     │ │  DO     │ │  Term  │
    │        │ │  Ctrl   │ │  Ctrl   │ │        │
    │ MYIR   │ │         │ │         │ │  ANA   │
    │ Master │ │         │ │         │ │  Ctrl  │
    └────────┘ └─────────┘ └─────────┘ └────────┘
    
    Note: 120Ω termination at both ends of bus
```

---

## Communication Interfaces

### RS485 Configuration

| Parameter | Value |
|-----------|-------|
| Baud Rate | 115200 |
| Data Bits | 8 |
| Parity | None |
| Stop Bits | 1 |
| Mode | Half-duplex |
| Protocol | Enersion Custom |

### RS485 Direction Control (MYIR)

```bash
# Export GPIO 138 (PI10)
echo 138 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio138/direction

# Set to Receive mode
echo 0 > /sys/class/gpio/gpio138/value

# Set to Transmit mode
echo 1 > /sys/class/gpio/gpio138/value
```

### CAN Bus (Future)

| Parameter | Value |
|-----------|-------|
| Standard | CAN 2.0B |
| Baud Rate | 500kbps (configurable) |
| Mode | Normal / Listen-only |
| Device | /dev/can0 |

---

## Power Requirements

### Power Budget

| Board | Voltage | Current (Typ) | Current (Max) |
|-------|---------|---------------|---------------|
| MYIR HMI | 5VDC | 500mA | 1A |
| DI Controller | 3.3VDC | 150mA | 300mA |
| DO Controller | 5VDC | 200mA | 500mA (+ loads) |
| ANA Controller | 3.3VDC | 100mA | 200mA |

### Power Supply Recommendations

```
┌─────────────────────────────────────────────────────────┐
│                  POWER SUPPLY DESIGN                     │
│                                                         │
│   AC Input ──► 24VDC/10A PSU ──┬──► Field Power         │
│                                │     (DI sensors, DO)   │
│                                │                        │
│                                └──► DC-DC 5V/2A ──────► │
│                                     │                   │
│                                     ├──► MYIR Board     │
│                                     ├──► DO Drivers     │
│                                     │                   │
│                                     └──► LDO 3.3V ────► │
│                                          │              │
│                                          ├──► DI MCU    │
│                                          ├──► DO MCU    │
│                                          └──► ANA MCU   │
└─────────────────────────────────────────────────────────┘
```

---

## Wiring Diagrams

### Digital Input Wiring

```
    Field Device                         DI Controller
    (PNP Sensor)                        Terminal Block
    ─────────────────────────────────────────────────────
    
    (+24V) ────────┬─────────────────► (+) Power Rail
                   │
            ┌──────┴──────┐
            │   Sensor    │
            │   (PNP)     │
            └──────┬──────┘
                   │
    (Signal)───────┴─────────────────► DI Channel
                                       (e.g., DI0)
    
    (0V) ────────────────────────────► (-) Common
```

### Digital Output Wiring

```
    DO Controller                        Load
    Terminal Block                       (24V Relay)
    ─────────────────────────────────────────────────────
    
    (+24V Supply) ──────┬───────────► (+) Relay Coil
                        │
    (DO Channel) ───────┴───────────► (-) Relay Coil
    e.g., DO0                           (Sinks to GND)
    
    Note: DO is open-drain, sinks current to ground
```

### Analog Input Wiring (4-20mA)

```
    4-20mA Transmitter                  ANA Controller
    (2-wire Loop)                       Terminal Block
    ─────────────────────────────────────────────────────
    
    (+24V Loop) ───────┐
                       │
                ┌──────┴──────┐
                │ Transmitter │
                │  4-20mA     │
                └──────┬──────┘
                       │
    (Signal+) ─────────┴─────────────► AI+ (Channel)
                                       e.g., AI0+
    
    (Signal-) ───────────────────────► AI- (Channel)
                                       e.g., AI0-
    
    Note: 250Ω shunt resistor inside ANA Controller
```

---

## Troubleshooting

### RS485 Communication Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No response | Wrong address | Verify device address |
| Garbled data | Wrong baud rate | Check 115200 8N1 |
| Intermittent | Termination | Add 120Ω at bus ends |
| Timeout | Direction GPIO | Check PI10 switching |

### GPIO Not Exporting

```bash
# Check if GPIO is already exported
ls /sys/class/gpio/gpio138

# If not, export it
echo 138 > /sys/class/gpio/export

# Check kernel config for sysfs GPIO support
dmesg | grep gpio
```

---

## Reference Documents

- **HW_ENERSION_CONTROLLER_R1M1.pdf** - Main schematic
- **STM32H753ZIT6 Datasheet** - MCU specifications
- **AD4114 Datasheet** - 24-bit ADC specifications
- **STM32MP257 Reference Manual** - Linux board specs

---

*Document Version: 1.0*  
*Last Updated: December 2024*

