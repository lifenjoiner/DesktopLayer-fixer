@echo off
setlocal ENABLEDELAYEDEXPANSION
pushd %~dp0

if "%~1"=="" goto :END
set Cleaner=DesktopLayer-pe-recovery.exe
for /f "usebackq delims=" %%i in (%1) do (
    %Cleaner% "%%i"
)

:END
popd
exit /b
