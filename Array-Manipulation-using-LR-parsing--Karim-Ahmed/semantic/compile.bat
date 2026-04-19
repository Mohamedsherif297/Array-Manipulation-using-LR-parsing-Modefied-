@echo off
cd /d "%~dp0"
g++ -std=c++17 -o semantic_main.exe semantic_main.cpp semantic_analyzer.cpp ast_builder.cpp symbol_table.cpp semantic_output.cpp
echo Compilation complete
pause
