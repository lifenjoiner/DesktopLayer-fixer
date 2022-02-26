@echo off
setlocal ENABLEDELAYEDEXPANSION
pushd %~dp0

if "%~2"=="" goto :END
set Cleaner=cscript.exe //Nologo "%~dp0\DesktopLayer-html-recovery.vbs"
for /f "usebackq delims=" %%i in (%1) do (
    %Cleaner% "%%i" %2
)

:END
popd
exit /b
