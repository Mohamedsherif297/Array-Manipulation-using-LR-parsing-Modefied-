@echo off
cd /d "%~dp0"
echo Compiling Main.exe (standalone)...
g++ -std=c++17 -o Main.exe Main.cpp
if errorlevel 1 (
    echo Compilation FAILED
    exit /b 1
)
echo Main.exe compiled successfully