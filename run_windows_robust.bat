@echo off
setlocal EnableDelayedExpansion
title Fireboy ^& Watergirl Build ^& Run (Robust)

rem Always run relative to this script's folder
pushd "%~dp0"

echo =======================================================
echo  [*] Fireboy ^& Watergirl Launcher (Robust)
echo =======================================================
echo.

rem Ensure build/bin dirs exist
if not exist "build\obj" mkdir "build\obj"
if not exist "build\moc" mkdir "build\moc"
if not exist "build\rcc" mkdir "build\rcc"
if not exist "build\ui"  mkdir "build\ui"
if not exist "bin"       mkdir "bin"

goto :Main

rem ---- Subroutine: add Qt bin to PATH if qmake exists ----
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
rem 1) Find qmake
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
  echo [?] qmake not found in PATH. Searching...

  rem Environment variables
  if defined QTDIR    call :TryAddQtBin "%QTDIR%"
  if defined QT_DIR   call :TryAddQtBin "%QT_DIR%"
  if defined Qt6_DIR  call :TryAddQtBin "%Qt6_DIR%"
  if defined Qt5_DIR  call :TryAddQtBin "%Qt5_DIR%"

  rem Qt Online Installer defaults
  for /D %%D in (C:\Qt\5.* C:\Qt\6.*) do (
    for /D %%M in ("%%D\mingw*" "%%D\msvc*") do (
      if exist "%%M\bin\qmake.exe" call :TryAddQtBin "%%M"
    )
  )

  rem Extra depth: C:\Qt\<major>.<minor>.<patch>\<kit>\bin
  for /D %%D in (C:\Qt\5.* C:\Qt\6.*) do (
    for /D %%V in ("%%D\*") do (
      for /D %%M in ("%%V\mingw*" "%%V\msvc*") do (
        if exist "%%M\bin\qmake.exe" call :TryAddQtBin "%%M"
      )
    )
  )

  rem MSYS2 common paths
  if exist "C:\msys64\mingw64\bin\qmake.exe" call :TryAddQtBin "C:\msys64\mingw64"
  if exist "C:\msys64\mingw32\bin\qmake.exe" call :TryAddQtBin "C:\msys64\mingw32"

  where qmake >nul 2>nul
  if %ERRORLEVEL% NEQ 0 (
    echo [X] Still couldn't find qmake.exe.
    echo [>] Set QTDIR to your Qt kit folder, e.g. C:\Qt\6.6.3\mingw_64
    goto :RunGame
  )
)

rem 2) qmake
echo [*] Running qmake...
qmake FireboyWatergirl.pro
if %ERRORLEVEL% NEQ 0 (
  echo [X] qmake failed. Trying to run last build if present.
  goto :RunGame
)

rem 3) pick make tool
set "MAKE_CMD="
where mingw32-make >nul 2>nul && set "MAKE_CMD=mingw32-make -j%NUMBER_OF_PROCESSORS%"
if "%MAKE_CMD%"=="" (where jom >nul 2>nul && set "MAKE_CMD=jom")
if "%MAKE_CMD%"=="" (where nmake >nul 2>nul && set "MAKE_CMD=nmake")

if "%MAKE_CMD%"=="" (
  echo [X] No make tool found (mingw32-make/jom/nmake).
  goto :RunGame
)

rem 4) build
echo [*] Building with: %MAKE_CMD%
%MAKE_CMD%
if %ERRORLEVEL% NEQ 0 (
  echo [X] Build failed. Trying to run last build if present.
  goto :RunGame
)

echo.
echo [+] Build succeeded.
echo.

:RunGame
set "EXE="
if exist "bin\FireboyWatergirl.exe"     set "EXE=bin\FireboyWatergirl.exe"
if "%EXE%"=="" if exist "release\FireboyWatergirl.exe" set "EXE=release\FireboyWatergirl.exe"
if "%EXE%"=="" if exist "debug\FireboyWatergirl.exe"   set "EXE=debug\FireboyWatergirl.exe"

if not "%EXE%"=="" (
  echo [>] Starting: %EXE%
  start "" "%EXE%"
  goto :End
)

echo [X] Couldn't find FireboyWatergirl.exe (bin/release/debug).
pause

:End
popd
endlocal
exit /b 0

