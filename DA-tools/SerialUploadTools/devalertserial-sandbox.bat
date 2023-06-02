echo off
rm tterm.log
rm data.bin
type nul > data.bin
echo Teraterm Log Begins > %cd%\tterm.log
start "" ttermpro.exe /C=9 /L="%cd%\tterm.log"
sleep 1
python devalertserial.py --file %cd%\tterm.log --upload sandbox | devalerthttps.exe store-trace
