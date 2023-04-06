/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * DFM Crash Catcher integration
 */


#include <FreeRTOS.h>
#include "task.h"
#include <CrashCatcher.h>
#include <string.h>
#include "dfm.h"
#include "dfmCrashCatcher.h"
#include "CrashCatcherPriv.h"

#if ((DFM_CFG_ENABLED) >= 1)

#if ((CRASH_ADD_TRACE) >= 1)
#include "trcRecorder.h"
static void prvAddTracePayload();
#endif


DfmAlertHandle_t xAlertHandle;

uint8_t dfmAlertStarted = 0;

dfmTrapInfo_t dfmTrapInfo = {-1, ""};

TraceStringHandle_t TzUserEventChannel = NULL;

void *__stack_chk_guard = (void *)0xdeadbeef;

static uint8_t* ucBufferPos;
static uint8_t ucDataBuffer[CRASH_DUMP_BUFFER_SIZE] __attribute__ ((aligned (8)));

#if (DFM_DEBUG_LOG == 1)

// used by DFM_DEBUG_PRINTF (right after ucDataBuffer is a good location)
char dfmCrashCatcherPrintBuffer[256];

#endif

static void CrashCatcher_DumpStart_OnFault(const CrashCatcherInfo* pInfo);
static void CrashCatcher_DumpStart_OnTrap(const CrashCatcherInfo* pInfo);

static void dumpHalfWords(const uint16_t* pMemory, size_t elementCount);
static void dumpWords(const uint32_t* pMemory, size_t elementCount);

uint32_t stackPointer = 0;

const CrashCatcherMemoryRegion* CrashCatcher_GetMemoryRegions(void)
{
    static CrashCatcherMemoryRegion regions[] = {
    		{0xFFFFFFFF, 0xFFFFFFFF, CRASH_CATCHER_BYTE},
    		{CRASH_MEM_REGION1_START, CRASH_MEM_REGION1_START + CRASH_MEM_REGION1_SIZE, CRASH_CATCHER_BYTE},
			{CRASH_MEM_REGION2_START, CRASH_MEM_REGION2_START + CRASH_MEM_REGION2_SIZE, CRASH_CATCHER_BYTE},
			{CRASH_MEM_REGION3_START, CRASH_MEM_REGION3_START + CRASH_MEM_REGION3_SIZE, CRASH_CATCHER_BYTE}
    };

	regions[0].startAddress = stackPointer;
    regions[0].endAddress = stackPointer + CRASH_STACK_CAPTURE_SIZE;

    return regions;
}

void CrashCatcher_DumpStart(const CrashCatcherInfo* pInfo)
{

	stackPointer = pInfo->sp;

#if ((CRASH_ADD_TRACE) >= 1)
	if (TzUserEventChannel == NULL)
	{
		xTraceStringRegister("ALERT", &TzUserEventChannel);
	}
#endif

	configPRINT_STRING("\nDFM Alert\n");

	if (dfmTrapInfo.alertType >= 0)
	{
		/* On the DFM_TRAP macro. This sets dfmTrapInfo.alertType and then generates an NMI exception to trigger this error handler. */
		CrashCatcher_DumpStart_OnTrap(pInfo);
	}
    else
    {
    	/* On hardware fault exceptions */
    	CrashCatcher_DumpStart_OnFault(pInfo);
    }
}

// We get a hard fault in CrashCatcher_DumpEnd, hard fault on hard fault...
// Why??? (could be the changes I made in dfmStoragePort.c?) - don't think so, we never get there.
// pxDfmSessionData is 0xa5a5a5a5 at DumpEnd, is the crash dump overflowing the buffer?

