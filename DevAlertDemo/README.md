**DFM**: The DevAlert client library. This provides an API for generating "alerts" (errors/warnings) that typically include "payloads" (debug data like core dumps and traces).

**TraceRecorder**: The tracing library for Percepio Tracealyzer. This is configured to use the "RingBuffer" stream port, that keeps the most recent trace data in memory until requested. On hard faults and other alerts, the CrashCatcher integration code in DFM adds the trace data as a payload in the alert. See DFM/dfmCrashCatcher.c.

**CrashCatcher**: Provides core dumps for GDB. This is based on Adam Green's [CrashCatcher](https://github.com/adamgreen/CrashCatcher) library.

Configuration header files are found in config subfolders.
