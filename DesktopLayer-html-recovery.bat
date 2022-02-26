@echo off
pushd %~dp0
cscript //Nologo "%~dpn0.vbs" %*
pause
