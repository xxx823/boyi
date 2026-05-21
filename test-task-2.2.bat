@echo off
REM 测试任务2.2：中局着法排序增强

echo ========================================
echo 任务2.2测试：中局着法排序增强
echo ========================================
echo.

REM 检查是否存在测试文件
if not exist "test-midgame-move-ordering.cpp" (
    echo [错误] 找不到测试文件 test-midgame-move-ordering.cpp
    pause
    exit /b 1
)

echo 正在编译测试程序...
echo.

REM 尝试使用g++编译
where g++ >nul 2>&1
if %errorlevel% equ 0 (
    echo 使用g++编译...
    g++ -std=c++17 -O2 -o test-task-2.2.exe test-midgame-move-ordering.cpp
    
    if %errorlevel% equ 0 (
        echo 编译成功！
        echo.
        echo 运行测试...
        echo ========================================
        echo.
        test-task-2.2.exe
        echo.
        echo ========================================
        echo 测试完成！
        pause
        exit /b 0
    ) else (
        echo [错误] g++编译失败
    )
)

REM 尝试使用cl编译
where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo 使用MSVC编译...
    cl /EHsc /std:c++17 /O2 /Fe:test-task-2.2.exe test-midgame-move-ordering.cpp
    
    if %errorlevel% equ 0 (
        echo 编译成功！
        echo.
        echo 运行测试...
        echo ========================================
        echo.
        test-task-2.2.exe
        echo.
        echo ========================================
        echo 测试完成！
        pause
        exit /b 0
    ) else (
        echo [错误] MSVC编译失败
    )
)

echo.
echo [错误] 未找到可用的C++编译器（g++或cl）
echo.
echo 请安装以下之一：
echo 1. MinGW-w64 (提供g++编译器)
echo 2. Visual Studio (提供cl编译器)
echo.
pause
exit /b 1