static void CrashCatcher_DumpStart_OnFault(const CrashCatcherInfo* pInfo)
{

	configPRINT_STRING("  DFM: Hard fault\n");

	ucBufferPos = &ucDataBuffer[0];
    /* Do not try to send the alert */
    xDfmSessionSetCloudStrategy(DFM_CLOUD_STRATEGY_OFFLINE);
    if (xDfmAlertBegin(DFM_TYPE_HARDFAULT, "Fault Exception", &xAlertHandle) == DFM_SUCCESS)
    {
    	xDfmAlertAddSymptom(xAlertHandle, DFM_SYMPTOM_CURRENT_TASK, simplechecksum32(pcTaskGetName( xTaskGetCurrentTaskHandle() )));
    	xDfmAlertAddSymptom(xAlertHandle, DFM_SYMPTOM_STACKPTR, pInfo->sp);

#if ((CRASH_ADD_TRACE) >= 1)
    	xTracePrintF(TzUserEventChannel, "Fault exception! See crash dump for details.");
        prvAddTracePayload();
#endif
        dfmAlertStarted = 1;
        configPRINT_STRING("  DFM: Storing the alert.\n");

    }else{
    	configPRINT_STRING("  DFM: Not yet initialized. Alert ignored.\n"); // Always log this!
    }
    configPRINT_STRING("\n");

}

static void CrashCatcher_DumpStart_OnTrap(const CrashCatcherInfo* pInfo)
{
	// Always log this!
	configPRINT_STRING("  DFM: Software trap (");
	configPRINT_STRING(dfmTrapInfo.message);
	configPRINT_STRING(")\n");

	ucBufferPos = &ucDataBuffer[0];
    /* Do not try to send the alert */
    xDfmSessionSetCloudStrategy(DFM_CLOUD_STRATEGY_OFFLINE);
    if (xDfmAlertBegin(dfmTrapInfo.alertType, dfmTrapInfo.message, &xAlertHandle) == DFM_SUCCESS)
    {
    	xDfmAlertAddSymptom(xAlertHandle, DFM_SYMPTOM_CURRENT_TASK, simplechecksum32(pcTaskGetName( xTaskGetCurrentTaskHandle() )));
    	xDfmAlertAddSymptom(xAlertHandle, DFM_SYMPTOM_STACKPTR, pInfo->sp);

#if ((CRASH_ADD_TRACE) >= 1)
    	xTracePrintF(TzUserEventChannel, "DFM Trap: \"%s\". See crash dump for details.", dfmTrapInfo.message);
        prvAddTracePayload();
#endif
        dfmAlertStarted = 1;
        configPRINT_STRING("  DFM: Storing the alert.\n");

    }
    else{
    	configPRINT_STRING("  DFM: Not yet initialized. Alert ignored.\n"); // Always log this!
    }
    configPRINT_STRING("\n");

    dfmTrapInfo.alertType = -1;
    dfmTrapInfo.message = NULL;
}

static void prvAddTracePayload()
{
    char* szName;
    void* pvBuffer = (void*)0;
    uint32_t ulBufferSize = 0;
    if (TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)
    {
        szName = "dfm_trace.psfs";
    }
    else if (TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_SNAPSHOT)
    {
        szName = "dfm_trace.trc";
    }

    if (xTraceIsRecorderEnabled() == 1)
    {

        xTraceDisable();
    }
    xTraceGetEventBuffer(&pvBuffer, &ulBufferSize);
    xDfmAlertAddPayload(xAlertHandle, pvBuffer, ulBufferSize, szName);
}

