@echo off
rem ================================================================
rem  Fireboy ^& Watergirl  --  Build ^& Run
rem  Usage:
rem    run.bat          -- build then launch
rem    run.bat clean    -- wipe cache, rebuild, then launch
rem ================================================================
pushd "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0run.ps1" %*
popd
