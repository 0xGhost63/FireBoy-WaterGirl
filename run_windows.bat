@echo off
setlocal EnableDelayedExpansion
title Fireboy ^& Watergirl Build ^& Run

rem Explicitly add Qt and MinGW to PATH for instant compilation
set "PATH=C:\Qt\6.11.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%"

rem Always run relative to this script's folder
pushd "%~dp0"

echo =======================================================
echo  [*] Welcome to the Fireboy ^& Watergirl Launcher! :)
echo =======================================================
echo  Let's get everything ready for you...
echo.

if /I "%~1"=="clean" (
    echo [*] Clean requested! Wiping build cache...
    if exist "build" rmdir /S /Q "build"
    if exist "bin" rmdir /S /Q "bin"
    echo [*] Cache cleared.
    echo.
)

rem 1) Ensure build and bin directories exist
if not exist "build\obj" mkdir "build\obj"
if not exist "build\moc" mkdir "build\moc"
if not exist "build\rcc" mkdir "build\rcc"
if not exist "build\ui"  mkdir "build\ui"
if not exist "bin"       mkdir "bin"

rem 1b) Sync root styles to assets folder
if exist "styles.qss" (
    echo [*] Syncing styles...
    copy /Y styles.qss "assets\styles\game.qss" >nul
    copy /b resources.qrc +,, >nul
)

goto :Main

rem -------------------------------------------------------
rem Helper: given a Qt root/kit folder, add its bin to PATH
rem -------------------------------------------------------
:TryAddQtBin
set "CAND=%~1"
if "%CAND%"=="" exit /b 0
if exist "%CAND%\bin\qmake.exe" (
    set "PATH=%CAND%\bin;!PATH!"
    exit /b 0
)
if exist "%CAND%\qmake.exe" (
    set "PATH=%CAND%;!PATH!"
    exit /b 0
)
exit /b 0

:Main
rem -------------------------------------------------------
rem 2) Find qmake (Qt) from many locations
rem -------------------------------------------------------
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [?] Couldn't find Qt [qmake] in PATH. Searching common locations...

    rem 2a) Environment variables (many Qt setups export one of these)
    if defined QTDIR    call :TryAddQtBin "%QTDIR%"
    if defined QT_DIR   call :TryAddQtBin "%QT_DIR%"
    if defined Qt6_DIR  call :TryAddQtBin "%Qt6_DIR%"
    if defined Qt5_DIR  call :TryAddQtBin "%Qt5_DIR%"

    rem 2b) Online installer defaults
    for /D %%D in (C:\Qt\5.* C:\Qt\6.*) do (
        for /D %%M in ("%%D\mingw*" "%%D\msvc*") do (
            if exist "%%M\bin\qmake.exe" call :TryAddQtBin "%%M"
        )
    )

    rem 2c) Extra depth: C:\Qt\<major>.<minor>.<patch>\<kit>\bin
    for /D %%D in (C:\Qt\5.* C:\Qt\6.*) do (
        for /D %%V in ("%%D\*") do (
            for /D %%M in ("%%V\mingw*" "%%V\msvc*") do (
                if exist "%%M\bin\qmake.exe" call :TryAddQtBin "%%M"
            )
        )
    )

    rem 2d) MSYS2 common paths
    if exist "C:\msys64\mingw64\bin\qmake.exe" call :TryAddQtBin "C:\msys64\mingw64"
    if exist "C:\msys64\mingw32\bin\qmake.exe" call :TryAddQtBin "C:\msys64\mingw32"

    rem Re-check after searching
    where qmake >nul 2>nul
    if %ERRORLEVEL% NEQ 0 (
        echo [X] Still couldn't find qmake.exe.
        echo [>] Fix: set QTDIR to your Qt kit folder, e.g. C:\Qt\6.6.3\mingw_64
        echo [>] I'll try to run the game anyway if it's already built.
        goto :RunGame
    )
)

rem -------------------------------------------------------
rem 3) Generate Makefile using qmake
rem -------------------------------------------------------
echo [*] Preparing the project files (running qmake)...
qmake FireboyWatergirl.pro
if %ERRORLEVEL% NEQ 0 (
    echo [!] qmake failed. Skipping straight to launching the game if available.
    goto :RunGame
)

rem -------------------------------------------------------
rem 4) Detect the right Make tool (mingw32-make, jom, or nmake)
rem -------------------------------------------------------
set "MAKE_CMD="
where mingw32-make >nul 2>nul
if %ERRORLEVEL% EQU 0 set "MAKE_CMD=mingw32-make -j%NUMBER_OF_PROCESSORS%"

if "%MAKE_CMD%"=="" (
    where jom >nul 2>nul
    if %ERRORLEVEL% EQU 0 set "MAKE_CMD=jom"
)

if "%MAKE_CMD%"=="" (
    where nmake >nul 2>nul
    if %ERRORLEVEL% EQU 0 set "MAKE_CMD=nmake"
)

if "%MAKE_CMD%"=="" (
    echo [X] Couldn't find a build tool [mingw32-make, jom, or nmake].
    echo [>] Skipping straight to launching the game if available.
    goto :RunGame
)

rem -------------------------------------------------------
rem 5) Build
rem -------------------------------------------------------
echo [*] Compiling... (this might take a second)
%MAKE_CMD%
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [X] Build failed. Trying to run last built executable if present...
    echo.
    goto :RunGame
)

echo.
echo =======================================================
echo  [+] Build Successful! You're good to go! ^_^
echo =======================================================
echo.

rem -------------------------------------------------------
rem 6) Run
rem -------------------------------------------------------
:RunGame
echo [>] Launching Fireboy ^& Watergirl...

set "EXE="
if exist "bin\FireboyWatergirl.exe"     set "EXE=bin\FireboyWatergirl.exe"
if "%EXE%"=="" if exist "release\FireboyWatergirl.exe" set "EXE=release\FireboyWatergirl.exe"
if "%EXE%"=="" if exist "debug\FireboyWatergirl.exe"   set "EXE=debug\FireboyWatergirl.exe"

if not "%EXE%"=="" (
    rem -------------------------------------------------------
    rem Deploy Qt DLLs so the game doesn't crash on Windows
    rem windeployqt copies all required Qt .dll files next to the .exe
    rem -------------------------------------------------------
    where windeployqt >nul 2>&1
    if %errorlevel%==0 (
        echo [*] Running windeployqt to bundle Qt DLLs...
        windeployqt "%EXE%" >nul 2>&1
        echo [*] DLLs deployed successfully.
    ) else (
        echo [!] Warning: windeployqt not found in PATH.
        echo     If the game crashes, open a Qt MinGW terminal and run:
        echo     windeployqt %EXE%
    )
    echo [*] Game is running... Close the game window to return here.
    "%EXE%"
    goto :End
)


echo.
echo =======================================================
echo  [X] I couldn't find the compiled game executable.
echo  Make sure your Qt kit matches your compiler (MinGW vs MSVC),
echo  then build once in Qt Creator, or run this script again.
echo =======================================================
pause

:End
echo.
echo [*] Script finished.
pause
popd
endlocal
exit /b 0
