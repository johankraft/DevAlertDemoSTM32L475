echo off
rm tterm.log
echo Teraterm Log Begins > %cd%\tterm.log
start "Teraterm" ttermpro.exe /C=9 /L="%cd%\tterm.log"
sleep 2
python devalertserial.py --file %cd%\tterm.log --upload s3
