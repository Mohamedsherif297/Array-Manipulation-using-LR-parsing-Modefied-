@echo off
echo ========================================
echo   Array Manipulation Compiler
echo ========================================
echo.
echo Starting the compiler system...
echo.

REM Check if Main.exe exists
if not exist "Array-Manipulation-using-LR-parsing--Karim-Ahmed\Main.exe" (
    echo Compiling the main compiler...
    cd Array-Manipulation-using-LR-parsing--Karim-Ahmed
    call compile_main_only.bat
    cd ..
    echo.
)

echo Starting servers...
echo.
echo Backend will run on: http://localhost:3003
echo Frontend will run on: http://localhost:3000
echo.
echo Press Ctrl+C to stop the servers
echo.

REM Start backend server in new window
start "Backend Server" cmd /k "cd GUI && node server.js"
timeout /t 3 /nobreak >nul

REM Start frontend server in new window  
start "Frontend Server" cmd /k "cd GUI && npm run dev"
timeout /t 3 /nobreak >nul

REM Open browser
start http://localhost:3000

echo.
echo Both servers are starting in separate windows...
echo Open http://localhost:3000 in your browser
echo.
pause