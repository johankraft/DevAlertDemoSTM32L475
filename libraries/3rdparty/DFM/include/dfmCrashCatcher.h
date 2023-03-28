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

#define CRASH_DUMP_NAME         "crash.dmp"
/* CRASH_DUMP_MAX_SIZE - The maximum size the crash dump. */
#define CRASH_DUMP_MAX_SIZE     5000

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
#define CRASH_FINALIZE() CRASH_STRATEGY_RESET()

#define CRASH_MEM_REGION1_START	0x10002000
#define CRASH_MEM_REGION1_SIZE	0x800
#define CRASH_MEM_REGION2_START	0x20017800
#define CRASH_MEM_REGION2_SIZE	0x200
#define CRASH_MEM_REGION3_START	0
#define CRASH_MEM_REGION3_SIZE	0

#if ((CRASH_MEM_REGION1_SIZE) + (CRASH_MEM_REGION2_SIZE) + (CRASH_MEM_REGION3_SIZE)) > (CRASH_DUMP_MAX_SIZE)
#error "Dumped memory regions are larger than crash buffer!"
#endif

#endif
