@echo off
cd /d "%~dp0"
echo Compiling Main.exe...
g++ -std=c++17 -o Main.exe Main.cpp parser/Parser_States.cpp parser/Parsing_Table.cpp parser/Item.cpp
if errorlevel 1 (
    echo Compilation FAILED
    exit /b 1
)
echo Main.exe compiled successfully
