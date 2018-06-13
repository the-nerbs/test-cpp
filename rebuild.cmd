@echo off
setlocal EnableDelayedExpansion
cd %~dp0

echo Cleaning . . .
call clean.cmd

echo.
echo Building . . .
call build.cmd
