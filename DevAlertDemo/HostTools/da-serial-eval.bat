
rem Example script for uploading DevAlert data from a serial terminal log to a DevAlert evaluation account
echo off

rem Update these paths to match your project build and file system.
set CURRENT_ELF_FILE="C:\src\github-repos\DevAlertDemoSTM32L475-May24\Debug-Serial\aws_demos.elf"
set ELF_ARCHIVE_FOLDER="X:\elf-files"

rem Copy elf file and rename it using the GCC Build ID.
python store-elf.py %CURRENT_ELF_FILE% %ELF_ARCHIVE_FOLDER%

rem Parse all text that is written to the log file, extract the DevAlert data and pass it to devalerthttps for upload
python devalertserial.py --upload sandbox teraterm.log | devalerthttps.exe store-trace
