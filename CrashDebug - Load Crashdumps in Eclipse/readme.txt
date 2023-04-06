This is not yet a complete instruction, mainly some notes to improve further on. If you need help to configure this, please contact support@percepio.com.

Use the DevAlertDump.launch debug configuration in Eclipse to load the latest crashdump downloaded via DevAlert Dispatcher.
These files used to be placed in a folder named C:\DevAlertDispatcher so the easiet option is to do the same.
Otherwise you need to update those paths. (This is just an example, may need more polishing)

This assumes that DevAlert Dispatcher maps the dmp files to the crash_debug.bat script, that launches CrashDebug.exe via gdb.
The bat file copies the latest viewed elf file and dumpfile into a hardcoded folder, which is referenced from DevAlertDump.launch.
