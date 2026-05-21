@echo off
REM 简化的编译脚本 - 使用Visual Studio项目编译

echo ========================================
echo 国际跳棋AI - 使用VS项目编译
echo ========================================
echo.

REM 检查是否存在sln文件
if not exist "boyi.sln" (
    echo [错误] 找不到 boyi.sln
    pause
    exit /b 1
)

echo 使用Visual Studio编译项目...
echo.
echo 方法1: 使用MSBuild命令行编译
echo ----------------------------------------
echo.

REM 尝试使用MSBuild
where msbuild >nul 2>&1
if %errorlevel% equ 0 (
    echo 找到MSBuild，开始编译...
    msbuild boyi.sln /p:Configuration=Release /p:Platform=x64 /m
    
    if %errorlevel% equ 0 (
        echo.
        echo ========================================
        echo 编译成功！
        echo ========================================
        echo.
        echo 可执行文件: x64\Release\boyi.exe
        echo.
        pause
        exit /b 0
    ) else (
        echo.
        echo [错误] MSBuild编译失败
        echo.
    )
) else (
    echo MSBuild未找到
    echo.
)

echo 方法2: 手动使用Visual Studio编译
echo ----------------------------------------
echo.
echo 请按照以下步骤操作：
echo.
echo 1. 双击打开 boyi.sln
echo 2. 在顶部工具栏选择 "Release" 配置
echo 3. 在顶部工具栏选择 "x64" 平台
echo 4. 按 F7 或点击 "生成" -^> "生成解决方案"
echo 5. 等待编译完成
echo.
echo 编译完成后，可执行文件位于: x64\Release\boyi.exe
echo.
echo 然后运行: test-opening-book.bat 测试性能
echo.
pause
