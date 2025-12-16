# Test SSH Connection to MYIR STM32MP257 Board
$targetIP = "192.168.0.10"
$targetUser = "root"
$targetPort = 22

Write-Host "==========================================="
Write-Host " MYIR STM32MP257 Connection Test"
Write-Host "==========================================="
Write-Host ""
Write-Host "Target: $targetUser@$targetIP"
Write-Host ""

# Test ping
Write-Host "1. Testing PING..."
$pingResult = Test-Connection -ComputerName $targetIP -Count 2 -Quiet
if ($pingResult) {
    Write-Host "   [OK] Host is reachable" -ForegroundColor Green
} else {
    Write-Host "   [FAIL] Host not reachable" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please check:"
    Write-Host "  - Board is powered on"
    Write-Host "  - Network cable connected"
    Write-Host "  - IP address is correct"
    exit 1
}

# Test SSH port
Write-Host ""
Write-Host "2. Testing SSH Port ($targetPort)..."
$tcpResult = Test-NetConnection -ComputerName $targetIP -Port $targetPort -WarningAction SilentlyContinue
if ($tcpResult.TcpTestSucceeded) {
    Write-Host "   [OK] SSH port is open" -ForegroundColor Green
} else {
    Write-Host "   [FAIL] SSH port not accessible" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "==========================================="
Write-Host " Connection Test PASSED" -ForegroundColor Green
Write-Host "==========================================="
Write-Host ""
Write-Host "To connect via SSH:"
Write-Host "  ssh root@192.168.0.10"
Write-Host "  Password: 123"
Write-Host ""
Write-Host "To deploy application:"
Write-Host "  scp LinuxApp/build-arm/enersion_gui root@192.168.0.10:/usr/bin/"



