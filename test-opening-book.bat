@echo off
REM 开局库测试脚本
REM 用于测试国际跳棋AI的开局库功能

echo ========================================
echo 国际跳棋AI - 开局库测试
echo ========================================
echo.

REM 检查可执行文件是否存在
if not exist "x64\Debug\boyi.exe" (
    echo [错误] 找不到 x64\Debug\boyi.exe
    echo 请先编译项目！
    echo.
    echo 编译步骤：
    echo 1. 打开 boyi.sln
    echo 2. 选择 Debug 配置
    echo 3. 按 F7 编译
    pause
    exit /b 1
)

REM 检查开局库文件是否存在
if not exist "opening_book.txt" (
    echo [警告] 找不到 opening_book.txt
    echo 程序将使用内置开局库
    echo.
)

echo [步骤1] 编译检查 - 通过
echo [步骤2] 文件检查 - 通过
echo.

REM 创建测试输入文件
echo [步骤3] 创建测试输入...
(
    echo START
    echo MOVE
    echo QUIT
) > test_input.txt

echo 测试输入已创建
echo.

REM 运行测试
echo [步骤4] 运行开局库测试...
echo ----------------------------------------
echo 测试场景：初始局面查询开局库
echo ----------------------------------------
echo.

REM 运行程序并捕获输出
x64\Debug\boyi.exe < test_input.txt > test_output.txt 2>&1

REM 显示输出
type test_output.txt

echo.
echo ========================================
echo 测试结果分析
echo ========================================
echo.

REM 检查是否命中开局库
findstr /C:"Opening book hit" test_output.txt > nul
if %errorlevel% equ 0 (
    echo [✓] 开局库命中测试 - 通过
    echo     程序成功从开局库中查询到走法
) else (
    echo [✗] 开局库命中测试 - 失败
    echo     程序未能从开局库中查询到走法
    echo     可能原因：
    echo     1. opening_book.txt 文件不存在或为空
    echo     2. 开局库加载失败
    echo     3. 初始局面不在开局库中
)
echo.

REM 检查是否输出统计信息
findstr /C:"开局库统计" test_output.txt > nul
if %errorlevel% equ 0 (
    echo [✓] 统计信息输出 - 通过
    echo     程序输出了开局库统计信息
) else (
    echo [✗] 统计信息输出 - 失败
    echo     程序未输出开局库统计信息
)
echo.

REM 检查是否有错误
findstr /C:"ERROR" test_output.txt > nul
if %errorlevel% equ 0 (
    echo [✗] 错误检查 - 发现错误
    echo     请查看 test_output.txt 了解详情
) else (
    echo [✓] 错误检查 - 无错误
)
echo.

echo ========================================
echo 详细测试报告
echo ========================================
echo.
echo 测试输出已保存到: test_output.txt
echo 测试输入已保存到: test_input.txt
echo.
echo 查看完整输出：
echo     type test_output.txt
echo.
echo 查看开局库统计：
echo     findstr /C:"开局库" test_output.txt
echo.

REM 清理临时文件（可选）
REM del test_input.txt
REM del test_output.txt

echo 测试完成！
echo.
pause
