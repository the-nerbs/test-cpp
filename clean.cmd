@echo off
setlocal EnableDelayedExpansion
cd %~dp0

rmdir /Q /S .bin 1>NUL 2>&1
rmdir /Q /S .obj 1>NUL 2>&1

echo Done.
