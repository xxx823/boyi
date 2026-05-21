@echo off
REM 简化的性能测试脚本 - 直接使用现有的测试输出

echo ========================================
echo 国际跳棋AI - 性能分析
echo ========================================
echo.

echo 正在分析之前的测试结果...
echo.

if not exist "test_output.txt" (
    echo [错误] 找不到 test_output.txt
    echo 请先运行: test-opening-book.bat
    pause
    exit /b 1
)

echo ========== 性能指标分析 ==========
echo.

REM 从test_output.txt中提取NPS数据
findstr /C:"NPS" test_output.txt

echo.
echo ========== 搜索性能 ==========
findstr /C:"搜索" test_output.txt | findstr /C:"节点"

echo.
echo ========== 开局库性能 ==========
findstr /C:"开局库" test_output.txt

echo.
echo ========================================
echo.
echo 详细结果请查看: test_output.txt
echo.
echo 性能目标:
echo   NPS: ^> 100,000 (当前约 55,000-60,000)
echo   需要提升: 约 70-80%%
echo.
echo 优化建议:
echo 1. 使用 compile-optimized.bat 重新编译
echo 2. 关闭其他占用CPU的程序
echo 3. 考虑实现多线程搜索
echo.
pause
