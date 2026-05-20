@echo off
echo ========================================
echo   国际跳棋AI服务器启动器
echo ========================================
echo.

REM 检查Node.js
where node >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [错误] 未检测到Node.js！
    echo 请先运行 install.bat 安装依赖
    echo.
    pause
    exit /b 1
)

REM 检查node_modules
if not exist "node_modules" (
    echo [警告] 未检测到依赖包！
    echo 正在自动安装...
    echo.
    call npm install
    echo.
)

echo 正在启动服务器...
echo.
echo ========================================
echo   服务器信息
echo ========================================
echo   地址: http://localhost:3000
echo   API: http://localhost:3000/api/health
echo ========================================
echo.
echo 按 Ctrl+C 停止服务器
echo.

REM 启动服务器
node server.js

pause
