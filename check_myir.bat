@echo off
echo ========================================
echo  MYIR STM32MP257 Connection Check
echo ========================================
echo.
echo Target: root@192.168.0.10
echo Password: 123
echo.

echo [1/2] Testing PING...
ping -n 2 192.168.0.10
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Board not reachable!
    echo Check: Power, Network cable, IP address
    pause
    exit /b 1
)

echo.
echo [2/2] Testing SSH connection...
echo Enter password when prompted: 123
echo.
ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no root@192.168.0.10 "echo; echo '=== CONNECTION SUCCESS ==='; echo; uname -a; echo; cat /etc/os-release | head -5; echo; df -h | head -3; echo; echo '=== SERIAL PORTS ==='; ls /dev/ttySTM* 2>/dev/null || echo 'No ttySTM ports'; ls /dev/ttyUSB* 2>/dev/null || echo 'No ttyUSB ports'"

echo.
echo ========================================
pause
