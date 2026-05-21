@echo off
REM 自我对弈测试运行脚本

echo ========================================
echo 国际跳棋AI - 自我对弈测试
echo ========================================
echo.

REM 步骤1：编译selfplay
echo [步骤1] 编译自我对弈程序...
cl /EHsc /std:c++17 /O2 /Oi /Ot /GL /arch:AVX2 selfplay.cpp /Fe:selfplay.exe /link /LTCG 2>nul

if %errorlevel% neq 0 (
    echo [警告] AVX2编译失败，尝试标准优化...
    cl /EHsc /std:c++17 /O2 /Oi /Ot selfplay.cpp /Fe:selfplay.exe 2>nul
    
    if %errorlevel% neq 0 (
        echo [错误] 编译失败！
        pause
        exit /b 1
    )
)

echo ✓ selfplay.exe 编译成功
echo.

REM 步骤2：运行selfplay
echo [步骤2] 运行自我对弈测试...
echo.
echo 默认配置：100局，每步1000ms
echo 如需修改，使用: selfplay.exe [对局数] [每步时间ms]
echo.
echo ========================================
echo.

selfplay.exe 100 1000

echo.
echo ========================================
echo 自我对弈测试完成！
echo ========================================
echo.
echo 结果已保存到: selfplay_results.txt
echo.

REM 清理临时文件
del selfplay.obj 2>nul
del selfplay.exp 2>nul
del selfplay.lib 2>nul

pause
