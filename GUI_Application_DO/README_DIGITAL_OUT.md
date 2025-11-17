# Digital OUT Controller - Test Application

## Overview
This is a simplified GUI application focused **ONLY** on testing the **Digital OUT Controller** (RS485 Address: 0x03).

- **56 Digital Output Channels** (DO0 - DO55)
- RS485 Communication Protocol
- Real-time read/write capabilities
- Health monitoring and heartbeat

## Hardware Setup

### Required Components
1. **Controller OUT** - Digital Output Module (RS485 Address: 0x03)
2. **RS485 to USB Converter** - Connected to PC
3. **Power Supply** - For the controller

### Wiring
```
PC (USB) <---> RS485 Converter <---> Controller OUT
                                      (Address: 0x03)
```

### RS485 Connection
- **A+** and **B-** terminals
- Default Baud Rate: **115200**
- Address: **0x03**

## Software Installation

### 1. Install Python Dependencies
```bash
pip install PyQt5 pyserial
```

Or use requirements file:
```bash
pip install -r requirements.txt
```

### 2. Run the Application

**Option A: Direct Python**
```bash
python main_gui.py
```

**Option B: Batch File**
```bash
run.bat
```

## How to Use

### Step 1: Connect to RS485
1. Select your **COM Port** from the dropdown
2. Choose **Baud Rate** (default: 115200)
3. Click **Connect** button

### Step 2: Scan for Device
1. Go to **Tools** → **Scan Device**
2. Wait for confirmation that Controller OUT is detected
3. Check the status panel shows "Connected" (green)

### Step 3: Control Outputs

#### Individual Control
- Check/uncheck any of the **DO00** to **DO55** checkboxes
- Click **✓ Write Outputs to Controller** to apply changes
- Status bar will show confirmation

#### Read Current State
- Click **↻ Read Current Output State**
- All checkboxes will update to match controller state

#### Bulk Operations
- **All ON** - Selects all 56 outputs (must still click Write)
- **All OFF** - Deselects all outputs (must still click Write)

## RS485 Protocol Details

### Command Set (Controller OUT Only)
| Command | Code | Description |
|---------|------|-------------|
| PING | 0x01 | Check if controller is alive |
| GET_VERSION | 0x03 | Read firmware version |
| HEARTBEAT | 0x05 | Health check with response |
| GET_STATUS | 0x10 | Read detailed status |
| WRITE_DO | 0x30 | Write 56 digital outputs (7 bytes) |
| READ_DO | 0x32 | Read current output state (7 bytes) |

### Data Format
**Digital Outputs:** 56 channels = 7 bytes
- Byte 0: DO0-DO7 (bit 0 to bit 7)
- Byte 1: DO8-DO15
- Byte 2: DO16-DO23
- Byte 3: DO24-DO31
- Byte 4: DO32-DO39
- Byte 5: DO40-DO47
- Byte 6: DO48-DO55

**Example:**
```python
# Turn ON DO0, DO5, DO10
output_bytes = [0x21, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00]
#                ^--- DO0=1, DO5=1
#                      ^--- DO10=1
```

## Troubleshooting

### Port Not Found
- Check USB cable connection
- Install USB-RS485 driver if needed
- Try different USB port

### Controller Not Detected
1. **Check Power** - Is controller powered on?
2. **Check Wiring** - A+/B- correctly connected?
3. **Check Address** - Controller must be configured as 0x03
4. **Check Baud Rate** - Must match (default: 115200)
5. **Check RS485 Termination** - May need 120Ω resistor

### Outputs Not Responding
1. Click **Read Outputs** to verify controller state
2. Check controller has power to output stages
3. Verify output load connections
4. Check for error messages in status bar

### Connection Timeouts
- Try reducing baud rate (57600 or 38400)
- Check for RS485 bus conflicts
- Verify cable quality and length (<1000m for RS485)

## Health Monitoring

The application continuously monitors:
- **Connection Status** - Green = connected, Red = disconnected
- **Health %** - Controller health (0-100%)
- **Uptime** - How long controller has been running
- **RX/TX Packets** - Communication statistics
- **Errors** - Communication error count

## Technical Specifications

### Protocol Layer
- **Start Byte:** 0xAA
- **End Byte:** 0x55
- **CRC:** CRC16 (Modbus style)
- **Max Packet Size:** 256 bytes
- **Timeout:** 100ms

### Application
- **Framework:** PyQt5
- **Python Version:** 3.8+
- **Platform:** Windows, Linux, macOS
- **Serial Library:** pyserial 3.5

## Version Information
- **Application:** PLC Controller GUI v1.0.0.1
- **Company:** Enersion
- **Mode:** Digital OUT Controller Only

## Support

For issues or questions:
1. Check troubleshooting section above
2. Verify hardware connections
3. Check serial console for debug messages
4. Contact Enersion support

---

**Last Updated:** November 2024
**Application Focus:** Digital OUT Controller Testing (56 Channels)



