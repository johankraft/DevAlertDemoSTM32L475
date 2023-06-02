@echo off

rem This is an example of how firmware image files (ELF) files can be resolved in DevAlert. 
rem
rem Background
rem
rem DevAlert relies analysis tools like Tracealyzer and GDB, that runs on your local computer
rem and may need the ELF file in order to analyze the provided data, like traces and core dumps. 
rem 
rem For security reasons, DevAlert doesn't store ELF files inside the DevAlert service. 
rem The user can keep the ELF files in private storage at all times, as there is no need to upload them.
rem Instead, when you click on DevAlert dashboard links, the Dispatcher tool is started on your local computer
rem and it received information like the firmware revision number of the alert.
rem This allows Dispatcher to fetch the right ELF file by calling a customer-specific script. 
rem This is a Windows example of such a script, that you can modify to suit your needs. 
rem
rem The Revision Attribute and File Access
rem 
rem The revision (version) attribute is part of the alert metadata and is specified in the DevAlert device client library (DFM).
rem This is included within each alert and is visible under "Details" in the DevAlert dashboard. 
rem The revision field is a string and doesn't need to be strictly numerial. 
rem See DFM_CFG_FIRMWARE_VERSION in dfmConfig.h. 
rem
rem It is assumed that your firmware images are accessible from your local computer, either on the local file system, 
rem on mounted file server or in cloud storage that can be accessed using a command-line tool.
rem
rem For example, if your ELF file is stored in /files/releases/fw-image/<revision>/image.elf and the current revision is v1.2.3, 
rem you set DFM_CFG_FIRMWARE_VERSION to "v1.2.3" in dfmConfig.h before you build and run/deploy the software. 
rem Then use a script similar to this example to produce a full path for accessing the ELF file from your local computer. 
rem Like /files/releases/fw-image/v1.2.3/image.elf
rem
rem A similar solution can be used if the ELF file is stored in the cloud, although you need a tool to download the data.
rem If using AWS you can use the AWS CLI tools for this purpose, otherwise rclone supports many cloud storage solutions
rem (see https://rclone.org/) including Microsoft Azure.
rem
rem This Example Script
rem
rem In this example, we use the ELF path to copy the ELF file to a known location (a hardcoded folder), from where it can be loaded by analysis like GDB or 
rem Tracealyzer together with the core dump. 
rem 
rem By default, the Dispatcher scripts start a stand-alone GDB client to show core dumps, but core dumps can also be loaded into an IDE debugger, 
rem like Eclipse CDT. This is simplified by storing the latest files in a known folder, i.e., so they can be referenced from the GDB script.
rem To learn more about IDE debugging of core dumps, see readme.txt.
rem
rem To use this script, you add it as a "script" in the DevAlertDispatcher configuration.
rem The following setup will make Dispatcher call the script on every alert payload (file) that is downloaded:
rem 
rem  1. Review the folder paths in the script and update to match your folder structure.
rem
rem  2. Start Dispatcher manually.
rem
rem  3. Make sure the "Backend provider" has been configured. If using a DevAlert evaluation account, 
rem     make sure you have select DevAlert Evaluation and added your account credentials (email and password).
rem 
rem  4. Select "Change Dispatcher Settings" (lower button).
rem
rem  5. Select the "Scrips" tab and add a new entry with the following information:
rem 
rem    - Executable: /your_path_to/fetch_elf_file.bat
rem
rem    - Startup folder: C:/DevAlertDispatcher (Dispatcher directory)
rem
rem    - Parameters: ${revision} ${file}
rem      - Argument 1, ${revision}: the revision field from the downloaded alert
rem      - Argument 2, ${file}: the filename of the downloaded file
rem 
rem    - Use "catch all" to trigger the script on all types of payloads. 
rem
rem  6. Click on a payload link in the DevAlert dashboard (e.g. crash.dmp) and ensure the Dispatcher window opens.
rem     The script will run once the data has been downloaded. Verify that the destination folder has been updated with the new files.
rem     If the Dispatcher window doesn't open, make sure Dispatcher has been registered as a "protocol handler" in your web broswer. 
rem     This can be checked under General Settings -> Download Link Status in the Dispatcher Settings. It should say "Handling Percepio links from DevAlert".
rem     This is normally an automatic registration when starting Dispatcher the first time, but you can use the button "Enable Download Links" to try again. 
rem
rem If the firmware revision is "v1.0-LatestDevBuild", this script will use the ELF file from the build folder.
rem Otherwise, it is assumed that the ELF file is found in ./elf-files/${revision}.elf
rem
rem Notes and Details
rem
rem This is an EXAMPLE script. It doesn't work out of the box due to hardcoded paths.
rem You need to review it so you understand what it does, and update the hard-coded paths to match your folders. 
rem 
rem There is another example of a "fetch" script found in /templates/pre_script.bat (windows) and pre_script.sh (linux). 
rem Those are only "skeletons", but can be used as starting point.
rem
rem Note that also Tracealyzer needs the ELF file, at least when when "compact log" features are used. 
rem This capability, added in Tracealyzer v4.7, allows for more efficient tracing more efficient logging constant strings (e.g. object names) by their address only
rem and the Tracealyzer application then resolves the string using the ELF file, instead of storing each string character in the trace.
rem This requires that Tracealyzer knows where your ELF file is stored. 
rem If using Tracealyzer 4.7, set the environment variable TZ_CFG_PATH to the output folder of this script, e.g. "C:/DevAlertDispatcher/latestcrashdump/".
rem If using Tracealyzer v4.8.0 or later, you can instead add the folder to your Tracealyzer settings (File -> Settings -> Project Settings -> Definition File Paths) 
rem
rem If you have any questions, please contact support@percepio.com.
rem
rem Copyright(c) Percepio AB, 2023

rem Argument 1, revision
set REV=%1%

rem Argument 2, the original location of payload file (created by Dispatcher)
set DMP_SRC=%2%

rem REVIEW AND UPDATE THE FOUR PATHS BELOW

rem Where to copy the files (a known location). These are temporary files, overwritten on next Dispatcher download.
set DMP_DEST=C:/DevAlertDispatcher/latestcrashdump/latest.dmp
set ELF_DEST=C:/DevAlertDispatcher/latestcrashdump/aws_demos.elf

rem By default, we use an archived ELF file (e.g. an official release). 
set ELF_SRC=C:/DevAlertDispatcher/elf-files/%REV%.elf

rem If the alert revision is "v1.0-LatestDevBuild", we instead use the latest build, i.e. the ELF file from the local build folder.
if %REV%==v1.0-LatestDevBuild (set ELF_SRC=C:/src/DevAlertDemoSTM32L475-2023/projects/st/stm32l475_discovery/ac6/aws_demos/Debug-SerialOnly/aws_demos.elf)

echo Firmware revision according to Alert: %REV%
echo Using ELF file: %ELF_SRC%
echo Copying ELF to: %ELF_DEST%
echo Using DMP file: %DMP_SRC%
echo Copying DMP to: %DMP_DEST%

rem Copy ELF file
cp %ELF_SRC% %ELF_DEST%

rem Copy alert payload file (e.g. a core dump)
cp %DMP_SRC% %DMP_DEST%
