# DevAlertDemoSTM32L475

This project demonstrates Percepio DevAlert (https://percepio.com/devalert) on an STM32L475 device. Currently this is still a work in progress, but is working. The instructions for viewing crash dumps are however not yet complete.

The development tools used is SW4STM32 (https://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32) but it should be possible to import the project into STM32CubeIDE.

This demo includes the DevAlert target library (DFM) extended with the CrashCatcher library, and as well as the TraceRecorder library for Tracealyzer. These are found under \libraries\3rdparty.

Examples of how to use the DFM library is found in main.c (vendors\st\boards\stm32l475_discovery\aws_demos\application_code\main.c)





