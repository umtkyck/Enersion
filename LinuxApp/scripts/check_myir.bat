@echo off
echo =============================================
echo  MYIR STM32MP257 Connection Test
echo =============================================
echo.
echo Target: root@192.168.0.10
echo.

echo 1. Testing PING...
ping -n 2 192.168.0.10
echo.

echo 2. Testing SSH connection...
echo    (Password is: 123)
echo.
ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no root@192.168.0.10 "echo CONNECTION_SUCCESS && uname -a && cat /etc/os-release | head -3"

echo.
echo =============================================
echo  Test Complete
echo =============================================
pause



