# Analog Input Controller - RS485 GUI

GUI application for monitoring analog inputs from SW_Controller_ANA via RS485 communication.

## Features

- **26x 4-20mA Current Loop Inputs**
  - Real-time monitoring
  - Color-coded status (green: OK, red: fault/out-of-range)
  - Fault detection (wire break, over-range)

- **6x 0-10V Voltage Inputs**
  - Precision voltage monitoring
  - Range validation

- **4x NTC Temperature Sensors**
  - Temperature display in °C
  - Range: -40°C to +125°C

- **Auto-Refresh Mode**
  - Configurable refresh rate (1s default)
  - Manual read option

- **RS485 Communication**
  - 115200 baud (default)
  - CRC16 error checking
  - Device scanning

## Requirements

```
Python 3.7+
PyQt5
pyserial
```

Install dependencies:
```bash
pip install -r requirements.txt
```

## Usage

### 1. Hardware Setup
- Connect RS485 transceiver to PC (USB-RS485 adapter)
- Connect SW_Controller_ANA to RS485 bus
- Power on the controller
- Note: Controller address is **0x01** (RS485_ADDR_CONTROLLER_420)

### 2. Run GUI
```bash
python main_gui.py
```

### 3. Connect
1. Select COM port from dropdown
2. Verify baud rate (115200)
3. Click "Connect"
4. Wait for automatic device scan
5. Monitor analog inputs

### 4. Read Analog Inputs
- **Manual:** Click "Read All Now"
- **Auto:** Click "Start Auto-Refresh (1s)"

## RS485 Protocol

### Commands
- `CMD_READ_ANALOG_420` (0x40) - Read all 4-20mA inputs
- `CMD_READ_ANALOG_VOLTAGE` (0x42) - Read all 0-10V inputs
- `CMD_READ_NTC` (0x44) - Read all NTC temperatures
- `CMD_READ_ALL_ANALOG` (0x46) - Read all analog inputs at once

### Data Format
- **4-20mA Response:** 26 × 4 bytes (float, little-endian) = 104 bytes
- **0-10V Response:** 6 × 4 bytes (float, little-endian) = 24 bytes
- **NTC Response:** 4 × 4 bytes (float, little-endian) = 16 bytes

## Status Indicators

### 4-20mA
- **Green:** 4.0-20.0 mA (normal)
- **Orange:** 3.8-4.0 mA or 20.0-21.0 mA (warning)
- **Red:** <3.8 mA (wire break) or >21.0 mA (over-range)

### 0-10V
- **Green:** 0.0-10.0 V (normal)
- **Red:** Out of range

### NTC
- **Green:** -40 to +125 °C (normal)
- **Red:** Out of range

## Troubleshooting

### "Controller 420 not detected"
- Check RS485 connections
- Verify controller power
- Confirm baud rate (115200)
- Check controller address (0x01)
- Ensure firmware is flashed

### "Failed to read analog inputs"
- Check RS485 communication
- Verify ADC initialization
- Check SPI connection to ADC chips
- Review firmware debug messages (COM port for debug UART)

### Values show "-- mA/V/°C"
- Connection not established
- Click "Read All Now" to fetch values
- Enable "Auto-Refresh"

## Version

See `version.py` for current version information.

## License

Copyright © 2025 Enersion

