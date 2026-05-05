# flash.ps1 — Build, flash (OpenOCD), and open serial

$ErrorActionPreference = "Stop"

# --- Config ---

$openocd = "C:\Users\tuvab\openocd\openocd.exe"
$scripts  = "C:\Users\tuvab\openocd\scripts"
$putty    = "C:\Program Files\PuTTY\putty.exe"
$comPort  = "COM10"
$board    = "rpi_pico2/rp2350a/m33"


$probe = Get-PnpDevice | Where-Object {$_.FriendlyName -like "*CMSIS-DAP*"}
if (-not $probe) {
    Write-Host "Error: Debug probe not detected. Flash debugprobe.uf2 and reconnect."
    exit 1
}

# --- Step 1: Build ---

Write-Host "Building Zephyr project..."
west build -b $board

# --- Step 2: Verify ELF exists ---

$elf = (Resolve-Path "build\zephyr\zephyr.elf").Path -replace '\\','/'
if (!(Test-Path $elf)) {
Write-Host "Error: ELF not found at $elf"
exit 1
}

# --- Step 3: Flash using OpenOCD ---

Write-Host "Flashing via OpenOCD..."
& $openocd `
-s $scripts `
-f interface/cmsis-dap.cfg `
-f target/rp2350.cfg `
-c "adapter speed 5000" `
-c "program $elf verify reset exit"


if ($LASTEXITCODE -ne 0) {
Write-Host "Flashing failed"
exit 1
}

# --- Step 4: Wait for USB CDC ---

Write-Host "Waiting for serial device..."
Start-Sleep -Seconds 3

# --- Step 5: Open PuTTY ---

Write-Host "Opening PuTTY on $comPort..."
Start-Process $putty -ArgumentList "-serial $comPort -sercfg 115200,8,n,1,N"
