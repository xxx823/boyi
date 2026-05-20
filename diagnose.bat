@echo off
chcp 65001 >nul
echo ========================================
echo   System Diagnostic Tool
echo ========================================
echo.

echo [Step 1/6] Checking Node.js...
where node >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [OK] Node.js found
    node --version
    npm --version
) else (
    echo [FAIL] Node.js not found
    echo Please install from: https://nodejs.org/
)
echo.

echo [Step 2/6] Checking project structure...
if exist "server\server.js" (
    echo [OK] server\server.js exists
) else (
    echo [FAIL] server\server.js missing
)

if exist "web\index.html" (
    echo [OK] web\index.html exists
) else (
    echo [FAIL] web\index.html missing
)

if exist "x64\Debug\boyi.exe" (
    echo [OK] x64\Debug\boyi.exe exists
) else (
    echo [WARN] x64\Debug\boyi.exe not found (optional)
)
echo.

echo [Step 3/6] Checking dependencies...
if exist "server\node_modules" (
    echo [OK] Dependencies installed
) else (
    echo [WARN] Dependencies not installed
    echo Run: cd server ^&^& npm install
)
echo.

echo [Step 4/6] Testing port 3000...
netstat -ano | findstr ":3000" >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [WARN] Port 3000 is in use
    echo Current processes using port 3000:
    netstat -ano | findstr ":3000"
) else (
    echo [OK] Port 3000 is available
)
echo.

echo [Step 5/6] Checking firewall...
echo [INFO] If connection fails, check Windows Firewall
echo       Allow Node.js through firewall if prompted
echo.

echo [Step 6/6] Testing basic server...
if exist "server\node_modules" (
    echo Starting test server for 5 seconds...
    cd server
    start /B node test-server.js
    timeout /t 2 /nobreak >nul
    
    echo Testing connection...
    curl -s http://localhost:3000/api/health >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        echo [OK] Server responds correctly
    ) else (
        echo [FAIL] Server not responding
        echo Try: curl http://localhost:3000/api/health
    )
    
    REM Stop test server
    taskkill /F /FI "WINDOWTITLE eq node*" >nul 2>nul
    cd ..
) else (
    echo [SKIP] Dependencies not installed
)
echo.

echo ========================================
echo   Diagnostic Complete
echo ========================================
echo.
echo Next steps:
echo 1. If Node.js missing: Install from nodejs.org
echo 2. If dependencies missing: Run server\install.bat
echo 3. If port in use: Close other applications
echo 4. If all OK: Run start-all.bat
echo.

pause
