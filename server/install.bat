@echo off
echo ========================================
echo   安装Node.js依赖
echo ========================================
echo.

REM 检查Node.js是否安装
where node >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [错误] 未检测到Node.js！
    echo.
    echo 请先安装Node.js:
    echo 1. 访问 https://nodejs.org/
    echo 2. 下载并安装LTS版本
    echo 3. 重新运行此脚本
    echo.
    pause
    exit /b 1
)

echo [✓] Node.js已安装
node --version
echo.

echo 正在安装依赖包...
echo.

call npm install

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   安装完成！
    echo ========================================
    echo.
    echo 下一步：运行 start.bat 启动服务器
    echo.
) else (
    echo.
    echo [错误] 安装失败！
    echo.
)

pause
