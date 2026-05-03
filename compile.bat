@echo off
echo ========================================
echo COMPILING COMPILER
echo ========================================
echo.

cd Array-Manipulation-using-LR-parsing--Karim-Ahmed

echo Compiling Main.exe...
g++ -std=c++17 Main.cpp -o Main.exe
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Main.exe compilation failed!
    cd ..
    pause
    exit /b 1
)
echo Main.exe compiled successfully!
echo.

echo Compiling semantic analyzer...
cd semantic
g++ -std=c++17 semantic_main.cpp ast_builder.cpp semantic_analyzer.cpp semantic_output.cpp symbol_table.cpp -o semantic_main.exe
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Semantic analyzer compilation failed!
    cd ..\..
    pause
    exit /b 1
)
cd ..
echo Semantic analyzer compiled successfully!
echo.

echo Compiling code generator...
cd codegen
g++ -std=c++17 codegen_main.cpp ast_loader.cpp symbol_table_loader.cpp code_generator.cpp -o codegen.exe
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Code generator compilation failed!
    cd ..\..
    pause
    exit /b 1
)
cd ..
echo Code generator compiled successfully!
echo.

echo Compiling optimizer...
cd optimizer
g++ -std=c++17 optimizer_main.cpp ir_loader.cpp optimizer.cpp -o optimizer.exe
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Optimizer compilation failed!
    cd ..\..
    pause
    exit /b 1
)
cd ..
echo Optimizer compiled successfully!
echo.

echo ========================================
echo COMPILATION COMPLETE!
echo ========================================
cd ..
pause
