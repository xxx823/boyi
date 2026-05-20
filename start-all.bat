@echo off
chcp 65001 >nul
cls
echo.
echo ========================================
echo   International Checkers AI System
echo   Starting Up...
echo ========================================
echo.

REM Check Node.js
where node >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Node.js not found!
    echo.
    echo Please install Node.js from: https://nodejs.org/
    echo.
    pause
    exit /b 1
)

echo [1/4] Node.js detected
node --version
echo.

REM Check dependencies
if not exist "server\node_modules" (
    echo [2/4] Installing dependencies...
    echo.
    cd server
    call npm install
    if %ERRORLEVEL% NEQ 0 (
        echo.
        echo [ERROR] Failed to install dependencies!
        cd ..
        pause
        exit /b 1
    )
    cd ..
    echo.
) else (
    echo [2/4] Dependencies already installed
)

echo [3/4] Starting backend server...
echo.

REM Start server in new window
start "Checkers AI Server" cmd /k "cd /d %~dp0server && node server.js"

REM Wait for server to start
timeout /t 3 /nobreak >nul

echo [4/4] Opening browser...
echo.

REM Open browser
start http://localhost:3000

echo.
echo ========================================
echo   System Started Successfully!
echo ========================================
echo.
echo   Web Interface: http://localhost:3000
echo   Server Window: Keep it open
echo.
echo To stop the system:
echo   1. Close the browser
echo   2. Press Ctrl+C in server window
echo.
echo ========================================
echo.

pause
