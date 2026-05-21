@echo off
REM 性能测试运行脚本

echo ========================================
echo 国际跳棋AI - 性能测试套件
echo ========================================
echo.

REM 步骤1：编译benchmark
echo [步骤1] 编译性能基准测试...
cl /EHsc /std:c++17 /O2 /Oi /Ot /GL /arch:AVX2 benchmark.cpp /Fe:benchmark.exe /link /LTCG 2>nul

if %errorlevel% neq 0 (
    echo [警告] AVX2编译失败，尝试标准优化...
    cl /EHsc /std:c++17 /O2 /Oi /Ot benchmark.cpp /Fe:benchmark.exe 2>nul
    
    if %errorlevel% neq 0 (
        echo [错误] 编译失败！
        pause
        exit /b 1
    )
)

echo ✓ benchmark.exe 编译成功
echo.

REM 步骤2：运行benchmark
echo [步骤2] 运行性能基准测试...
echo ========================================
echo.

benchmark.exe

echo.
echo ========================================
echo 性能测试完成！
echo ========================================
echo.

REM 清理临时文件
del benchmark.obj 2>nul
del benchmark.exp 2>nul
del benchmark.lib 2>nul

pause
