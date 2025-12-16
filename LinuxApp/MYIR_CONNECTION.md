# MYIR STM32MP257 Board Connection

## Connection Details

| Parameter | Value |
|-----------|-------|
| **IP Address** | 192.168.0.10 |
| **Username** | root |
| **Password** | 123 |
| **SSH Port** | 22 (default) |

## Quick Connect

### Windows (PowerShell/CMD)
```powershell
ssh root@192.168.0.10
# Password: 123
```

### Linux/Mac
```bash
ssh root@192.168.0.10
# Password: 123
```

## Check Connection

### Windows
```powershell
# Test ping
ping 192.168.0.10

# Test SSH (will prompt for password)
ssh root@192.168.0.10 "echo Connection OK && uname -a"
```

### Linux/Mac
```bash
# Test ping
ping -c 3 192.168.0.10

# Test SSH with password
sshpass -p "123" ssh root@192.168.0.10 "echo Connection OK && uname -a"

# Or use the deploy script
./scripts/deploy.sh --check
```

## Deploy Application

```bash
# Build for STM32MP257 (requires SDK)
source /opt/st/stm32mp2/environment-setup-cortexa35-ostl-linux
./scripts/build.sh --target stm32mp257

# Deploy to board
./scripts/deploy.sh --run
```

## Manual Deployment

```bash
# Copy binary
scp build-arm/enersion_gui root@192.168.0.10:/usr/bin/

# Run on target
ssh root@192.168.0.10
export DISPLAY=:0
/usr/bin/enersion_gui
```

## Troubleshooting

### Host Unreachable
1. Check MYIR board is powered on
2. Verify Ethernet cable connection
3. Ensure PC and board are on same network (192.168.0.x)
4. Check PC firewall settings

### SSH Connection Refused
1. Verify SSH service is running on board
2. Check if port 22 is open
3. Verify credentials are correct

### Display Issues
1. Ensure HDMI is connected before boot
2. Set `DISPLAY=:0` environment variable
3. Check if Weston/Wayland is running

## Board Information

- **Board**: MYIR MYD-YM62X (STM32MP257)
- **Processor**: ARM Cortex-A35
- **Display**: HDMI Touchscreen
- **OS**: OpenSTLinux / Yocto

