# Enersion Linux Application

## Overview

Industrial control application for STM32MP257 MYIR board with HDMI touchscreen.
Provides Digital Input monitoring and Digital Output control via RS485 communication.

## Target Platform

- **Board**: STM32MP257 MYIR
- **Display**: HDMI Touchscreen
- **OS**: Linux (Yocto/OpenSTLinux)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Presentation Layer                        │
│                  (Qt Quick/QML - Modern UI)                  │
├─────────────────────────────────────────────────────────────┤
│                    Application Layer                         │
│              (Qt C++ - Controllers/ViewModels)               │
├─────────────────────────────────────────────────────────────┤
│                     Service Layer                            │
│               (Business Logic - DI/DO Manager)               │
├─────────────────────────────────────────────────────────────┤
│                    Protocol Layer                            │
│             (Enersion Protocol - MISRA 2012)                 │
├─────────────────────────────────────────────────────────────┤
│                       HAL Layer                              │
│              (RS485 Serial - MISRA 2012)                     │
└─────────────────────────────────────────────────────────────┘
```

## Features

- **Digital Input Monitor**: 64 channels real-time monitoring
- **Digital Output Control**: 64 channels with individual control
- **Connection Management**: RS485 port selection and monitoring
- **Health Monitoring**: Device status and heartbeat
- **Touch-Friendly UI**: Modern interface optimized for touchscreen

## Directory Structure

```
LinuxApp/
├── CMakeLists.txt              # Main build configuration
├── src/
│   ├── hal/                    # Hardware Abstraction Layer
│   │   ├── rs485_serial.h/c    # RS485 serial port (MISRA 2012)
│   │   └── rs485_types.h       # Type definitions
│   ├── protocol/               # Protocol Layer
│   │   ├── enersion_protocol.h/c
│   │   └── enersion_crc.h/c
│   ├── service/                # Service Layer
│   │   ├── device_manager.h/cpp
│   │   ├── di_service.h/cpp
│   │   └── do_service.h/cpp
│   ├── app/                    # Application Layer
│   │   ├── main.cpp
│   │   ├── app_controller.h/cpp
│   │   └── models/
│   └── ui/                     # Presentation Layer
│       ├── qml/
│       │   ├── main.qml
│       │   ├── components/
│       │   └── pages/
│       └── resources.qrc
├── tests/                      # Unit Tests
│   ├── test_protocol.cpp
│   ├── test_service.cpp
│   └── CMakeLists.txt
└── scripts/                    # Build/Deploy scripts
    ├── build.sh
    └── deploy.sh
```

## Building

### Local Development (x86_64)

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Cross-Compilation (STM32MP257)

```bash
source /opt/st/stm32mp2/environment-setup-cortexa35-ostl-linux
mkdir build-arm && cd build-arm
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/stm32mp257.cmake
make -j$(nproc)
```

## Deployment

```bash
# Copy to target
scp build-arm/enersion_gui root@192.168.1.100:/usr/bin/

# Run on target
ssh root@192.168.1.100 "/usr/bin/enersion_gui"
```

## Code Standards

- **C Code**: MISRA C:2012 compliant
- **C++ Code**: Modern C++17
- **QML**: Qt 6.x best practices

## License

Copyright (c) 2024 Enersion. All rights reserved.



