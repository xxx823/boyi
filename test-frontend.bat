@echo off
chcp 65001 >nul
echo ========================================
echo   Testing Frontend
echo ========================================
echo.
echo Opening web interface in browser...
echo URL: http://localhost:8080
echo.
echo Press Ctrl+C to stop the server
echo.

cd web
python -m http.server 8080
