@echo off
echo ========================================
echo Recompiling All Compiler Components
echo ========================================

echo.
echo [1/3] Compiling Main (Lexer + Parser)...
cd "Array-Manipulation-using-LR-parsing--Karim-Ahmed"
g++ -o Main.exe Main.cpp -std=c++17
if %errorlevel% neq 0 (
    echo ERROR: Main compilation failed!
    pause
    exit /b 1
)
echo Main.exe compiled successfully!

echo.
echo [2/3] Compiling Semantic Analyzer...
cd semantic
g++ -o semantic_main.exe semantic_main.cpp ast_builder.cpp semantic_analyzer.cpp semantic_output.cpp symbol_table.cpp -std=c++17
if %errorlevel% neq 0 (
    echo ERROR: Semantic analyzer compilation failed!
    pause
    exit /b 1
)
echo semantic_main.exe compiled successfully!
cd ..

echo.
echo [3/3] Compiling Code Generator...
cd codegen
g++ -o codegen.exe codegen_main.cpp code_generator.cpp ast_loader.cpp symbol_table_loader.cpp -std=c++17
if %errorlevel% neq 0 (
    echo ERROR: Code generator compilation failed!
    pause
    exit /b 1
)
echo codegen.exe compiled successfully!
cd ..\..

echo.
echo ========================================
echo All components compiled successfully!
echo ========================================
pause
