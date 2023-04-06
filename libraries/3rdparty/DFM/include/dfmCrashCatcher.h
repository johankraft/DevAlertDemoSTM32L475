/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief DFM Crash Catcher integration defines
 */

#ifndef DFM_CRASHCATCHER_H
#define DFM_CRASHCATCHER_H



#include "FreeRTOS.h"
#include "stddef.h"


#define CRASH_DUMP_NAME         "crash.dmp"



/**
 * @brief If this is set to 1 ít will attempt to also save a trace with the Alert. This requires the Percepio Trace Recorder to also be included in the project.
 */
#define CRASH_ADD_TRACE	(1)

/**
 * @brief Should call a function that reboots device
 * Example: #define CRASH_STRATEGY_RESET() HAL_NVIC_SystemReset()
 */
#define CRASH_STRATEGY_RESET() HAL_NVIC_SystemReset()

/**
 * @brief Endless loop
 * Example: #define CRASH_STRATEGY_LOOP() while(1)
 */
#define CRASH_STRATEGY_LOOP() while(1)

/**
 * @brief Strategy to use after crash has been handled
 * Values: CRASH_STRATEGY_RESET() or CRASH_STRATEGY_LOOP()
 */
#define CRASH_FINALIZE() configPRINT_STRING("DFM: Restarting...\n\n\n"); CRASH_STRATEGY_RESET()


/* CRASH_STACK_CAPTURE_SIZE
 * The number of bytes from the current stack to capture, relative to the stack pointer.
 * The capture is from SP to SP + CRASH_PSP_CAPTURE_SIZE, so only the most recent stack
 * frames are included. Since relative to the current stack pointer, you don't need to
 * specify a stack memory range manually.
 * */
#include "dfmConfig.h" // For accessing the Demo settings
#define CRASH_STACK_CAPTURE_SIZE DFM_DEMO_STACKDUMP_SIZE

/* Additional memory ranges to include in the crash dump (e.g. heap memory) */
#define CRASH_MEM_REGION1_START	0xFFFFFFFF /* 0xFFFFFFFF = not used */
#define CRASH_MEM_REGION1_SIZE	0

#define CRASH_MEM_REGION2_START	0xFFFFFFFF /* 0xFFFFFFFF = not used */
#define CRASH_MEM_REGION2_SIZE	0

#define CRASH_MEM_REGION3_START	0xFFFFFFFF /* 0xFFFFFFFF = not used */
#define CRASH_MEM_REGION3_SIZE	0

/* CRASH_DUMP_MAX_SIZE - The size of the temporary RAM buffer for crash dumps.
 * Any attempt to write outside this buffer will be caught in CrashCatcher_DumpMemory() and give an error.
 * Enable DFM_DEBUG_LOG to see the actual usage.
 * 240 is the
 * */
#define CRASH_DUMP_BUFFER_SIZE (300 + (CRASH_STACK_CAPTURE_SIZE) + (CRASH_MEM_REGION1_SIZE) + (CRASH_MEM_REGION2_SIZE) + (CRASH_MEM_REGION3_SIZE))

typedef struct{
	int alertType;
	char* message;
} dfmTrapInfo_t;

extern dfmTrapInfo_t dfmTrapInfo;

// Arm Cortex-M specific (but crashcatsher is too..)
#define DFM_TRIGGER_NMI() SCB->ICSR |= SCB_ICSR_NMIPENDSET_Msk;

#define DFM_TRAP(type, msg) { dfmTrapInfo.alertType = type; dfmTrapInfo.message = msg; DFM_TRIGGER_NMI(); }

#endif
