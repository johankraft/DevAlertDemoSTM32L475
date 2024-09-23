
rem Update these paths to match your setup.
set ALERTS_DIR=alert_data_test
set DEVICE_OUTPUT_LOGFILE="C:\da-tools-zephyr\tterm.log"

python percepio_receiver.py --upload file --folder %ALERTS_DIR% --eof wait --deviceid TestBoard77 %DEVICE_OUTPUT_LOGFILE% 