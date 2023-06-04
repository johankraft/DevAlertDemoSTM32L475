# DevAlert Demo on STM32L4 IoT Discovery Kit

This project demonstrates Percepio DevAlert (https://percepio.com/devalert) on the STM32L4 IoT Discovery kit with FreeRTOS.
This includes the DevAlert target library (DFM) with crash dump support and as well as the TraceRecorder library for providing traces to Tracealyzer.

Learn more about the STM32L4 IoT Discovery kit at https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html. 

Important folders and files:
- /libraries/3rdparty/DFM  -- in particular /config/dfmConfig.h
- /libraries/3rdparty/tracerecorder  -- in particular /config/trcConfig.h
- /vendors/st/boards/stm32l475_discovery/aws_demos/application_code/main.c
- /vendors/st/boards/stm32l475_discovery/aws_demos/config_files/FreeRTOSConfig.h
- /demos/include/aws_clientcredential.h
- /demos/include/aws_clientcredential_keys.h

Examples of how to use the DFM library and TraceRecorder is found in main.c and in FreeRTOSConfig.h.

In main.c, the important parts are:
 - DevAlert test cases, https://github.com/johankraft/DevAlertDemoSTM32L475/blob/main/vendors/st/boards/stm32l475_discovery/aws_demos/application_code/main.c#L223
 - Tracing a state machine, https://github.com/johankraft/DevAlertDemoSTM32L475/blob/main/vendors/st/boards/stm32l475_discovery/aws_demos/application_code/main.c#L287
 - Initializing DFM, https://github.com/johankraft/DevAlertDemoSTM32L475/blob/main/vendors/st/boards/stm32l475_discovery/aws_demos/application_code/main.c#L406
 - Initializing TraceRecorder, https://github.com/johankraft/DevAlertDemoSTM32L475/blob/main/vendors/st/boards/stm32l475_discovery/aws_demos/application_code/main.c#L789

In FreeRTOSConfig.h, the important parts are:

- #define configASSERT( x ) ... definition and the related #includes (just above)
- #define configUSE_TRACE_FACILITY 1 -- Critical for recording kernel events
- #include "trcRecorder.h" -- Critical for recording kernel events

## Loading the demo project

The development tool used for this demo project is SW4STM32, available at https://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32

To open and build the project in SW4STM32, follow the following steps:
- Import the files into a new project using "Import...", "Projects from Folder or Archive".

UPDATE THIS - WHAT IS ACTUALLY NEEDED?
- Select the root folder of this repository and make sure to include all projects found.
- Right-click on the "aws_demos" project and select "Build project". 

CREATE A DEBUG CONFIGURATION?

and , but it seems the others are needed to the build to work.

To connect to Wifi and AWS IoT Core, you need to update aws_clientcredential.h and aws_clientcredential_keys.h (see \demos\include) as described on https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html.


## Uploading DevAlert data

This demo contains two "build configurations" showing two different methods for uploading the data:

- "Debug" uploads directly to AWS IoT Core using a Wi-Fi network.
- "Debug-SerialOnly" uploads via the host computer using the Virtual COM port on the onboard STLINK debugger.

The build configuration is selected under "Project" -> "Build Configurations" -> "Set Active". The following sections explain how to use them.

### Uploading via the STLINK COM port ("Debug-SerialOnly")

This method is easiest to get started with and doesn't require any AWS account or cloud-side configuration. 
The only things needed is a DevAlert evaluation account (https://devalert.io/auth/signup), a terminal program like Teraterm, and Python installed.
The rest is provided in this repository.

In this configuration, the DFM library outputs the data as hex strings to the serial VCOM interface. This way, it can be received using a plain terminal program. 
The terminal program is configured to log the data to a text file, which is processed by the DevAlert tools and then uploaded to the cloud.
This can run continously and upload multiple alerts from the device over a long time period, if needed.

(Demo application) ---> (STLINK VCOM/USB) ---> (DevAlert desktop tools) ---> (DevAlert cloud account)

For the terminal program we mostly use Teraterm, but any terminal program with a logging capability should work. 

The processing by the DevAlert desktop tools consists of two steps:
- devalertserial.py - Extracts the data from the terminal log file and converts to binary.
- Upload to cloud, using one of the upload tools:
  - devalerthttps - uploads to your DevAlert evaluation account (hosted by Percepio)
  - devalerts3 - uploads to your Amazon S3 bucket ("bring-your-own-storage")

In this demo, we use the devalerthttps tool for the upload, which uploads to Percepio-hosted storage in your DevAlert evaluation account.
This is not intended for direct upload from devices or production data, but is very easy to get started with.

#### Setup

1. Install Python if you don't already have it.

2. Run devalerthttps to enter your DevAlert credentials. These are saved for future sessions.

	devalerthttps set-username --username email@domain.com
	
	devalerthttps set-password
	Password: 

Note that the username/password authentication is only used for evaluation accounts. For production use, we recommend
using our privacy-friendly "bring-your-own-storage" solution where you integrate own storage, e.g. an Amazon S3 bucket.

3. If using Windows and have Teraterm installed, run DA-tools/devalertserial-sandbox.bat with the COM port provided as argument. Like this:

	devalertserial-sandbox.bat 9	

	This will start Teraterm on COM9 and set up logging to the file tterm.log, and then run the following:

		python devalertserial.py --file tterm.log --upload sandbox | devalerthttps.exe store-trace
	
	You can easily create a script for this, that also starts the terminal program.
	This assumes that Teraterm is installed, but that part can easily be changed if needed.

If using Linux or prefer another terminal program, follow these steps (manually or in a custom script)

	- Start a terminal program on the right COM port, with logging to file MYFILE.
	
	- Run "python devalertserial.py --file MYFILE --upload sandbox | devalerthttps.exe store-trace"

Just make sure no other terminal program instances are connected to the same COM port.

### Uploading via AWS IoT Core ("Debug")

This method requires a Wi-Fi network, an AWS account that has been set up for DevAlert, and a full DevAlert account with AWS support (provided on request).
Contact support@percepio.com if you need help to set this up.

Befure building the demo project, make sure to select the build configuration "Debug". This will include the AWS_MQTT cloud port (DFM/kernelports/FreeRTOS/cloudports/AWS_MQTT) and set the build flag DFM_CFG_SERIAL_UPLOAD_ONLY=0 which enables the Wi-Fi code.

To connect to your Wi-Fi network and to AWS IoT Core in your AWS account, you need to update aws_clientcredential.h and aws_clientcredential_keys.h (see \demos\include) as described on https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html.








## Demo operation

Start a debug session in SW4STM32 using the right debug configuration (AWS or Serial) and it should begin generating errors and alerts randomly. 
The system is configured to restart after each error/alert.

Log in to https://devalert.io and see the reported issues on the Dashboard page.

## Downloading and viewing diagnostic data

The provided alerts include diagnostic payloads such as traces and crash dumps. They are stored outside the DevAlert service for privacy reasons (e.g. in your own Amazon S3 bucket). When clicking on the dashboard links, the selected data is downloaded to your local computer so it can be inspected using the appropriate desktop tool, for example GDB or Tracealyzer. This workflow is automated using the DevAlert Dispatcher tool, available at https://devalert.io/dispatcher. 
The first time you need to start it manually and configure it, as decribed at https://devalert.io/dispatcher/setup.

### Viewing event traces in Tracealyzer

The demo includes the TraceRecorder library that produces event traces for Tracealyzer. This shows a timeline of the FreeRTOS scheduling and API calls, and also allows for custom logging from the application code. If you don't already have Tracealyzer installed, you can download it for evaluation at https://percepio.com.

### Viewing crash dumps with GDB/CrashDebug

Copy the crash_debug.bat or crash_debug.sh script from the <dispatcher>/template directory and update the "File Mapping" entry for "dmp" so that it is called with the right arguments. For example:
- Extension: dmp
- Executable: C:\DevAlertDispatcher\crash_debug.bat
- Startup folder: C:\DevAlertDispatcher
- Parameters: /path/to/firmware-${revision}.elf ${file} --gdb
 
In general, CrashDebug is called via the GDB client, in the following way:

    arm-none-eabi-gdb main.elf -ex "set target-charset ASCII" -ex "target remote | CrashDebug --elf main.elf

But Dispatcher include scripts that take care of this for you, crash_debug.bat (for Windows) and crash_debug.sh (for Linux). 

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

Note that this assumes that the elf and dmp files can be found by loadcrashdump.cfg. This example uses a hardcoded folder for this, where the trace and dmp files from Dispatcher are copied by slightly a modified version of the crashdebug script. For example by adding:

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

## License and Copyright

The DFM and TraceRecorder libraries are copyright (c) Percepio AB 2023 (https://percepio.com) and provided under the Apache 2.0 license.

The CrashCatcher library is copyright (c) Adam Green (https://github.com/adamgreen) and provided under the Apache 2.0 license.

The demo application is based on the AWS FreeRTOS demo for the STM32L4 IoT Discovery Kit, provided under the MIT license. 
