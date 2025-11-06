# Quick Start Guide - Enersion PLC Controller System

## Getting Started in 5 Minutes

### Step 1: Build and Flash Firmware

#### Controller 420 (4-20mA Interface)
1. Open `SW_Controller_420` in STM32CubeIDE
2. Build project (Ctrl+B)
3. Flash to MCU via ST-Link
4. Expected behavior: Status LED blinking every 500ms

#### Controller DIO (Digital Inputs)
1. Open `SW_Controller_DIO` in STM32CubeIDE
2. Build project (Ctrl+B)
3. Flash to MCU via ST-Link
4. Expected behavior: Status LED blinking every 500ms

#### Controller OUT (Digital Outputs)
1. Open `SW_Controller_OUT` in STM32CubeIDE
2. Build project (Ctrl+B)
3. Flash to MCU via ST-Link
4. Expected behavior: Status LED blinking every 500ms

### Step 2: Wire RS485 Network

```
PC with RS485 Adapter
    │
    ├── A (RS485+)
    ├── B (RS485-)
    └── GND
         │
         ├──[Controller 420]── A, B, GND
         │
         ├──[Controller DIO]── A, B, GND
         │
         └──[Controller OUT]── A, B, GND

Note: Add 120Ω termination resistor at both ends of bus if needed
```

### Step 3: Launch GUI

#### Option A: Run from source
```bash
cd GUI_Application
pip install -r requirements.txt
python main_gui.py
```

#### Option B: Use batch file (Windows)
```bash
cd GUI_Application
run.bat
```

### Step 4: Connect and Test

1. **Select Port**: Choose your RS485 adapter COM port
2. **Connect**: Click "Connect" button
3. **Scan**: Click "Tools" → "Scan Devices"
4. **Verify**: All three controllers should show "Connected" (green)

### Step 5: Test Digital I/O

#### Test Inputs (Controller DIO)
1. Click "Read Inputs" button
2. Green dots = HIGH, Gray dots = LOW
3. Apply signal to any input and read again

#### Test Outputs (Controller OUT)
1. Check a few output boxes
2. Click "Write Outputs" button
3. Verify outputs using multimeter or LED
4. Click "Read Outputs" to verify state

## Monitoring Health

- **Status**: Green = Connected, Red = Disconnected
- **Health Bar**: Shows system health (0-100%)
  - Green: 80-100% (Good)
  - Orange: 50-79% (Warning)
  - Red: 0-49% (Critical)
- **Uptime**: Hours:Minutes:Seconds since boot
- **Statistics**: Packet counts and errors

## Debug Output

### Viewing Debug Messages

1. Connect USB-UART adapter to USART1 of any controller
   - TX pin → USB-UART RX
   - GND → USB-UART GND

2. Open serial terminal (PuTTY, TeraTerm, etc.)
   - Port: Your USB-UART COM port
   - Baud: 115200
   - Data: 8 bits
   - Parity: None
   - Stop: 1 bit

3. Reset MCU to see startup messages

### Expected Debug Output

```
[    1234] [INFO ] ===========================================
[    1235] [INFO ]   CONTROLLER_DIO v1.0.0.1 HW:R1M1 Built: Oct 27 2025 11:00:00
[    1236] [INFO ] ===========================================
[    1240] [INFO ] Debug UART initialized
[    1245] [INFO ] Digital Input Handler initialized, 54 inputs
[    1250] [INFO ] RS485 Protocol initialized, Address: 0x02
[    1255] [INFO ] System initialization complete
[    1260] [INFO ] Entering main loop...
[   11260] [INFO ] Heartbeat: Uptime=10 RX=5 TX=5 Err=0 Health=100%
```

## Building Executable

```bash
cd GUI_Application
python build_exe.py
```

Executable will be in: `dist/PLC_Controller_GUI.exe`

Distribute this single file - no Python installation needed!

## Troubleshooting

### Problem: GUI can't find COM port
**Solution**: 
- Install RS485 adapter drivers
- Check Device Manager (Windows)
- Try different USB port

### Problem: No devices found
**Solution**:
- Verify RS485 wiring (A, B, GND)
- Check MCU power supply (3.3V)
- Verify firmware is flashed
- Check debug UART output

### Problem: Communication errors
**Solution**:
- Check baud rate (must be 115200)
- Verify RS485 termination
- Check cable length (< 100m recommended)
- Add termination resistors (120Ω)

### Problem: High error count
**Solution**:
- Check RS485 signal quality
- Reduce cable length
- Add twisted pair cable
- Add termination resistors
- Check for electromagnetic interference

### Problem: Outputs don't change
**Solution**:
- Verify "Write Outputs" was clicked
- Check power supply to output circuits
- Verify GPIO configuration in firmware
- Check debug output for errors
- Use multimeter to verify output voltage

## Command Line Testing

### Using Python directly

```python
from rs485_protocol import RS485Protocol

# Connect
protocol = RS485Protocol('COM3', 115200)
protocol.connect()

# Ping device
if protocol.ping(0x01):
    print("Controller 420 is alive!")

# Get version
version = protocol.get_version(0x02)
print(f"Controller DIO: {version}")

# Read inputs
inputs = protocol.read_digital_inputs(0x02)
print(f"Inputs: {inputs.hex()}")

# Write outputs (all off)
protocol.write_digital_outputs(0x03, b'\x00\x00\x00\x00\x00\x00\x00\x00')

# Disconnect
protocol.disconnect()
```

## System Requirements

### Firmware Development
- STM32CubeIDE 1.10.0 or later
- ST-Link programmer
- Windows/Linux/Mac

### GUI Application
- Python 3.8 or later
- Windows/Linux/Mac
- RS485 USB adapter

### Hardware
- STM32H753ZI microcontrollers (3x)
- RS485 transceiver on each board
- Power supply (3.3V or 5V depending on board)
- RS485 twisted pair cable

## Next Steps

1. ✅ Test basic communication
2. ✅ Verify all digital I/O
3. ✅ Monitor health status
4. [ ] Integrate into your application
5. [ ] Add custom commands if needed
6. [ ] Deploy to production

## Support

- Check `PROJECT_SUMMARY.md` for complete documentation
- Review `GUI_Application/README.md` for GUI details
- Examine `HW_ENERSION_CONTROLLER_R1M1.pdf` for hardware info

## Tips

- Always verify RS485 connections before powering on
- Monitor debug output during development
- Use health monitoring to detect issues early
- Keep firmware versions synchronized
- Back up working configurations

---
**Happy Coding!** 🚀