void CrashCatcher_DumpMemory(const void* pvMemory, CrashCatcherElementSizes elementSize, size_t elementCount)
{
	int32_t current_usage = (uint32_t)ucBufferPos - (uint32_t)ucDataBuffer;
	int32_t bytesWritten = 0;

	/* This function is called when CrashCatcher detects an internal stack overflow (it has a separate stack) */
	if (g_crashCatcherStack[0] != CRASH_CATCHER_STACK_SENTINEL)
	{
		// This might actually not print, as the memory has been partially corrupted (e.g. UART address)
		configPRINT_STRING("DFM: ERROR, stack overflow in CrashCatcher, see comment in dfmCrashCatcher.c\n");

		/**********************************************************************************************************

		If you get here, there has been a stack overflow on the CrashCatcher stack.
		This is separate from the main stack and defined in CrashCatcher.c (g_crashCatcherStack).

		This error might happen because of diagnostic prints and other function calls while saving the alert.
		You may increase the stack size in CrashCatcherPriv.h or turn off diagnostic prints (e.g. DFM_DEBUG_LOG).

		***********************************************************************************************************/

		portDISABLE_INTERRUPTS();
		for (;;); // Stop here...
	}

	if (elementCount == 0)
	{
		/* May happen if CRASH_MEM_REGION<X>_SIZE is set to 0 by mistake (e.g. if using 0 instead of 0xFFFFFFFF for CRASH_MEM_REGION<X>_START on unused slots. */
		configPRINT_STRING("DFM: Warning, memory region size is zero, ignoring this!\n");
		return;
	}

	DFM_DEBUG_PRINTF("  Dumping %d bytes to ucDataBuffer. ", (elementCount* elementSize));

    switch (elementSize)
    {

    	case CRASH_CATCHER_BYTE:

			if ( current_usage + elementCount >= CRASH_DUMP_BUFFER_SIZE)
			{
				configPRINT_STRING("\nDFM: Error, ucDataBuffer not large enough!\n\n");
				return;
			}

			memcpy((void*)ucBufferPos, pvMemory, elementCount);
			ucBufferPos += elementCount;
			bytesWritten = elementCount;
			break;

    	case CRASH_CATCHER_HALFWORD:

			if ( current_usage + elementCount*2 >= CRASH_DUMP_BUFFER_SIZE)
			{
				configPRINT_STRING("\nDFM: Error, ucDataBuffer not large enough!\n\n");
				return;
			}
			dumpHalfWords(pvMemory, elementCount);

			bytesWritten = elementCount * 2;
			break;

    	case CRASH_CATCHER_WORD:

			if ( current_usage + elementCount*4 >= CRASH_DUMP_BUFFER_SIZE)
			{
				configPRINT_STRING("\nDFM: Error, ucDataBuffer not large enough!\n\n");
				return;
			}

			dumpWords(pvMemory, elementCount);

			bytesWritten = elementCount * 4;
			break;

    	default:
    		configPRINT_STRING("\nDFM: Error, unhandled case!\n\n");
    		break;
    }

    DFM_DEBUG_PRINTF(" OK. Usage now %d/%d bytes\n", (uint32_t)ucBufferPos - (uint32_t)ucDataBuffer, CRASH_DUMP_BUFFER_SIZE);
}

static void dumpHalfWords(const uint16_t* pMemory, size_t elementCount)
{
    size_t i;
    for (i = 0 ; i < elementCount ; i++)
    {
        uint16_t val = *pMemory++;
        memcpy((void*)ucBufferPos, &val, sizeof(val));
        ucBufferPos += sizeof(val);
    }
}

static void dumpWords(const uint32_t* pMemory, size_t elementCount)
{
    size_t i;
    for (i = 0 ; i < elementCount ; i++)
    {
        uint32_t val = *pMemory++;
        memcpy((void*)ucBufferPos, &val, sizeof(val));
        ucBufferPos += sizeof(val);
    }
}

CrashCatcherReturnCodes CrashCatcher_DumpEnd(void)
{
    if (dfmAlertStarted)
    {
        uint32_t size = (uint32_t)ucBufferPos - (uint32_t)ucDataBuffer;
        if (xDfmAlertAddPayload(xAlertHandle, ucDataBuffer, size, CRASH_DUMP_NAME) == DFM_SUCCESS)
        {
        	DFM_DEBUG_PRINT("  DFM: Crash dump stored.\n");
        }
        else
        {
        	DFM_DEBUG_PRINT("  DFM: Failed storing crash dump.\n")
        }

        if (xDfmAlertEndOffline(xAlertHandle) == DFM_SUCCESS)
        {
        	DFM_DEBUG_PRINT("  DFM: Alert stored OK.\n");
        }
        else
        {
           DFM_DEBUG_PRINT("  DFM: Failed storing alert.\n");
        }

    }

    CRASH_FINALIZE();
    return CRASH_CATCHER_EXIT;
}


/* Called by GCC instrumentation if using one of the gcc options -fstack-protector, -fstack-protector-strong or -fstack-protector-all */
void __stack_chk_fail(void)
{
    // If this happens, the stack has been corrupted by the previous function in the call stack.
	// Note that the exact location of the stack corruption is not known, since detected when exiting the function.
	DFM_TRAP(DFM_TYPE_STACK_CHK_FAILED, "Stack corrupted");
}

#endif
