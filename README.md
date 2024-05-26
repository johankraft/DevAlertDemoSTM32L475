# Percepio DevAlert on the STM32L4 IoT Discovery Kit
[Percepio DevAlert](https://percepio.com/devalert) is a cloud-based observability solution for edge devices and embedded software,
that lets you detect and analyze issues remotely. Both during system testing and in deployed customer use. 
If your device software would encounter an unexpected situation or misbehave for other reasons, you will be notified right away
and get comprehensive diagnostic information to quickly solve the problem.

This project demonstrates DevAlert v2.0 on the STM32L4 IoT Discovery kit using STM32CubeIDE, including support for GDB core dump debugging and Tracealyzer traces.

More information about how to run this demo is found in the [DevAlert Getting Started Guide](https://percepio.com/getstarted/devalert-guide.pdf). When using this demo, only steps 1-13 are needed. Steps 14-16 are already implemented in the demo code.

Learn more about the STM32L4 IoT Discovery kit at https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html. 

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

## License and Copyright

The DFM and TraceRecorder libraries are copyright (c) Percepio AB 2023 (https://percepio.com) and provided under the Apache 2.0 license.

The CrashCatcher library is copyright (c) Adam Green (https://github.com/adamgreen) and provided under the Apache 2.0 license.

The demo application is based on the AWS FreeRTOS demo for the STM32L4 IoT Discovery Kit, provided under the MIT license. 
