@echo off
SETLOCAL

rem This script will work if gdb and CrashDebug executables
rem are located in the same directory as the script

set CURRENT_DIR=%~dp0
set GDB=%CURRENT_DIR%arm-none-eabi-gdb.exe
rem CrashDebug path must use forward slash
set CRASHDEBUG=%CURRENT_DIR:\=/%CrashDebug.exe

if "%~1" == "" if [%1] == [] call :Usage & goto :ExitScript
if "%~2" == "" if [%2] == [] call :Usage & goto :ExitScript
if not [%3] == [] if not "%~3" == "--gdb" call :Usage & goto :ExitScript

if not exist %1% (
    echo.
    echo Elf file does not exist
    goto UsageAndExit
)
if not exist %2% (
    echo.
    echo Dump file does not exist
    goto UsageAndExit
)

set ELF=%1
set ELF=%ELF:\=\\%
set DUMP=%2
set DUMP=%DUMP:\=\\%

if "%~3" == "--gdb" (
	%GDB% %1 -ex "set target-charset ASCII" -ex "target remote |  %CRASHDEBUG% --elf %ELF% --dump %DUMP%" -quiet -ex "echo \nBacktrace\n" -ex "bt" -ex "echo \nFault status registers\n" -ex "x/5xw 0xE000ED28" -ex "echo \nRegisters\n" -ex "info registers"
) else (
	%GDB% %1 -ex "set target-charset ASCII" -ex "target remote |  %CRASHDEBUG% --elf %ELF% --dump %DUMP%" -quiet -ex "echo \nBacktrace\n" -ex "bt" -ex "echo \nFault status registers\n" -ex "x/5xw 0xE000ED28" -ex "echo \nRegisters\n" -ex "info registers" -ex "kill inferiors 1" -ex "quit" > %CURRENT_DIR%\crash_dump.txt 2>nul
	%CURRENT_DIR%\crash_dump.txt
)
goto ExitScript


:UsageAndExit
call :Usage
goto :ExitScript

:Usage
echo.
echo Usage:
echo 	crash_debug.bat ^<elf-file^> ^<crash-dump-file^> [--gdb]
echo.
echo 	Options:
echo 	--gdb		: Keep the gdb session active
echo.
exit /b 0

:ExitScript
