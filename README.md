# DevAlert Demo on STM32L4 IoT Discovery Kit

This project demonstrates Percepio DevAlert (https://percepio.com/devalert) on the STM32L4 IoT Discovery kit, https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html, with FreeRTOS. 

The demo includes the DevAlert target library (DFM) extended with the CrashCatcher library (for crash dumps), and as well as the TraceRecorder library for providing traces to Tracealyzer. These libraries are found under \libraries\3rdparty.

Examples of how to use the DFM library is found in main.c (vendors\st\boards\stm32l475_discovery\aws_demos\application_code\main.c)

The development tool used for this project is SW4STM32 (https://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32) but it should be possible to import the project into STM32CubeIDE.

To build the project in SW4STM32, import it using "Import...", "Projects from Folder or Archive", select the root folder of this repository and make sure to include all projects found. The relevant one is "aws_demos", but it seems the others are needed to the build to work.

To connect to Wifi and AWS IoT Core, you need to update aws_clientcredential.h and aws_clientcredential_keys.h (see \demos\include) as described on https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html.

## Downloading and viewing diagnostic data

The provided alerts include diagnostic payloads such as traces and crash dumps. They are stored outside the DevAlert service for privacy reasons (e.g. in your own Amazon S3 bucket). When clicking on the dashboard links, the selected data is downloaded to your local computer so it can be inspected using the appropriate desktop tool, for example GDB or Tracealyzer. This workflow is automated using the DevAlert Dispatcher tool, available at https://devalert.io/dispatcher. 
The first time you need to start it manually and configure it, as decribed at https://devalert.io/dispatcher/setup.

### Viewing event traces in Tracealyzer

The demo includes the TraceRecorder library that produces event traces for Tracealyzer. This shows a timeline of the FreeRTOS scheduling and API calls, and also allows for custom logging from the application code. If you don't already have Tracealyzer installed, you can download it for evaluation at https://percepio.com.

### Viewing crash dumps with GDB/CrashDebug

Copy the crash_debug.bat or crash_debug.sh script from the <dispatcher>/template directory and update the File Mappin rule for "dmp" so that it is called with the right arguments. For example:
Extension: dmp
Executable: C:\DevAlertDispatcher\crash_debug.bat
Startup folder: C:\DevAlertDispatcher
Parameters: ./latestcrashdump/aws_demos.elf ./latestcrashdump/latest.dmp --gdb

In general, CrashDebug is called from the GDB client in the following way:

    arm-none-eabi-gdb main.elf -ex "set target-charset ASCII" -ex "target remote | CrashDebug --elf main.elf

But Dispatcher include scripts that take care of this for you. 

Note that CrashDebug is only available for Arm Cortex-M devices, but similar integrations are possible for other processors.

Note that the ELF file is not provided via DevAlert. But the firmware version/revision is provided and this can be used as a parameter in a custom script called by Dispatcher (under the Scripts tab) in order to generate the path to the right ELF file and perhaps copy it to a local folder.

### Viewing crash dumps in Eclipse

You can create a separate "debug configuration" in Eclipse-based IDEs to load crashdumps from DevAlert.

See the example in "CrashDebug - Load Crashdumps in Eclipse". The main file is loadcrashdebug.cfg, which is a GDB command file.
This is used in an Eclipse debug configuration of the type "GDB Hardware Debugging". In the Debug Configuration editor, enter the following:
- Main / C/C++ Application: the ELF file in the build directory (not used but must be provided)
- Main / Disable auto build: Checked
- Debugger / GDB command: Full path to the right GDB client (e.g. arm-none-eabi-gdb.exe)
- Debugger / Use remote target: Unchecked
- Startup / Initialization commands: "source /path/to/loadcrashdump.cfg"
- Startup / Other things: All unchecked (don't "load image" or "load symbols", this is done by loadcrashdump.cfg)

Note that this assumes that the elf and dmp files can be found by loadcrashdump.cfg. This example uses a hardcoded folder for this, where the trace and dmp files from Dispatcher are copied by a modified version of the crashdebug script. For example by adding:

    cp %1 ./latestcrashdump/latest.elf
    cp %2 ./latestcrashdump/latest.dmp

## Porting / Integration

To port this to another target, you may need to update the following files:
- dfmUser.c - Used-defined definitions needed by DFM.
- dfmStoragePort.c - How to store Alerts to flash (before restarting) - currently using "libraries\3rdparty\DFM\storageports\Dummy\dfmStoragePort.c"
- dfmCloudPort.c - How to upload Alerts to the device backend (e.g. an AWS account). See \libraries\3rdparty\DFM\kernelports\FreeRTOS\cloudports\AWS_MQTT\dfmCloudPort.c
- trcConfig.h - If porting to a different processor architecture, you need to select the right hardware port setting. Currently set to "Arm Cortex-M" matching STM32L4.

## Target support

The DFM library (the DevAlert client) is not hardware dependent and can be used on any processor. But the crash dump support relies on crashcatcher and crashdebug which is only available for Arm Cortex-M devices. Officially it only supports ARMv6-M and ARMv7-M processors, but it seems to work fine also on ARMv8-M cores like Cortex-M33, at least with Arm TrustZone disabled. As of April 2023, it has not yet been tested with ARM TrustZone. 
