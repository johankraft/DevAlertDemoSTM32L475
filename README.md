# Percepio DevAlert Demonstration on the STM32L4 IoT Discovery Kit

This project demonstrates Percepio DevAlert on the STM32L4 IoT Discovery kit.

DevAlert is an observability solution for IoT/Edge devices providing insight on errors and anomalies in the device software. By adding a small set of "alerts" at strategic locations in your code, like demonstrated in this project, software issues and anomalies can be captured, e.g., hardware fault exceptions, failed ASSERT checks and stack corruptions from buffer overruns. The alerts provide detailed diagnostic information such as core dumps and system traces. You can create your own alert types and include any diagnostic data of interest.

All reported issues can be overviewed in the DevAlert Dashboard and from here you can also access the diagnostic data.

![gdb view](https://percepio.com/github_images/DevAlert-Overview.png)

Out-of-the-box this provides core dumps for your Arm Cortex-M gdb debugger and system traces for Percepio Tracealyzer. You may also integrate additional diagnostic payloads and analysis tools as needed.

The payloads are not stored inside the DevAlert service but in a separate storage backend that can be managed by the user. 

## Using DevAlert in Development and Testing

By using DevAlert during software development and testing, debugging data is captured automatically on unexpected issues like crashes and intermittent errors. 
This way, you always have detailed information available, and easily accessible in the web browser for all stakeholders. 
This can speed up debugging significantly and avoids costly project delays. 

Using DevAlert in this way only requires a local connection (e.g. a UART/USB serial connection) to a computer with internet connection. This is demonstrated by using the Virtual COM port on the onboard STLINK debugger.

## Using DevAlert in Deployment

Once your software is ready for release, you can keep DevAlert enabled in deployment. This allows for detecting cybersecurity-related anomalies, such as stack corruption from buffer overruns. The provided core dumps can tell you if the anomaly was a cyberattack or an accidental error. Having DevAlert enabled in the field also lets you detect and debug any remaning issues in the production code, within minutes after the very first occurence. This without needing a physical debug port in the shipped devices or having to ask the users for additional information.

Using DevAlert in deployment works best if you have some kind of connectivity in the device. This way, you can get instant notification. This demonstrated here by connecting to AWS IoT Core via Wi-Fi.

Alerts can also be stored on the device for later retrieval, e,g, by connecting to the device via bluetooth using a smartphone app with DevAlert integration. That use-case is not demonstrated in this demo, but is certainly doable.

Read more at https://percepio.com/devalert. 

Learn more about the STM32L4 IoT Discovery kit at https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html. 

# DevAlert Solution Overview

The following illustration shows an overview of the DevAlert solution when using the DevAlert evaluation account. Here a local device is connected using a serial port/UART and the host computer takes care of the upload using the devalerthttps tool. The file names reflect the Windows versions, but the desktop tools are also available for Linux.

![DevAlert overview and desktop tools for evaluation account](https://percepio.com/github_images/DA-tools-overview.png)

This may appear complicated, but it is very automated and easy to use. You simply log in to the dashboard at https://devalert.io, click on a payload (core dump or trace) and the data is displayed in a local tool within a few seconds. 

However, to configure the desktop tools you need to understand a bit about how it works. This involves four parts:

1. The device library (DFM) that emit "alerts" when called from the application code. An alert provides diagnostic payloads such as core dumps and traces. This is demonstrated in this project as explained in [The Demo Code](#The-Demo-Code) with links to important parts of the code.

2. The DevAlert backend storage, where the uploaded alerts are stored. The evaluation accounts include Percepio-hosted storage to make it easier to get started. Production accounts can be configured to use your own AWS account as backend for maximum privacy and data control. 

3. The download from the backend storage to your local computer is provided by the DevAlert Dispatcher tool, as explained in [Analyzing Alerts from the DevAlert Dashboard](#Analyzing-Alerts-from-the-DevAlert-Dashboard). This allows for using your own backend, seperated from the DevAlert service. Dispatcher can also invoke custom scripts when activated. An example script is provided ("fetch_elf_file.bat") to fetch the right ELF file based on the software revision of the alert.

4. Alerts can be uploaded to the backend storage in different ways. This demo project demonstrates two methods, [direct upload using AWS IoT Core/MQTT](#Uploading-via-AWS-IoT-Core) and indirect upload using a serial connection together with the [DevAlert upload tools](#Uploading-via-a-Serial-Connection). The latter allows for using DevAlert also on device lacking cloud connectivity. This is also the easiest way to get started with DevAlert for evaluation.

### DevAlert with AWS IoT Core

![DevAlert overview and desktop tools for aws](https://percepio.com/github_images/DA-tools-overview-aws.png)

When integrating DevAlert with an AWS account, the alerts are stored in your own Amazon S3 bucket. They are uploaded either directly from the device via AWS IoT Core (MQTT) or using the desktop client devalerts3. 

## Loading the demo project

This demo project is for the SW4STM32 development tools. If you don't already have it installed, get it from https://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32.

Import the project files using the following steps:

- Clone or copy this repository from Github to a suitable location on your computer.

- In SW4STM32, select "File" -> "Import..." -> "General" -> "Projects from Folder or Archive".

- In the Import window, click on "Directory..." and select the project in the following folder:

	/projects/st/stm32l475_discovery/ac6/aws_demos
	
- Select "Finish" and the project will be loaded.

## Build Configurations

This demo contains two "build configurations" demonstrating two different methods for uploading the DevAlert data:
The build configuration is selected under "Project" -> "Build Configurations" -> "Set Active". 
You should see the following options:

- "Debug-SerialOnly" uploads to your DevAlert evaluation account via the host computer, using the Virtual COM port on the onboard STLINK debugger.
  This is easy to get started with for evaluation and a similar setup can also be used for test monitoring.

- "Debug" uploads directly to AWS IoT Core using a Wi-Fi network. This is more representative for deployed operation. 
  But it requires a bit more configuration, including an AWS account configured for DevAlert and a full DevAlert account with AWS support. 
  This is provided on request by contacting support@percepio.com.
  
You probably want to start with "Debug-SerialOnly" to begin with.

## Building and Running the Demo

First, make sure you have selected what build configuration to use under "Project" -> "Build Configurations" -> "Set Active".

Next, we start a debug session. This will also build the project.

- Right-click on the "aws_demos" project and selecting "Debug as" -> "Ac6 STM32 C/C++ Application".
  This will start the debug session, with the execution halted in main. 

- Select "Resume (F8)" to start execution (the green triangle button)

- Open a terminal program on the right COM port to view the output. You should see something like:

	------ Starting up DevAlert demo ------

	1 9 [Tmr Svc] Firmware revision: v1.0-LatestDevBuild

	2 14 [Tmr Svc] Upload method: Serial (upload via host computer)

	3 22 [DemoTask1] Provoking random errors...
	
	4 2027 [DemoTask1] Test case: Buffer overrun
	
The demo has four intentional errors that are captured and reported using DevAlert.
These are selected randomly and the device will restart after each reported error.

To see these alerts in DevAlert and download the diagnostic payload, like core dumps and traces, you need to configure the data upload.

## Uploading via a Serial Connection

This method is easiest to get started with and doesn't require any AWS account or cloud-side configuration. 
The only things needed are:

- Your DevAlert evaluation account. If you don't have this already, sign up at https://devalert.io/auth/signup and you get the account immidiatly.
- A terminal program like Teraterm
- Python installed

The rest is provided in this repository.

In this configuration, the DFM library outputs the data as hex strings to the serial VCOM interface.
The data is received using a terminal program (together with other output) and logged to a text file.
The text file is then processed by the DevAlert tools and the data is uploaded to the cloud.
This can run continously and upload multiple alerts from the device over a long time period, if needed.

For the terminal program we mostly use Teraterm, but any terminal program with a logging capability should work. 

On the host computer, the cloud upload is done using one of the following tools:
  
- devalerthttps - uploads to your DevAlert evaluation account (hosted by Percepio)
  
- devalerts3 - uploads to your Amazon S3 bucket (hosted by the device builder)
	
In this demo, we use the devalerthttps tool for the upload. This is not intended for direct upload from devices, but is easier to get started with.
To use it, follow the following steps:

1. Install Python if you don't already have it.

2. Run devalerthttps to enter your DevAlert credentials. These are saved for future sessions.

	devalerthttps set-username --username email@domain.com
	
	devalerthttps set-password
	Password: 

Note that this username/password authentication is only used for DevAlert evaluation accounts, to simplify evaluation.
Production accounts use mutual TLS authentication for receiving alert data and only "metadata" is uploaded in such configurations.
The diagnostic payloads like core dumps and traces then remain on the customer side, in your own storage.

3.	Close any terminal programs that are already connected to the same COM port.

	If using Windows and have Teraterm installed:
	
		- Copy the DA-tools directory to a suitable location.
	
		- Make sure your terminal program is in your PATH (e.g. ttermpro.exe) so it can be called from the command line.
	
		- Run the da-serial-eval script (.bat) with the COM port provided as argument. This will start Teraterm on COM9 and upload the data using the devalerthttps tool.
	
			devalertserial-sandbox.bat 9	
	
	If using Linux or prefer another terminal program, make a custom script similar to da-serial-eval.bat with the following steps:

		- Start a terminal program on the right COM port, logging the output to a text file ("MYFILE" below).
	
		- Run the following:
		
			python devalertserial.py --file MYFILE --upload sandbox | devalerthttps.exe store-trace

	
## Uploading via AWS IoT Core

This method requires a Wi-Fi network, an AWS account that has been set up for DevAlert, and a full DevAlert account with AWS support (provided on request).
Contact support@percepio.com if you need help to set this up.

Befure building the demo project, make sure to select the build configuration "Debug". This will include the AWS_MQTT cloud port (DFM/kernelports/FreeRTOS/cloudports/AWS_MQTT) and set the build flag DFM_CFG_SERIAL_UPLOAD_ONLY=0 which enables the Wi-Fi code.

To connect to your Wi-Fi network and to AWS IoT Core in your AWS account, you need to update aws_clientcredential.h and aws_clientcredential_keys.h (see \demos\include) as described on https://docs.aws.amazon.com/freertos/latest/userguide/freertos-prereqs.html.

## Analyzing Alerts from the DevAlert Dashboard

The alerts include diagnostic payloads such as traces and core dumps, that explain what happened. 
They are downloaded via the DevAlert Dispatcher tool, that needs to be installed locally. This is available at https://devalert.io/dispatcher. 
Once this is installed and configured, you only need to click on the "payload" links in the DevAlert dashboard. 

![DevAlert Dashboard and Dispatcher](https://percepio.com/github_images/DevAlert-Dashboard-Dispatcher.png)

The Dispatcher tool has two jobs. First, it download the selected payload from the storage you have configured.
If the payload has been divided into multiple pieced by the DFM library, the Dispatcher tool also recombined the pieces into a single file.
Finally it launches the right tool, typically with the payload file and ELF file as argument.

The first time, you need to start Dispatcher manually. You should see the following window.

![DevAlert Dispatcher Configuration](https://percepio.com/github_images/DevAlert-Dispatcher-Main.png)

Click on "Provider Settings Wizard..." and configure it as described at https://devalert.io/dispatcher/setup.
When using a DevAlert evaluation account, you only need to enter the following information:

1. Your DevAlert account credentials (username and password) for the DevAlert service. Check the "Remember password" option to avoid having to repeat the password every time.

2. Select "DevAlert Evaluation" as Backend Provider. This selects the Percepio-hosted evaluation account storage. 

3. Enter your DevAlert account credentials (username and password) again for the Backend (storage). This is technically separate from the DevAlert account.

Note that Dispatcher will register itself as a protocol handler in your standard web browser to handle "percepio://" links. 
This starts it when clicking the DevAlert download links.

### Providing the ELF file

DevAlert relies on desktop tools like GDB and Tracealyzer.
This lets you view the provided payloads on your local computer, instead of processing these rather sensitive files in the cloud.
However, the tools often need the corresponding image (ELF) file. 

For security reasons, DevAlert doesn't process or store your ELF files. They remain in your private storage at all times, e.g. on a local network drive.

Instead, when you click on DevAlert dashboard links, the Dispatcher tool is started on your local computer and 
receives information including the revision (version) of the device software. 
This allows Dispatcher to fetch the right ELF file by calling a user-specific script with the revision as argument.
The script reconstructs the path to the right image file, for example

	/files/releases/fw-image/<revision>/image.elf

The script "fetch_elf_file.bat" provides an example of such a script for Windows, that you can modify to suit your needs. 
This also shows how to combine development builds (without official version number) with official release versions.
By using a special revision value the ELF file from your local build directory is used. 

You can add this script to the Dispatcher configuration to run every time you download a payload. 
This will copy the right elf-file to a hardcoded path that can be referenced from GDB scripts and other tools.
To learn more, open the .bat file in a text editor and read the extensive documentation there.

You also find templates for both Windows and Linux in the "template" directory in the Dispatcher install directory.

### Viewing Traces in Tracealyzer

The demo includes the TraceRecorder library that produces event traces for Tracealyzer. 
This shows a timeline of the FreeRTOS scheduling and API calls, and also allows for custom logging from the application code.
If you don't already have Tracealyzer installed, you can download it for evaluation at https://percepio.com/tracealyzer/update/.

The default Dispatcher configuration will start Tracealyzer automatically for payloads ending with .psf, .psfs, .bin and .trc.

You need a license to run Tracealyzer, which is provided by your DevAlert evaluation account.
When starting Tracealyzer for the first time, you will see a Welcome screen with three big buttons.
Select "Activate License", select "Percepio License Service" and enter your DevAlert credentials. 
Select "Rememer password" if you don't want to log in each time you start the tool.

### Viewing Core Dumps from Arm Cortex-M devices

Assuming the DevAlert client library has been properly integrated, core dumps are provided both on fault exceptions and when calling DFM_TRAP().

Core dumps include processor state like registers, stack and other memory contents. They are loaded into gdb using the CrashDebug tool.
CrashDebug is included in the DA-tools directory but is also available at https://github.com/adamgreen/CrashDebug/tree/master/bins.
Note that CrashDebug is only available for Arm Cortex-M devices, but similar DevAlert integrations are possible for other processors.

You also need the right version of gdb for Arm Cortex-M devices, typically arm-none-eabi-gdb. 
If using a tool-chain based on the gcc or clang compilers, you should already have arm-none-eabi-gdb on your computer.
But we also include it in the DA-tools directory for convinience. 

The steps needed to configure Dispatcher for viewing Core Dumps are:

1. Copy arm-none-eabi-gdb to the same directory as Dispatcher.

2. Copy the crash_debug.bat or crash_debug.sh script from the <dispatcher>/template directory to the Dispatcher directory.

3. Verify the "File Mapping" entry for the "dmp" payload type, so the script is called with the right arguments. 
For example:
- Extension: dmp
- Executable: (Dispatcher directory)/crash_debug.bat  // or .sh for Linux
- Startup folder: (Dispatcher directory)
- Parameters: /path/to/firmware-${revision}.elf ${file} --gdb

![DevAlert Dispatcher](https://percepio.com/github_images/DevAlert-Dispatcher.png)

This will start a gdb client with the core dump loaded, like shown below.
In the screenshot we use a hardcoded path make it easier to load the core dump into an IDE, as described in the next section. 
 
![gdb view](https://percepio.com/github_images/DevAlert-gdb.png)


### Viewing Core Dumps in an IDE

You may also load the core dump into your IDE debugger. 

![Eclipse debugger view](https://percepio.com/github_images/DevAlert-Eclipse.png)

To view a core dump, you can configure your IDE debugger to start a gdb session that calls the provided gdb command file.
An example is provided in "loadcoredump.cfg". This only contains the following commands:

	cd C:/DevAlertDispatcher
	file ./latestcrashdump/aws_demos.elf
	target remote | ./CrashDebug.exe --elf ./latestcrashdump/aws_demos.elf --dump ./latestcrashdump/latest.dmp

The "file" command sets the right ELF file.

The "target remote" command loads the core dump using the CrashDebug tool.

The "latestcrashdump" directory is the output of the "fetch_elf_file" script, that fetches the right version of the ELF image file. 

If using an Eclipse-based IDE, this can be implemented as a separate "debug configuration".
For this demo, with SW4STM32, this can be configured in the following way:

1. Create a new debug configuration of the type "GDB Hardware Debugging". Name it e.g. "DevAlert Core Dump".
	
2. In the Debug Configuration editor, enter the following:
- Main / C/C++ Application: the ELF file in the build directory (not used but must be provided)
- Main / Disable auto build: Checked
- Debugger / GDB command: The full path to arm-none-eabi-gdb
- Debugger / Use remote target: Unchecked
- Startup / Initialization commands: "source /path/to/loadcrashdump.cfg"
- Startup / Other things: All unchecked (don't "load image" or "load symbols", since all is handled by loadcrashdump.cfg)

3. Make sure that the "fetch_elf_file" is triggered by Dispatcher and is updating the output directory, e.g. "latestcrashdump".

4. Download a core dump from DevAlert

5. Start the new "DevAlert Core Dump" debug configuration and the core dump should load quickly.

Note that this assumes that the elf and dmp files can be found by loadcrashdump.cfg. 
This example uses a hardcoded folder for this (latestcrashdump), where the files from Dispatcher are copied by the "fetch_elf_file" script.
 
## The Demo Code

The demo project generate alerts via DevAlert in two ways. On fault exceptions and by calling the DFM_TRAP() macro explicitly in the code, for example like:

	DFM_TRAP( DFM_TYPE_STACK_CHK_FAILED, "Stack corruption detected");

DFM_TRAP() is documented at /libraries/3rdparty/DFM/include/dfmCrashCatcher.h#L98
and you find usage examples on several locations in the demo source code, for example at:

- /vendors/st/boards/stm32l475_discovery/aws_demos/application_code/main.c#L1014

- /vendors/st/boards/stm32l475_discovery/aws_demos/config_files/FreeRTOSConfig.h#L139

- /libraries/3rdparty/DFM/dfmCrashCatcher.c#L336

To learn how to set up DevAlert for your own project, see the following code examples:

Initialization
- /vendors/st/boards/stm32l475_discovery/aws_demos/application_code/main.c
- /vendors/st/boards/stm32l475_discovery/aws_demos/config_files/FreeRTOSConfig.h
- /demos/include/aws_clientcredential.h
- /demos/include/aws_clientcredential_keys.h

DevAlert configuration
- /libraries/3rdparty/DFM  -- in particular /config/dfmConfig.h

TraceRecorder configuration
- /libraries/3rdparty/tracerecorder/config  -- in particular /config/trcConfig.h

In FreeRTOSConfig.h, the important parts are:

- #define configASSERT( x ) ... definition and the related #includes (just above)
- #define configUSE_TRACE_FACILITY 1 -- Needed for recording kernel events
- #include "trcRecorder.h" -- Needed for recording kernel events 
 
## Porting / Integration

To port this to another target, you may need to update the following files:
- dfmUser.c - Used-defined definitions needed by DFM.
- dfmStoragePort.c - How to store Alerts to flash (before restarting) - currently using "libraries\3rdparty\DFM\storageports\Dummy\dfmStoragePort.c"
- dfmCloudPort.c - How to upload Alerts to the device backend (e.g. an AWS account). See \libraries\3rdparty\DFM\kernelports\FreeRTOS\cloudports\AWS_MQTT\dfmCloudPort.c
- trcConfig.h - If porting to a different processor architecture, you need to select the right hardware port setting. Currently set to "Arm Cortex-M" matching STM32L4.

## Target Support

The DFM library (the DevAlert device client) is not hardware dependent and can be used on any processor. 
But the core dump support relies on crashcatcher and crashdebug which is only available for Arm Cortex-M devices. 
Officially it only supports ARMv6-M and ARMv7-M processors, but it seems to work fine also on ARMv8-M cores like Cortex-M33, at least with Arm TrustZone disabled. 
As of April 2023, it has not yet been tested with ARM TrustZone. 

## License and Copyright

The DFM and TraceRecorder libraries are copyright (c) Percepio AB 2023 (https://percepio.com) and provided under the Apache 2.0 license.

The CrashCatcher library is copyright (c) Adam Green (https://github.com/adamgreen) and provided under the Apache 2.0 license.

The demo application is based on the AWS FreeRTOS demo for the STM32L4 IoT Discovery Kit, provided under the MIT license. 
