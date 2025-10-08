@echo off
setlocal enabledelayedexpansion

echo ========================================
echo RLXLand 集成测试运行脚本
echo ========================================
echo.

REM 检查xmake是否可用
where xmake >nul 2>nul
if %errorlevel% neq 0 (
    echo 错误: 未找到xmake，请确保xmake已安装并添加到PATH
    pause
    exit /b 1
)

REM 设置构建配置
set BUILD_TYPE=release
set TARGET_PLATFORM=windows
set TARGET_ARCH=x64

REM 解析命令行参数
:parse_args
if "%1"=="--debug" set BUILD_TYPE=debug & shift & goto parse_args
if "%1"=="--release" set BUILD_TYPE=release & shift & goto parse_args
if "%1"=="--clean" set CLEAN_BUILD=1 & shift & goto parse_args
if "%1"=="--verbose" set VERBOSE=1 & shift & goto parse_args
if "%1"=="--help" goto show_help
if not "%1"=="" goto unknown_arg

echo 配置测试环境...
echo 构建类型: %BUILD_TYPE%
echo 目标平台: %TARGET_PLATFORM%
echo 目标架构: %TARGET_ARCH%
echo.

REM 清理构建（如果指定）
if defined CLEAN_BUILD (
    echo 清理之前的构建...
    xmake clean -v
    if %errorlevel% neq 0 (
        echo 清理失败
        pause
        exit /b 1
    )
    echo.
)

REM 配置项目
echo 配置项目...
xmake f -y -p %TARGET_PLATFORM% -a %TARGET_ARCH% -m %BUILD_TYPE% --tests=y
if %errorlevel% neq 0 (
    echo 项目配置失败
    pause
    exit /b 1
)

REM 构建测试
echo.
echo 构建测试目标...
if defined VERBOSE (
    xmake build -v RLXLand_tests
) else (
    xmake build RLXLand_tests
)

if %errorlevel% neq 0 (
    echo 测试构建失败
    pause
    exit /b 1
)

REM 运行测试
echo.
echo ========================================
echo 运行集成测试
echo ========================================
echo.

if defined VERBOSE (
    xmake run -v RLXLand_tests -- --durations yes --reporter=console
) else (
    xmake run RLXLand_tests
)

set TEST_RESULT=%errorlevel%

echo.
echo ========================================
echo 测试完成
echo ========================================
echo.

REM 生成测试报告
if %TEST_RESULT% equ 0 (
    echo ✓ 所有测试通过
    echo 正在生成测试报告...
    
    REM 创建报告目录
    if not exist "tests\reports" mkdir tests\reports
    
    REM 运行测试并生成XML报告
    xmake run RLXLand_tests -- --durations yes --reporter=xml --out=tests\reports\test_report.xml
    
    echo 测试报告已生成: tests\reports\test_report.xml
) else (
    echo ✗ 测试失败，退出代码: %TEST_RESULT%
    echo 请检查上述错误信息并修复问题
)

echo.
pause
exit /b %TEST_RESULT%

:show_help
echo RLXLand 测试运行脚本
echo.
echo 用法: run_tests.bat [选项]
echo.
echo 选项:
echo   --debug     使用调试构建
echo   --release   使用发布构建 (默认)
echo   --clean     清理之前的构建
echo   --verbose   显示详细输出
echo   --help      显示此帮助信息
echo.
echo 示例:
echo   run_tests.bat                    # 默认发布构建
echo   run_tests.bat --debug --verbose  # 调试构建，详细输出
echo   run_tests.bat --clean            # 清理并重新构建
echo.
pause
exit /b 0

:unknown_arg
echo 未知参数: %1
echo 使用 --help 查看帮助信息
pause
exit /b 1