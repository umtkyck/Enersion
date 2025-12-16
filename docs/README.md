# Enersion Documentation Index

## Quick Links

| Document | Description |
|----------|-------------|
| [../README.md](../README.md) | Main project overview |
| [HARDWARE.md](HARDWARE.md) | Hardware connections and peripherals |
| [FIRMWARE.md](FIRMWARE.md) | STM32H7 firmware documentation |
| [SOFTWARE.md](SOFTWARE.md) | Linux application documentation |

## Project Documents

| Location | Document | Purpose |
|----------|----------|---------|
| `docs/` | HARDWARE.md | DI, DO, Analog, RS485, CAN, STM32MP257 connections |
| `docs/` | FIRMWARE.md | STM32H753 firmware for all controllers |
| `docs/` | SOFTWARE.md | Enterprise Linux application |
| `LinuxApp/` | PROJECT_STATUS.md | Detailed progress tracking |
| `LinuxApp/` | MYIR_CONNECTION.md | Board connection guide |
| Root | PROJECT_SUMMARY.md | High-level project summary |
| Root | QUICK_START_GUIDE.md | Getting started guide |
| Root | HARDWARE_CAPABILITY_MATRIX.md | Hardware capabilities |

## Document Structure

```
Enersion/
├── README.md                    # Main README
├── docs/
│   ├── README.md               # This file
│   ├── HARDWARE.md             # Hardware documentation
│   ├── FIRMWARE.md             # Firmware documentation
│   └── SOFTWARE.md             # Software documentation
└── LinuxApp/
    ├── README.md               # Linux app overview
    ├── PROJECT_STATUS.md       # Detailed progress
    └── MYIR_CONNECTION.md      # Connection info
```

## Quick Reference

### Hardware Addresses
- DI Controller: 0x02
- DO Controller: 0x03
- ANA Controller: 0x01
- Master (Linux): 0x10

### MYIR Connection
- IP: 192.168.0.10
- User: root
- Password: 123

### RS485 Configuration
- Device: /dev/ttySTM9
- Baud: 115200 8N1
- Direction GPIO: 138 (PI10)

