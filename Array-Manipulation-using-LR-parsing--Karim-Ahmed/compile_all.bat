@echo off
echo Compiling all components...

echo.
echo === Compiling Semantic Analyzer ===
cd /d "%~dp0semantic"
g++ -std=c++17 -o semantic_main.exe semantic_main.cpp semantic_analyzer.cpp ast_builder.cpp symbol_table.cpp semantic_output.cpp
if errorlevel 1 (
    echo Semantic analyzer compilation FAILED
    exit /b 1
)
echo Semantic analyzer compiled successfully

echo.
echo === Compiling Code Generator ===
cd /d "%~dp0codegen"
g++ -std=c++17 -o codegen.exe codegen_main.cpp code_generator.cpp ast_loader.cpp symbol_table_loader.cpp
if errorlevel 1 (
    echo Code generator compilation FAILED
    exit /b 1
)
echo Code generator compiled successfully

echo.
echo === Compiling Optimizer ===
cd /d "%~dp0optimizer"
g++ -std=c++17 -o optimizer.exe optimizer_main.cpp optimizer.cpp ir_loader.cpp
if errorlevel 1 (
    echo Optimizer compilation FAILED
    exit /b 1
)
echo Optimizer compiled successfully

echo.
echo === Compiling Main Compiler ===
cd /d "%~dp0"
g++ -std=c++17 -o Main.exe Main.cpp parser/Parser_States.cpp parser/Parsing_Table.cpp parser/Item.cpp
if errorlevel 1 (
    echo Main compiler compilation FAILED
    exit /b 1
)
echo Main compiler compiled successfully

echo.
echo === All components compiled successfully ===
pause
