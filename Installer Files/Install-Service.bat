echo off
cls
echo %~dp0
"%~dp0\munin-node.exe" --install
pause