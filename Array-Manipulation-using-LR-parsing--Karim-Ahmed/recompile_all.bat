@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   Recompiling All Compiler Components
echo ========================================
echo.

REM ── Locate g++ ────────────────────────────────────────────────────────────
set GPP=
for %%P in (
    "C:\msys64\ucrt64\bin\g++.exe"
    "C:\msys64\mingw64\bin\g++.exe"
    "C:\MinGW\bin\g++.exe"
    "C:\TDM-GCC-64\bin\g++.exe"
) do (
    if exist %%P (
        if "!GPP!"=="" set GPP=%%~P
    )
)

REM Fall back to PATH if none of the above found
if "!GPP!"=="" (
    where g++ >nul 2>&1
    if !errorlevel! equ 0 (
        set GPP=g++
    ) else (
        echo ERROR: g++ not found. Install MinGW/MSYS2 and try again.
        pause
        exit /b 1
    )
)

echo Using compiler: !GPP!
echo.

REM ── Root of the project (same folder as this bat) ─────────────────────────
set ROOT=%~dp0
REM Remove trailing backslash
if "!ROOT:~-1!"=="\" set ROOT=!ROOT:~0,-1!

echo Project root: !ROOT!
echo.

REM ── [1/4] Main (Lexer + Parser) ───────────────────────────────────────────
echo [1/4] Compiling Main (Lexer + Parser)...
"!GPP!" -std=c++17 -o "!ROOT!\Main.exe" "!ROOT!\Main.cpp"
if !errorlevel! neq 0 (
    echo ERROR: Main compilation failed!
    pause
    exit /b 1
)
echo   Main.exe compiled successfully!
echo.

REM ── [2/4] Semantic Analyzer ───────────────────────────────────────────────
echo [2/4] Compiling Semantic Analyzer...
"!GPP!" -std=c++17 ^
    -o "!ROOT!\semantic\semantic_main.exe" ^
    "!ROOT!\semantic\semantic_main.cpp" ^
    "!ROOT!\semantic\ast_builder.cpp" ^
    "!ROOT!\semantic\semantic_analyzer.cpp" ^
    "!ROOT!\semantic\semantic_output.cpp" ^
    "!ROOT!\semantic\symbol_table.cpp"
if !errorlevel! neq 0 (
    echo ERROR: Semantic analyzer compilation failed!
    pause
    exit /b 1
)
echo   semantic_main.exe compiled successfully!
echo.

REM ── [3/4] Code Generator ─────────────────────────────────────────────────
echo [3/4] Compiling Code Generator...
"!GPP!" -std=c++17 ^
    -o "!ROOT!\codegen\codegen.exe" ^
    "!ROOT!\codegen\codegen_main.cpp" ^
    "!ROOT!\codegen\code_generator.cpp" ^
    "!ROOT!\codegen\ast_loader.cpp" ^
    "!ROOT!\codegen\symbol_table_loader.cpp"
if !errorlevel! neq 0 (
    echo ERROR: Code generator compilation failed!
    pause
    exit /b 1
)
echo   codegen.exe compiled successfully!
echo.

REM ── [4/4] Optimizer ──────────────────────────────────────────────────────
echo [4/4] Compiling Optimizer...
"!GPP!" -std=c++17 ^
    -o "!ROOT!\optimizer\optimizer.exe" ^
    "!ROOT!\optimizer\optimizer_main.cpp" ^
    "!ROOT!\optimizer\optimizer.cpp" ^
    "!ROOT!\optimizer\ir_loader.cpp"
if !errorlevel! neq 0 (
    echo ERROR: Optimizer compilation failed!
    pause
    exit /b 1
)
echo   optimizer.exe compiled successfully!
echo.

echo ========================================
echo   All components compiled successfully!
echo ========================================
echo.
echo   Main.exe
echo   semantic\semantic_main.exe
echo   codegen\codegen.exe
echo   optimizer\optimizer.exe
echo.
pause
