@echo off
setlocal EnableDelayedExpansion
cd %~dp0

rmdir /Q /S .bin 2>&1 1>NUL
rmdir /Q /S .obj 2>&1 1>NUL

echo Done.
