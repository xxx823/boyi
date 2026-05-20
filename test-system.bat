@echo off
echo ========================================
echo   系统测试脚本
echo ========================================
echo.

echo [测试 1/4] 检查Node.js...
where node >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [✓] Node.js已安装
    node --version
) else (
    echo [✗] Node.js未安装
    echo     请访问 https://nodejs.org/ 下载安装
)
echo.

echo [测试 2/4] 检查C++引擎...
if exist "x64\Debug\boyi.exe" (
    echo [✓] C++引擎已编译
    echo     路径: x64\Debug\boyi.exe
) else (
    echo [✗] C++引擎未找到
    echo     请在Visual Studio中编译项目
)
echo.

echo [测试 3/4] 检查Web文件...
if exist "web\index.html" (
    echo [✓] Web前端文件存在
) else (
    echo [✗] Web前端文件缺失
)
echo.

echo [测试 4/4] 检查服务器文件...
if exist "server\server.js" (
    echo [✓] 服务器文件存在
) else (
    echo [✗] 服务器文件缺失
)
echo.

echo ========================================
echo   测试完成
echo ========================================
echo.

if exist "server\node_modules" (
    echo [提示] 依赖已安装，可以直接运行 start-all.bat
) else (
    echo [提示] 依赖未安装，首次运行会自动安装
)
echo.

pause
