# DevAlertDemoSTM32L475

This project demonstrates Percepio DevAlert (https://percepio.com/devalert) on an STM32L475 device with FreeRTOS. 
Currently this is still a work in progress, but is working. The instructions are however not yet complete.

The development tools used is SW4STM32 (https://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32) but it should be possible to import the project into STM32CubeIDE.

There are multiple projects here, the one to run is "aws_demos". This includes the DevAlert target library (DFM) extended with the CrashCatcher library, and as well as the TraceRecorder library for Tracealyzer. These are found under \libraries\3rdparty.

Examples of how to use the DFM library is found in main.c (vendors\st\boards\stm32l475_discovery\aws_demos\application_code\main.c)

To port this to another target, you may need to update the following files:
- dfmUser.c - Used-defined definitions needed by DFM.
- dfmStoragePort.c - How to store Alerts to flash (before restarting) - currently using "libraries\3rdparty\DFM\storageports\Dummy\dfmStoragePort.c"
- dfmCloudPort.c - How to upload Alerts to the device backend (e.g. an AWS account). See \libraries\3rdparty\DFM\kernelports\FreeRTOS\cloudports\AWS_MQTT\dfmCloudPort.c

You also need to update aws_clientcredential.h and aws_clientcredential_keys.h in \demos\include as described on https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html


