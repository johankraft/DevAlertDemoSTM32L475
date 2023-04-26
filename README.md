# DevAlert Demo on STM32L4 IoT Discovery Kit

This project demonstrates Percepio DevAlert (https://percepio.com/devalert) on the STM32L4 IoT Discovery kit, https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html, with FreeRTOS. 

The demo includes the DevAlert target library (DFM) extended with the CrashCatcher library (for crash dumps), and as well as the TraceRecorder library for providing traces to Tracealyzer. These libraries are found under \libraries\3rdparty.

Examples of how to use the DFM library is found in main.c (vendors\st\boards\stm32l475_discovery\aws_demos\application_code\main.c)

The development tool used for this project is SW4STM32 (https://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32) but it should be possible to import the project into STM32CubeIDE.

To build the project in SW4STM32, import it using "Import...", "Projects from Folder or Archive", select the root folder of this repository and make sure to include all projects found. The relevant one is "aws_demos", but it seems the others are needed to the build to work.

To connect to Wifi and AWS IoT Core, you need to update aws_clientcredential.h and aws_clientcredential_keys.h (see \demos\include) as described on https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html.

To port this to another target, you may need to update the following files:
- dfmUser.c - Used-defined definitions needed by DFM.
- dfmStoragePort.c - How to store Alerts to flash (before restarting) - currently using "libraries\3rdparty\DFM\storageports\Dummy\dfmStoragePort.c"
- dfmCloudPort.c - How to upload Alerts to the device backend (e.g. an AWS account). See \libraries\3rdparty\DFM\kernelports\FreeRTOS\cloudports\AWS_MQTT\dfmCloudPort.c
- trcConfig.h - If porting to a different processor architecture, you need to select the right hardware port setting. Currently set to "Arm Cortex-M" matching STM32L4.

Note that crashcatcher is only for Arm Cortex-M devices. Officially it only supports Arm v7-M processors, but it seems to work fine also on v8-M processors like Cortex-M33. But this has not been tested with ARM TrustZone enabled. 
