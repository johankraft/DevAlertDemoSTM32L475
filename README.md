# Percepio DevAlert on the STM32L4 IoT Discovery Kit
[Percepio DevAlert](https://percepio.com/devalert) is a cloud-based observability solution for edge devices and embedded software,
that lets you detect and analyze issues remotely. Both during system testing and in deployed customer use. 
If your device software would encounter an unexpected situation or misbehave for other reasons, you will be notified right away
and get comprehensive diagnostic information to quickly solve the problem.

This project demonstrates DevAlert v2.0 on the [STM32L4 IoT Discovery](https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html) kit using STM32CubeIDE, including support for GDB core dump debugging and Tracealyzer traces.

More information about how to run this demo is found in the [DevAlert Getting Started Guide](https://percepio.com/getstarted/devalert-guide.pdf). When using this demo, only steps 1-13 are needed. These are fairly quick and easy. The firmware integration (steps 14-16) is already implemented in this demo.

## Loading the demo project

Import the project files in STM32CubeIDE using the following steps:

- Clone or copy this repository from Github to a suitable location on your computer.

- In STM32CubeIDE, select "File" -> "Import..." -> "General" -> "Projects from Folder or Archive".

- In the Import window, click on "Directory..." and select the project found.
	
- Select "Finish" and the project will be loaded.

## Two Build Configurations

The demo project has two "build configurations" for different the two demos/setups included.
The build configuration is selected under "Project" -> "Build Configurations" -> "Set Active". 

**Debug-Serial**
In this mode, the alerts are written to the debug console (UART) as binary hex strings.
The data is received by a host tool and relayed to your DevAlert evaluation account, as described in the [DevAlert Getting Started Guide](https://percepio.com/devalert/gettingstarted/). 

**Debug-WifiAWS**
This mode uploads the alerts to AWS IoT Core using MQTT over Wi-Fi. This is not yet covered in the Getting Started guide. 

Please contact support@percepio.com if you wish to evaluate this.

## Two Core Dump Formats

This project has been extended to support two core dump formats, both CrashCatcher and the Zephyr core dump format, here called "zdump".
The Zephyr core dump format can be used with any RTOS and supports several processor types. CrashCatcher only supports Arm Cortex-M.

To select what core dump format to use, use the DEMO_CFG_USE_ZEPHYR_CORE_DUMP_FORMAT setting in config_files/dfm_demo_config.h.

- If set to 0, the CrashCatcher core dump format is used. To view such core dumps, see the [DevAlert Getting Started Guide](https://percepio.com/devalert/gettingstarted/). 

- If set to 1, the core dumps are generated using the Zephyr core dump format, intended for the coredump_gdbserver.py tool.

To view Zephyr-formatted core dumps, follow these steps. This guide assumes Dispatcher runs on Windows and the core dump viewer runs in WSL2 (tested on Ubuntu22).
If using Linux, you need to adjust the Dispatcher File Mapping accordingly (call zdump.sh directly, not wsl.exe). 

- On the WSL/Linux machine:

    - Make sure you have python3 installed.

    - Install the pyelftools package using "pip install pyelftools"

    - Install arm-none-eabi-gdb, for example using the example steps provided below:
 
	cd opt
	sudo wget "https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz"
	sudo tar -xf ./arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
	sudo rm arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
	sudo apt-get install libncurses5 libncursesw5
	export PATH="/opt/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin:$PATH"

    - Verify the GDB installation:

      arm-none-eabi-gdb -v

    - Copy the zdump-test folder from this repository to your WSL/Linux home folder, such that zdump.sh is found at ~/zdump-test/zdump.sh.

Using zdump.sh

The main script is zdump.sh, provided in the zdump-test folder. This script shows a summary report of the core dump, by running a set of gdb commands specified in gdb-commands.txt (edit these to tweak the report). It requires two arguments, arg 1 should be the ELF file (aws_demos.elf) and arg2 should be the core dump file (zcoredump.bin). 

Internally, this script works by first starting coredump_gdbserver.py, that works as a GDB server for viewing core dumps. Then it starts the gdb client (arm-none-eabi-gdb) and connects to coredump_gdbserver.py. Finally it runs gdb-commands.txt as a command file.

Follow the [DevAlert Getting Started Guide](https://percepio.com/devalert/gettingstarted/) unil you see a "zcoredump.bin" payload in the DevAlert dashboard.
Clicking the link will download the file using the Dispatcher tool. To make Dispatcher understand how to view the file, you need to create a File Mapping that calls zdump.sh.
Here is how to create a Dispatcher File Mapping for zdump.sh, assuming Dispatcher runs in Windows and zdump.sh in WSL:

- Description: zcoredump (or what you prefer to call it)

- Exact Filename: zcoredump.bin (hardcoded in the STM32 demo code)

- Executable: C:\Program Files\WSL\wsl.exe (adjust this path to match your system)

- Startup folder: (empty)

- Parameters: ~ zdump-test/zdump.sh elf-path '${file}' (elf-path should point to aws_demos.elf)

Notes: 
- The ELF file (aws_demos.elf) is found in the STM32CubeIDE project build folder (Debug-Serial). Copy this manually if needed. 
- Updates are currently in progress to simplify the DevAlert host tools configuration. More information will follow when this is ready.

## ELF File Management

This demo uses gdb for viewing core dumps, and the gdb tool needs the elf file from the build.
When setting up the Dispatcher configuration for gdb, the easiest solution is provide a hard-coded path to the elf file in your build directory.
Obviously, this is not proposed for production use, but can work as a first test on a development build.

However, if making changes to the STM32CubeIDE demo, you will soon notice that older core dumps are not displayed properly
since the ELF file has been updated and no longer matches the core dumps from earlier builds. 

To simplify ELF file management for development builds, a tool is included that archives a copy of the elf file. This can be called on each build, or before running a test session. 

The elf file copies are given unique names based on the gcc build id, and this is also included with each alert as metadata.
This enables the Dispatcher tool to automatically locate the right elf file in the "archive", even if the original file has been lost.

To use this solution:

1. Make sure python is installed and accessable from the command line.

2. Create an archive directory for your elf files. This can be a local folder, but you may also use a shared network drive or a Dropbox folder if you want to access the ELF archive on different computers. 

3. Archive the elf file from your build by running:

    python store-elf.py new-elf-file storage-directory

4. Use 'storage-directory/${revision}.elf' in the Dispatcher "File Mapping" settings when configuring tools that require the ELF file.

Note: The ${revision} field in Dispatcher gives the DFM_CFG_FIRMWARE_REVISION setting from dfmConfig.h, that is configured to give the gcc build id.
The gcc build id is also used in store-elf.py to name the archived elf file.

To replicate this solution on your own project, you need the following:

- Instruct the gcc linker to include the build id. This using the linker flag "-Wl,--build-id".

- Update your linker file with a .gnu_build_id section, like in STM32L475VGTx_FLASH.ld in the demo project root folder.
  Keep the same symbol name, since this is used in the vDfmSetGCCBuildID function (see below).

- The function vDfmSetGCCBuildID is used to read the build ID and format it as a hexadecimal string. 
  Store the result in a string buffer (at least 42 chars long) and define DFM_CFG_FIRMWARE_REVISION to this string (in dfmConfig.h).

## Building and Running the Demo

First, make sure you have selected what build configuration to use under "Project" -> "Build Configurations" -> "Set Active".

- Select "Project" -> "Build All".

- Right-click on the "Debug" symbol and select the right Debug Configuration (Serial or AWS_MQTT).

- Select "Debug" to start execution

- Open a terminal program on the right COM port to view the output. The baud rate is 115200. 
  You should see something like:

    ------ Starting up DevAlert demo ------

    1 7 [Tmr Svc] Firmware revision: _DevBuild_Serial

    2 12 [Tmr Svc] Upload method: Serial (upload via host computer)	

The demo has intentional errors that are captured and reported using DevAlert.
Press the blue button on the board to execute one of the test cases.

## DevAlert client code
An overview of the DevAlert client and related libraries is found in in the [DevAlertDemo readme file](https://github.com/johankraft/DevAlertDemoSTM32L475/blob/main/DevAlertDemo/README.md). 

## License and Copyright

The DFM and TraceRecorder libraries are copyright (c) Percepio AB 2023 (https://percepio.com) and provided under the Apache 2.0 license.

The CrashCatcher library is copyright (c) Adam Green (https://github.com/adamgreen) and provided under the Apache 2.0 license.

The demo application is based on the AWS FreeRTOS demo for the STM32L4 IoT Discovery Kit, provided under the MIT license. 
