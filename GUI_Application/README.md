# PLC Controller GUI Application

Professional GUI application for controlling and monitoring Enersion PLC controllers via RS485 communication.

## Features

- **RS485 Communication**: Robust protocol implementation with CRC16 error checking
- **Multi-Controller Support**: Simultaneously monitor 3 MCUs:
  - Controller 420 (4-20mA interfaces)
  - Controller DIO (Digital Inputs)
  - Controller OUT (Digital Outputs)
- **Health Monitoring**: Real-time health status and heartbeat monitoring
- **Version Management**: Track firmware versions across all controllers
- **Digital I/O Control**: Read inputs and write outputs
- **Connection Management**: Auto-detect serial ports
- **Modern UI**: Clean, professional interface using PyQt5

## Architecture

### Hierarchical Layers

```
┌─────────────────────────────────────┐
│     GUI Layer (PyQt5)               │
│  - Main Window                      │
│  - MCU Widgets                      │
│  - Digital I/O Controls             │
└─────────────────────────────────────┘
           ↓
┌─────────────────────────────────────┐
│   Application Layer                 │
│  - Health Monitoring                │
│  - Event Handling                   │
│  - State Management                 │
└─────────────────────────────────────┘
           ↓
┌─────────────────────────────────────┐
│   Protocol Layer                    │
│  - Packet Encoding/Decoding         │
│  - CRC Calculation                  │
│  - Command Handling                 │
└─────────────────────────────────────┘
           ↓
┌─────────────────────────────────────┐
│   Hardware Layer (PySerial)         │
│  - RS485 Communication              │
│  - Serial Port Management           │
└─────────────────────────────────────┘
```

## Installation

### Option 1: Run from Source

1. Install Python 3.8 or higher

2. Install dependencies:
```bash
pip install -r requirements.txt
```

3. Run the application:
```bash
python main_gui.py
```

### Option 2: Build Executable

1. Install dependencies:
```bash
pip install -r requirements.txt
```

2. Build executable:
```bash
python build_exe.py
```

3. Executable will be created in `dist/` folder

## Usage

### Connecting to Controllers

1. **Select Serial Port**: Choose RS485 adapter from dropdown
2. **Set Baud Rate**: Default is 115200 (must match firmware)
3. **Click Connect**: Establishes RS485 communication
4. **Scan Devices**: Automatically detects connected controllers

### Monitoring

- **Health Status**: Real-time health percentage (0-100%)
- **Connection Status**: Green = Connected, Red = Disconnected
- **Uptime**: Controller uptime in HH:MM:SS format
- **Statistics**: Packet counts and error tracking
- **Heartbeat**: Automatic every 2 seconds

### Digital I/O Control

#### Reading Inputs (DIO Controller)
1. Click "Read Inputs" button
2. Green dots indicate active inputs
3. Gray dots indicate inactive inputs

#### Writing Outputs (OUT Controller)
1. Check desired output boxes
2. Click "Write Outputs" button
3. Read back current state with "Read Outputs"

## Communication Protocol

### Packet Structure
```
┌──────────────┬──────────────┬──────────────┬──────────────┐
│ Start (0xAA) │ Dest Address │  Src Address │   Command    │
├──────────────┼──────────────┼──────────────┼──────────────┤
│    Length    │     Data     │  CRC16 (L)   │  CRC16 (H)   │
├──────────────┴──────────────┴──────────────┴──────────────┤
│                    End Byte (0x55)                         │
└────────────────────────────────────────────────────────────┘
```

### MCU Addresses
- `0x01` - Controller 420 (4-20mA)
- `0x02` - Controller DIO (Digital Inputs)
- `0x03` - Controller OUT (Digital Outputs)
- `0x10` - GUI Application

### Supported Commands
- `PING` (0x01) - Check device presence
- `GET_VERSION` (0x03) - Retrieve firmware version
- `HEARTBEAT` (0x05) - Health status check
- `GET_STATUS` (0x10) - Detailed status information
- `READ_DI` (0x20) - Read digital inputs
- `WRITE_DO` (0x30) - Write digital outputs
- `READ_DO` (0x32) - Read current outputs
- `READ_ANALOG` (0x40) - Read analog values

## Firmware Compatibility

This GUI is designed to work with the hierarchical firmware architecture:

### Required Firmware Modules
1. **Version Module**: Firmware version reporting
2. **Debug UART**: UART1 @ 115200 baud for debugging
3. **RS485 Protocol**: UART2 configured as RS485
4. **Command Handlers**: Support for all protocol commands

### Firmware Version Format
- Major.Minor.Patch.Build (e.g., 1.0.0.1)

## Troubleshooting

### Connection Issues
- Verify RS485 adapter is connected
- Check COM port in Device Manager (Windows)
- Ensure baud rate matches firmware (115200)
- Verify RS485 termination resistors

### No Devices Found
- Check RS485 wiring (A, B, GND)
- Verify firmware is programmed correctly
- Check MCU power supply
- Use UART debug output to verify MCU operation

### Communication Errors
- High error count indicates:
  - Poor RS485 signal quality
  - Electromagnetic interference
  - Incorrect baud rate
  - Loose connections

## Development

### Project Structure
```
GUI_Application/
├── main_gui.py           # Main application
├── rs485_protocol.py     # Protocol implementation
├── version.py            # Version management
├── requirements.txt      # Dependencies
├── build_exe.py          # Executable builder
└── README.md             # This file
```

### Adding New Features

1. **New Command**: Add to `RS485Command` enum in `rs485_protocol.py`
2. **New Widget**: Create widget class in `main_gui.py`
3. **New Handler**: Register handler in protocol layer

## Version History

### v1.0.0.1 (Current)
- Initial release
- Multi-controller support
- Health monitoring
- Digital I/O control
- RS485 protocol implementation
- Executable packaging

## License

Copyright (c) 2025 Enersion
All rights reserved.

## Support

For technical support or questions:
- Check firmware UART debug output
- Verify hardware connections
- Review protocol documentation


