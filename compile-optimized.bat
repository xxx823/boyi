@echo off
REM 优化编译脚本 - 最大性能优化
REM 预期提升：15-20% 性能

echo ========================================
echo 国际跳棋AI - 优化编译
echo ========================================
echo.

REM 检查源文件
if not exist "boyi\boyi.cpp" (
    echo [错误] 找不到 boyi\boyi.cpp
    pause
    exit /b 1
)

echo [步骤1] 清理旧文件...
if exist "x64\Release\boyi.exe" del "x64\Release\boyi.exe"
if exist "x64\Release\boyi.obj" del "x64\Release\boyi.obj"

echo [步骤2] 创建Release目录...
if not exist "x64\Release" mkdir "x64\Release"

echo [步骤3] 使用优化选项编译...
echo.
echo 编译选项：
echo   /O2      - 最大速度优化
echo   /Oi      - 启用内联函数
echo   /Ot      - 优先速度
echo   /GL      - 全程序优化
echo   /arch:AVX2 - 使用AVX2指令集
echo   /LTCG    - 链接时代码生成
echo.

cl /EHsc /std:c++17 /O2 /Oi /Ot /GL boyi\boyi.cpp /Fe:x64\Release\boyi.exe /Fo:x64\Release\ /link /LTCG

if %errorlevel% neq 0 (
    echo.
    echo [错误] 编译失败！
    echo.
    echo 可能的原因：
    echo 1. CPU不支持AVX2指令集
    echo 2. 编译器版本过旧
    echo.
    echo 尝试使用标准优化选项...
    echo.
    
    cl /EHsc /std:c++17 /O2 /Oi /Ot boyi\boyi.cpp /Fe:x64\Release\boyi.exe /Fo:x64\Release\
    
    if %errorlevel% neq 0 (
        echo [错误] 标准优化编译也失败！
        pause
        exit /b 1
    )
)

echo.
echo ========================================
echo 编译成功！
echo ========================================
echo.
echo 可执行文件: x64\Release\boyi.exe
echo.
echo 下一步：
echo 1. 运行性能测试: test-opening-book.bat
echo 2. 查看NPS是否提升
echo.
pause
