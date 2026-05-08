# =================================================================
#  Fireboy & Watergirl  --  Build & Run  (PowerShell)
#  Usage:
#    .\run.ps1          -- build then launch
#    .\run.ps1 clean    -- wipe cache, rebuild, then launch
# =================================================================

$ErrorActionPreference = "Continue"
Set-Location $PSScriptRoot

Write-Host ""
Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host "  [*] Fireboy and Watergirl Launcher" -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host ""

# -- 0) Optional clean --------------------------------------------
if ($args[0] -ieq "clean") {
    Write-Host "[*] Clean requested -- wiping build cache..." -ForegroundColor Yellow
    if (Test-Path "build") { Remove-Item "build" -Recurse -Force }
    if (Test-Path "bin")   { Remove-Item "bin"   -Recurse -Force }
    Write-Host "[*] Cache cleared." -ForegroundColor Green
    Write-Host ""
}

# -- 1) Add Qt + MinGW to PATH ------------------------------------
$qtBin    = "C:\Qt\6.11.0\mingw_64\bin"
$mingwBin = "C:\Qt\Tools\mingw1310_64\bin"

if (Test-Path $qtBin)    { $env:PATH = $qtBin    + ";" + $env:PATH }
if (Test-Path $mingwBin) { $env:PATH = $mingwBin + ";" + $env:PATH }

# Fallback: scan C:\Qt for any qmake.exe
if (-not (Get-Command qmake -ErrorAction SilentlyContinue)) {
    Write-Host "[?] qmake not in PATH -- scanning C:\Qt..." -ForegroundColor Yellow
    $found = Get-ChildItem "C:\Qt" -Recurse -Filter "qmake.exe" -ErrorAction SilentlyContinue |
             Select-Object -First 1
    if ($found) {
        $env:PATH = $found.DirectoryName + ";" + $env:PATH
        Write-Host "[*] Found: $($found.FullName)" -ForegroundColor Green
    } else {
        Write-Host "[X] Could not find qmake. Install Qt and try again." -ForegroundColor Red
        Read-Host "Press Enter to exit"
        exit 1
    }
}

# -- 2) Ensure build directories exist ----------------------------
@("build\obj","build\moc","build\rcc","build\ui","bin") | ForEach-Object {
    if (-not (Test-Path $_)) { New-Item -ItemType Directory -Path $_ | Out-Null }
}

# -- 3) Sync styles -----------------------------------------------
if (Test-Path "styles.qss") {
    Write-Host "[*] Syncing styles..." -ForegroundColor DarkCyan
    Copy-Item "styles.qss" "assets\styles\game.qss" -Force -ErrorAction SilentlyContinue
}

# -- 4) qmake -----------------------------------------------------
Write-Host "[*] Running qmake..." -ForegroundColor DarkCyan
& qmake FireboyWatergirl.pro
if ($LASTEXITCODE -ne 0) {
    Write-Host "[X] qmake failed." -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# -- 5) Build -----------------------------------------------------
$make = @("mingw32-make","jom","nmake") |
        Where-Object { Get-Command $_ -ErrorAction SilentlyContinue } |
        Select-Object -First 1

if (-not $make) {
    Write-Host "[X] No build tool found (mingw32-make / jom / nmake)." -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

$cpus = if ($env:NUMBER_OF_PROCESSORS) { $env:NUMBER_OF_PROCESSORS } else { "4" }
Write-Host "[*] Compiling with $make -j$cpus ..." -ForegroundColor DarkCyan
& $make "-j$cpus"

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[X] Build failed. Check errors above." -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""
Write-Host "=======================================================" -ForegroundColor Green
Write-Host "  [+] Build successful!" -ForegroundColor Green
Write-Host "=======================================================" -ForegroundColor Green
Write-Host ""

# -- 6) Find the exe ----------------------------------------------
$exe = @("bin\FireboyWatergirl.exe","release\FireboyWatergirl.exe","debug\FireboyWatergirl.exe") |
       Where-Object { Test-Path $_ } |
       Select-Object -First 1

if (-not $exe) {
    Write-Host "[X] Executable not found after build." -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# -- 7) Deploy Qt DLLs --------------------------------------------
if (Get-Command windeployqt -ErrorAction SilentlyContinue) {
    Write-Host "[*] Deploying Qt DLLs..." -ForegroundColor DarkCyan
    & windeployqt $exe 2>&1 | Out-Null
    Write-Host "[*] DLLs deployed." -ForegroundColor Green
} else {
    Write-Host "[!] windeployqt not found -- game may crash if DLLs are missing." -ForegroundColor Yellow
}

# -- 8) Launch ----------------------------------------------------
Write-Host "[>] Launching..." -ForegroundColor Cyan
& ".\$exe"

Write-Host ""
Write-Host "[*] Done. Script finished." -ForegroundColor Gray
