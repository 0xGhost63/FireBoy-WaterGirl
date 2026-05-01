@echo off
setlocal EnableDelayedExpansion
title Fireboy ^& Watergirl Build ^& Run

echo =======================================================
echo  [*] Welcome to the Fireboy ^& Watergirl Launcher! :)
echo =======================================================
echo  Let's get everything ready for you...
echo.

:: 1. Ensure build and bin directories exist
if not exist "build\obj" mkdir "build\obj"
if not exist "build\moc" mkdir "build\moc"
if not exist "build\rcc" mkdir "build\rcc"
if not exist "build\ui" mkdir "build\ui"
if not exist "bin" mkdir "bin"

:: 2. Try to find qmake if it's not in PATH
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [?] Hmm, I couldn't find Qt in your PATH right away. Let me look around for you...
    for /D %%D in (C:\Qt\5.* C:\Qt\6.*) do (
        for /D %%M in (%%D\mingw* %%D\msvc*) do (
            if exist "%%M\bin\qmake.exe" (
                set "PATH=%%M\bin;!PATH!"
                echo [!] Found it! Qt is hiding at: %%M\bin
                goto :FoundQt
            )
        )
    )
    echo [:(] Oh no! I couldn't find Qt automatically. Please make sure it's installed.
    echo [>] But don't worry! I'll try to run the game anyway if it's already built!
    goto :RunGame
)
:FoundQt

:: 3. Generate Makefile using qmake
echo [*] Preparing the project files (running qmake)...
qmake FireboyWatergirl.pro
if %ERRORLEVEL% NEQ 0 (
    echo [!] Oops, preparing the project failed. 
    echo [>] Skipping straight to launching the game!
    goto :RunGame
)

:: 4. Detect the right Make tool (mingw32-make, jom, or nmake)
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
    echo [!] I couldn't find a compiler tool (mingw32-make, jom, or nmake).
    echo [>] Skipping straight to launching the game!
    goto :RunGame
)

:: 5. Compile the project
echo [*] Compiling your awesome code... (This might take a second!)
%MAKE_CMD%
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [!] Looks like there are some code errors. The build failed!
    echo [>] No problem! I'll just run the last working version of your game...
    echo.
    goto :RunGame
)

echo.
echo =======================================================
echo  [+] Build Successful! You're good to go! ^_^
echo =======================================================
echo.

:RunGame
:: 6. Run the Game
echo [>] Launching Fireboy ^& Watergirl... Have fun Noob ! :D
if exist "bin\FireboyWatergirl.exe" (
    start "" "bin\FireboyWatergirl.exe"
) else (
    if exist "release\FireboyWatergirl.exe" (
        start "" "release\FireboyWatergirl.exe"
    ) else (
        if exist "debug\FireboyWatergirl.exe" (
            start "" "debug\FireboyWatergirl.exe"
        ) else (
            echo.
            echo =======================================================
            echo  [X]I couldn't find the compiled game executable anywhere.
            echo  Please open Qt Creator and build the project manually just once!
            echo =======================================================
            pause
        )
    )
)

:End
