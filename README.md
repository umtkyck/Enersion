# Enersion Industrial Control System

<div align="center">

```
    ⚡ ENERSION ⚡
    Industrial Control System
    Digital I/O • Analog • RS485 • CAN
```

**Enterprise-grade industrial control system for DI/DO/Analog management**

[![Platform](https://img.shields.io/badge/Platform-STM32-blue)]()
[![MCU](https://img.shields.io/badge/MCU-STM32H753-orange)]()
[![Linux](https://img.shields.io/badge/Linux-STM32MP257-green)]()
[![Protocol](https://img.shields.io/badge/Protocol-RS485-red)]()

</div>

---

## 📋 Table of Contents

1. [System Overview](#-system-overview)
2. [Hardware](#-hardware)
3. [Firmware](#-firmware)
4. [Software](#-software)
5. [Getting Started](#-getting-started)
6. [Project Structure](#-project-structure)
7. [Documentation](#-documentation)

---

## 🎯 System Overview

The Enersion Control System is a comprehensive industrial automation solution designed for:

- **64 Digital Inputs** - Isolated input monitoring
- **64 Digital Outputs** - High-current relay/transistor outputs
- **16 Analog Inputs** - 4-20mA current loop with AD4114 ADC
- **RS485 Communication** - Multi-drop industrial bus
- **CAN Bus** - Future expansion capability
- **Linux HMI** - Touchscreen interface on STM32MP257

### System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    MYIR STM32MP257 Linux Board                   │
│                         (Master Controller)                      │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │  Web GUI    │  │   Qt GUI    │  │   CLI App   │              │
│  │  (Python)   │  │   (C++/QML) │  │   (C)       │              │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘              │
│         └────────────────┼────────────────┘                      │
│                    ┌─────┴─────┐                                 │
│                    │ Enersion  │                                 │
│                    │ Protocol  │                                 │
│                    └─────┬─────┘                                 │
│                          │ RS485 (/dev/ttySTM9)                  │
└──────────────────────────┼──────────────────────────────────────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
         ▼                 ▼                 ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│  DI Controller  │ │  DO Controller  │ │  ANA Controller │
│  (STM32H753)    │ │  (STM32H753)    │ │  (STM32H753)    │
│  Address: 0x02  │ │  Address: 0x03  │ │  Address: 0x01  │
├─────────────────┤ ├─────────────────┤ ├─────────────────┤
│  64 Digital     │ │  64 Digital     │ │  16 Analog      │
│  Inputs         │ │  Outputs        │ │  Inputs (4-20mA)│
│  (Isolated)     │ │  (Relay/FET)    │ │  (AD4114 ADC)   │
└─────────────────┘ └─────────────────┘ └─────────────────┘
```

---

## 🔧 Hardware

### Controller Boards

| Board | MCU | Function | Channels | Status |
|-------|-----|----------|----------|--------|
| **DI Controller** | STM32H753ZIT | Digital Input | 64 DI | ✅ Ready |
| **DO Controller** | STM32H753ZIT | Digital Output | 64 DO | ✅ Ready |
| **ANA Controller** | STM32H753ZIT | Analog Input | 16 AI (4-20mA) | 🔄 In Progress |
| **Linux HMI** | STM32MP257 | Master/HMI | - | ✅ Ready |

### Hardware Connections

#### RS485 Bus

```
┌──────────────────────────────────────────────────────────────┐
│                       RS485 BUS                               │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐         │
│  │ MYIR    │  │ DI Ctrl │  │ DO Ctrl │  │ ANA Ctrl│         │
│  │ Master  │  │  0x02   │  │  0x03   │  │  0x01   │         │
│  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘         │
│       │            │            │            │               │
│  ─────┴────────────┴────────────┴────────────┴───────────── │
│       A(+)                                           A(+)    │
│  ─────┬────────────┬────────────┬────────────┬───────────── │
│       │            │            │            │               │
│       B(-)                                           B(-)    │
│                                                              │
│  Parameters: 115200 baud, 8N1, Half-duplex                  │
└──────────────────────────────────────────────────────────────┘
```

#### Digital Input Connections (DI Controller)

```
                    ┌─────────────────────┐
                    │   DI Controller     │
                    │   (STM32H753)       │
                    │                     │
    Field Device    │  ┌───────────────┐  │
    ────────────────┼──│ Optocoupler   │──┼── GPIO
    (24V/120V)      │  │ Isolation     │  │
                    │  └───────────────┘  │
                    │                     │
                    │  Channels: 64       │
                    │  Isolation: 2500V   │
                    │  Input: 24VDC       │
                    └─────────────────────┘
```

#### Digital Output Connections (DO Controller)

```
                    ┌─────────────────────┐
                    │   DO Controller     │
                    │   (STM32H753)       │
                    │                     │
    GPIO ───────────┼──┤ Driver IC    ├───┼──── Load
                    │  │ (ULN2803)    │   │   (Relay/Valve)
                    │  └──────────────┘   │
                    │                     │
                    │  Channels: 64       │
                    │  Output: 500mA/ch   │
                    │  Voltage: 50V max   │
                    └─────────────────────┘
```

#### Analog Input Connections (ANA Controller)

```
                    ┌─────────────────────────┐
                    │    ANA Controller       │
                    │    (STM32H753)          │
                    │                         │
    4-20mA ─────────┼──┤ 250Ω ├──┤ AD4114 ├──┼── SPI
    Sensor          │  Shunt     24-bit ADC  │
                    │                         │
                    │  Channels: 16           │
                    │  Resolution: 24-bit     │
                    │  Input: 4-20mA          │
                    │  Accuracy: 0.01%        │
                    └─────────────────────────┘
```

#### MYIR STM32MP257 Linux Board

```
┌────────────────────────────────────────────────────┐
│              MYIR MYD-LD25X                        │
│              (STM32MP257)                          │
├────────────────────────────────────────────────────┤
│                                                    │
│  ┌──────────────┐    ┌──────────────┐             │
│  │   Cortex-A35 │    │  Cortex-M33  │             │
│  │   (Linux)    │    │  (RTOS/Bare) │             │
│  └──────────────┘    └──────────────┘             │
│                                                    │
│  Interfaces:                                       │
│  ├─ RS485: /dev/ttySTM9 (J2 Connector)           │
│  │         GPIO PI10 (138) - Direction            │
│  ├─ HDMI: Touchscreen Display                     │
│  ├─ Ethernet: 192.168.0.10                        │
│  ├─ USB: Debug/Programming                        │
│  └─ CAN: Future expansion                         │
│                                                    │
│  Storage: eMMC + SD Card                          │
│  RAM: 1GB DDR4                                    │
│                                                    │
└────────────────────────────────────────────────────┘
```

### Pin Assignments

#### RS485 Interface (MYIR J2 Connector)

| Pin | Signal | Description |
|-----|--------|-------------|
| 5 | RS485_A | Positive differential |
| 6 | RS485_B | Negative differential |
| GND | Ground | Common ground |

#### RS485 Direction Control

| GPIO | Pin Name | Function |
|------|----------|----------|
| 138 | PI10 | nRTS (Direction Control) |

| State | Mode | Description |
|-------|------|-------------|
| LOW (0) | Receive | Enable RX, disable TX |
| HIGH (1) | Transmit | Enable TX, disable RX |

---

## 💾 Firmware

### STM32H7 Controller Firmware

#### DI Controller (`SW_Controller_DI/`)

```
SW_Controller_DI/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── gpio.h
│   │   ├── usart.h
│   │   └── protocol.h
│   └── Src/
│       ├── main.c
│       ├── gpio.c              # 64 DI GPIO config
│       ├── usart.c             # RS485 UART
│       └── protocol.c          # Enersion protocol
├── Drivers/                    # STM32 HAL
├── SW_Controller_DI.ioc        # STM32CubeMX config
└── Debug/                      # Build output
```

**Features:**
- 64 digital input channels
- Debounce filtering
- Input state change detection
- RS485 slave (Address: 0x02)
- Watchdog monitoring

**Commands Supported:**
| Command | Code | Description |
|---------|------|-------------|
| PING | 0x01 | Check online status |
| GET_VERSION | 0x03 | Firmware version |
| GET_STATUS | 0x10 | Controller status |
| READ_DI | 0x20 | Read all 64 inputs |

---

#### DO Controller (`SW_Controller_OUT/`)

```
SW_Controller_OUT/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── gpio.h
│   │   ├── usart.h
│   │   └── protocol.h
│   └── Src/
│       ├── main.c
│       ├── gpio.c              # 64 DO GPIO config
│       ├── usart.c             # RS485 UART
│       └── protocol.c          # Enersion protocol
├── Drivers/                    # STM32 HAL
├── SW_Controller_OUT.ioc       # STM32CubeMX config
└── Debug/                      # Build output
```

**Features:**
- 64 digital output channels
- PWM capability (selected channels)
- Output diagnostics
- RS485 slave (Address: 0x03)
- Failsafe mode

**Commands Supported:**
| Command | Code | Description |
|---------|------|-------------|
| PING | 0x01 | Check online status |
| GET_VERSION | 0x03 | Firmware version |
| GET_STATUS | 0x10 | Controller status |
| WRITE_DO | 0x30 | Write all 64 outputs |
| READ_DO | 0x32 | Read current states |

---

#### ANA Controller (`SW_Controller_ANA/`)

```
SW_Controller_ANA/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── spi.h
│   │   ├── ad4114.h            # ADC driver
│   │   └── protocol.h
│   └── Src/
│       ├── main.c
│       ├── spi.c               # SPI for AD4114
│       ├── ad4114.c            # 24-bit ADC driver
│       └── protocol.c          # Enersion protocol
├── Drivers/                    # STM32 HAL
├── SW_Controller_ANA.ioc       # STM32CubeMX config
└── Debug/                      # Build output
```

**Features:**
- 16 analog input channels
- AD4114 24-bit sigma-delta ADC
- 4-20mA current loop input
- Calibration storage
- RS485 slave (Address: 0x01)

---

### Firmware Build Instructions

```bash
# Using STM32CubeIDE
1. Open STM32CubeIDE
2. Import project (e.g., SW_Controller_DI)
3. Build Project (Ctrl+B)
4. Flash via ST-Link

# Using command line (ARM GCC)
cd SW_Controller_DI
make clean
make all
st-flash write Debug/SW_Controller_DI.elf 0x08000000
```

---

## 💻 Software

### Linux Application (`LinuxApp/`)

The enterprise-level software running on the MYIR STM32MP257 Linux board.

#### Architecture Layers

```
┌─────────────────────────────────────────────┐
│                 UI Layer                     │
│  ┌─────────────┐  ┌─────────────────────┐   │
│  │ Web GUI     │  │ Qt/QML GUI          │   │
│  │ (Python)    │  │ (C++)               │   │
│  └─────────────┘  └─────────────────────┘   │
├─────────────────────────────────────────────┤
│              Service Layer                   │
│  ┌─────────────┐  ┌─────────────────────┐   │
│  │ DI Service  │  │ DO Service          │   │
│  └─────────────┘  └─────────────────────┘   │
├─────────────────────────────────────────────┤
│              Protocol Layer                  │
│  ┌─────────────────────────────────────┐    │
│  │ Enersion Protocol (CRC, Packets)    │    │
│  └─────────────────────────────────────┘    │
├─────────────────────────────────────────────┤
│                HAL Layer                     │
│  ┌─────────────┐  ┌─────────────────────┐   │
│  │ RS485 Serial│  │ GPIO Direction      │   │
│  └─────────────┘  └─────────────────────┘   │
└─────────────────────────────────────────────┘
```

#### Implementation Options

| GUI Type | Technology | Status | Notes |
|----------|------------|--------|-------|
| **Web GUI** | Python + HTTP | ✅ Ready | Browser-based, touch-friendly |
| **Qt GUI** | C++ + QML | ✅ Code Ready | Needs Qt build |
| **CLI** | C | ✅ Ready | For testing/automation |

#### Directory Structure

```
LinuxApp/
├── python/                     # Python implementation
│   ├── web_app.py             # Web-based GUI server
│   ├── hal/
│   │   └── rs485.py           # RS485 HAL
│   └── protocol/
│       └── enersion.py        # Protocol implementation
│
├── src/
│   ├── hal/                   # C HAL (MISRA compliant)
│   │   ├── rs485_serial.c
│   │   └── rs485_gpio.c
│   ├── protocol/              # C Protocol
│   │   ├── enersion_protocol.c
│   │   └── enersion_crc.c
│   ├── test/                  # CLI test app
│   │   └── enersion_test.c
│   ├── app/                   # Qt backend
│   │   └── app_controller.cpp
│   └── ui/qml/                # QML UI files
│       ├── main.qml
│       └── pages/
│
└── CMakeLists.txt             # Build configuration
```

---

### Windows Test Applications

For development and testing on Windows PC:

| Application | Folder | Purpose |
|-------------|--------|---------|
| **DI Test GUI** | `GUI_Application_DI/` | Test DI controller |
| **DO Test GUI** | `GUI_Application_DO/` | Test DO controller |
| **ANA Test GUI** | `GUI_Application_ANA/` | Test Analog controller |

```bash
# Run Windows GUI
cd GUI_Application_DI
python main_gui.py
```

---

## 🚀 Getting Started

### Prerequisites

- STM32CubeIDE (for firmware)
- Python 3.7+ (for Linux app)
- USB-RS485 converter (for testing)
- MYIR MYD-LD25X board

### Quick Start

#### 1. Flash Firmware

```bash
# Flash DI Controller
st-flash write SW_Controller_DI/Debug/SW_Controller_DI.elf 0x08000000

# Flash DO Controller
st-flash write SW_Controller_OUT/Debug/SW_Controller_OUT.elf 0x08000000
```

#### 2. Connect to MYIR Board

```bash
ssh root@192.168.0.10
# Password: 123
```

#### 3. Upload Linux App

```bash
scp -r LinuxApp/python root@192.168.0.10:/home/root/enersion/
```

#### 4. Run Application

```bash
# On MYIR board
cd /home/root/enersion/python
python3 web_app.py

# Open browser: http://192.168.0.10:8080
```

---

## 📁 Project Structure

```
Enersion/
│
├── 📂 Hardware Documentation
│   ├── HW_ENERSION_CONTROLLER_R1M1.pdf    # Schematic
│   ├── HARDWARE_CAPABILITY_MATRIX.md
│   └── HARDWARE_FIRMWARE_GUI_CROSSCHECK.md
│
├── 📂 Firmware
│   ├── SW_Controller_DI/                   # DI firmware
│   ├── SW_Controller_OUT/                  # DO firmware
│   ├── SW_Controller_ANA/                  # Analog firmware
│   ├── AD4114_1_Test/                      # ADC test project
│   └── rs485_Test/                         # RS485 test project
│
├── 📂 Linux Application
│   └── LinuxApp/
│       ├── python/                         # Python implementation
│       ├── src/                            # C/C++ implementation
│       └── CMakeLists.txt
│
├── 📂 Windows Test GUIs
│   ├── GUI_Application_DI/                 # DI test GUI
│   ├── GUI_Application_DO/                 # DO test GUI
│   └── GUI_Application_ANA/                # Analog test GUI
│
├── 📂 Libraries
│   ├── no-OS/                              # Analog Devices no-OS
│   └── arduino_ad4114_test/                # Arduino AD4114 lib
│
└── 📂 Documentation
    ├── README.md                           # This file
    ├── PROJECT_SUMMARY.md
    ├── QUICK_START_GUIDE.md
    └── COMPARISON_TEST.md
```

---

## 📖 Documentation

| Document | Description |
|----------|-------------|
| [PROJECT_STATUS.md](LinuxApp/PROJECT_STATUS.md) | Detailed progress tracking |
| [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md) | Getting started guide |
| [HARDWARE_CAPABILITY_MATRIX.md](HARDWARE_CAPABILITY_MATRIX.md) | Hardware specs |
| [LinuxApp/README.md](LinuxApp/README.md) | Linux app documentation |
| [LinuxApp/MYIR_CONNECTION.md](LinuxApp/MYIR_CONNECTION.md) | Board connection info |

---

## 📊 Protocol Reference

### Packet Format

```
┌───────┬──────┬─────┬─────┬─────┬────────────┬───────┬──────┐
│ START │ DEST │ SRC │ CMD │ LEN │   DATA     │ CRC16 │ END  │
│ 0xAA  │  1B  │ 1B  │ 1B  │ 1B  │ 0-250 bytes│  2B   │ 0x55 │
└───────┴──────┴─────┴─────┴─────┴────────────┴───────┴──────┘
```

### Device Addresses

| Address | Device | Description |
|---------|--------|-------------|
| 0x00 | Broadcast | All devices |
| 0x01 | CTRL_420 | 4-20mA Analog |
| 0x02 | CTRL_DIO | Digital Inputs |
| 0x03 | CTRL_OUT | Digital Outputs |
| 0x10 | Master | Linux HMI |

### Commands

| Code | Command | Direction | Description |
|------|---------|-----------|-------------|
| 0x01 | PING | Master → Slave | Check online |
| 0x02 | PING_RESP | Slave → Master | Ping reply |
| 0x03 | GET_VERSION | Master → Slave | Get firmware version |
| 0x10 | GET_STATUS | Master → Slave | Get status info |
| 0x20 | READ_DI | Master → DI | Read 64 inputs |
| 0x21 | DI_RESPONSE | DI → Master | Input states |
| 0x30 | WRITE_DO | Master → DO | Write 64 outputs |
| 0x31 | DO_RESPONSE | DO → Master | Write ACK |
| 0x32 | READ_DO | Master → DO | Read output states |

---

## 🔧 Development Status

### Completed ✅

- [x] Hardware schematic design
- [x] DI Controller firmware
- [x] DO Controller firmware
- [x] RS485 communication protocol
- [x] Linux HAL layer (C)
- [x] Linux Protocol layer (C)
- [x] Python HAL implementation
- [x] Python Protocol implementation
- [x] Web-based GUI (Python)
- [x] Qt/QML GUI design
- [x] Windows test applications
- [x] Documentation

### In Progress 🔄

- [ ] Analog controller firmware completion
- [ ] AD4114 ADC driver integration
- [ ] CAN bus implementation
- [ ] Auto-start service configuration
- [ ] Production testing

### Planned 📋

- [ ] Data logging/historian
- [ ] Alarm management
- [ ] Remote access (VPN/VNC)
- [ ] Multi-language support
- [ ] User authentication
- [ ] OTA firmware update

---

## 📝 License

© 2024 Enersion. All rights reserved.

---

## 📞 Contact

**Project:** Enersion Industrial Control System  
**Platform:** STM32H7 + STM32MP257  
**Protocol:** RS485 / Enersion Custom

---

<div align="center">

**⚡ ENERSION - Industrial Automation Excellence ⚡**

</div>

