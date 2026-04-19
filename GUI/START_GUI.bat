@echo off
echo ========================================
echo   Array Manipulation Compiler - GUI
echo ========================================
echo.
echo Starting servers...
echo.
echo Backend will run on: http://localhost:3001
echo Frontend will run on: http://localhost:3000
echo.
echo Press Ctrl+C to stop the servers
echo.
start "Backend Server" cmd /k "node server.js"
timeout /t 2 /nobreak >nul
start "Frontend Server" cmd /k "npx vite"
timeout /t 3 /nobreak >nul
start http://localhost:3000
echo.
echo Both servers are starting in separate windows...
echo Close those windows to stop the servers.
echo.
pause
