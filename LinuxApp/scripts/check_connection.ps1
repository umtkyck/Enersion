# Check connection to MYIR STM32MP257 board
$TargetIP = "192.168.0.10"
$TargetUser = "root"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " MYIR STM32MP257 Connection Check" -ForegroundColor Cyan  
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Target: $TargetUser@$TargetIP"
Write-Host ""

# Test ping
Write-Host "1. Testing network connectivity (ping)..." -ForegroundColor Yellow
$ping = Test-Connection -ComputerName $TargetIP -Count 2 -Quiet
if ($ping) {
    Write-Host "   [OK] Host is reachable" -ForegroundColor Green
} else {
    Write-Host "   [FAIL] Host is NOT reachable" -ForegroundColor Red
    Write-Host ""
    Write-Host "Troubleshooting:" -ForegroundColor Yellow
    Write-Host "  - Check if MYIR board is powered on"
    Write-Host "  - Verify network cable connection"
    Write-Host "  - Ensure board and PC are on same network (192.168.0.x)"
    Write-Host "  - Check if firewall is blocking"
    exit 1
}

# Test SSH port
Write-Host ""
Write-Host "2. Testing SSH port (22)..." -ForegroundColor Yellow
try {
    $tcp = New-Object System.Net.Sockets.TcpClient
    $tcp.Connect($TargetIP, 22)
    if ($tcp.Connected) {
        Write-Host "   [OK] SSH port is open" -ForegroundColor Green
        $tcp.Close()
    }
} catch {
    Write-Host "   [FAIL] SSH port is NOT accessible" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host " Connection test PASSED!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "To connect manually:" -ForegroundColor Cyan
Write-Host "  ssh root@192.168.0.10"
Write-Host "  Password: 123"
Write-Host ""
Write-Host "To deploy application:" -ForegroundColor Cyan
Write-Host "  scp LinuxApp/build-arm/enersion_gui root@192.168.0.10:/usr/bin/"

