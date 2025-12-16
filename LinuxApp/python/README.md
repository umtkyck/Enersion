# Enersion Control System - Python

Modern GUI application for controlling Digital I/O on MYIR STM32MP257.

## Features

- 🎨 Modern industrial UI design
- 📊 Real-time monitoring of 64 Digital Inputs
- 🎛️ Control of 64 Digital Outputs
- 🔌 RS485 communication with GPIO direction control
- 🔄 Automatic polling and state synchronization

## Requirements

- Python 3.7+
- pyserial
- tkinter (included with Python)

## Installation

```bash
# Clone/copy to MYIR board
scp -r python/ root@192.168.0.10:/home/root/enersion/

# SSH to board
ssh root@192.168.0.10

# Install dependencies
cd /home/root/enersion/python
pip3 install -r requirements.txt
```

## Usage

### On MYIR Board (Linux)

```bash
cd /home/root/enersion/python
chmod +x run.sh
./run.sh
```

### On Windows (for testing)

```batch
cd LinuxApp\python
run.bat
```

## Project Structure

```
python/
├── app.py              # Main application
├── requirements.txt    # Python dependencies
├── run.sh             # Linux run script
├── run.bat            # Windows run script
├── hal/
│   ├── __init__.py
│   └── rs485.py       # RS485 HAL with GPIO control
├── protocol/
│   ├── __init__.py
│   └── enersion.py    # Enersion protocol implementation
└── gui/
    ├── __init__.py
    ├── theme.py       # UI theme and colors
    └── widgets.py     # Custom widgets
```

## Hardware Configuration

| Parameter | Value |
|-----------|-------|
| RS485 Port | /dev/ttySTM9 |
| Baud Rate | 115200 |
| GPIO Direction | PI10 (138) |
| DI Controller | Address 0x02 |
| DO Controller | Address 0x03 |

## Keyboard Shortcuts

- `Ctrl+1` - Dashboard
- `Ctrl+2` - Digital Inputs
- `Ctrl+3` - Digital Outputs
- `Ctrl+4` - Settings
- `Ctrl+Q` - Quit

## License

© 2024 Enersion. All rights reserved.

