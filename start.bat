@echo off
echo ========================================
echo STARTING COMPILER GUI
echo ========================================
echo.

cd GUI

REM Check if node_modules exists
if not exist "node_modules" (
    echo node_modules not found. Running npm install...
    npm install
    echo.
)

echo Starting backend server...
start "Compiler Backend" cmd /k "node server.js"
echo.
echo Starting frontend...
timeout /t 2 /nobreak >nul
npm run dev
