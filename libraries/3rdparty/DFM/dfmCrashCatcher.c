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

#if ((DFM_CFG_ENABLED) >= 1)

#if ((CRASH_ADD_TRACE) >= 1)
#include "trcRecorder.h"
static void prvAddTracePayload();
#endif

DfmAlertHandle_t xAlertHandle;
uint8_t dfmAlertStarted = 0;
static uint8_t ucDataBuffer[CRASH_DUMP_MAX_SIZE] __attribute__ ((aligned (8)));
static uint8_t* ucBufferPos;



static void dumpHalfWords(const uint16_t* pMemory, size_t elementCount);
static void dumpWords(const uint32_t* pMemory, size_t elementCount);

const CrashCatcherMemoryRegion* CrashCatcher_GetMemoryRegions(void)
{
    static const CrashCatcherMemoryRegion regions[] = {
        {CRASH_MEM_REGION1_START, CRASH_MEM_REGION1_START + CRASH_MEM_REGION1_SIZE, CRASH_CATCHER_BYTE},
        {CRASH_MEM_REGION2_START, CRASH_MEM_REGION2_START + CRASH_MEM_REGION2_SIZE, CRASH_CATCHER_BYTE},
        {CRASH_MEM_REGION3_START, CRASH_MEM_REGION3_START + CRASH_MEM_REGION3_SIZE, CRASH_CATCHER_BYTE},
        {0xFFFFFFFF, 0xFFFFFFFF, CRASH_CATCHER_BYTE}
    };
    return regions;
}

void CrashCatcher_DumpStart(const CrashCatcherInfo* pInfo)
{
    (void)pInfo;
    ucBufferPos = &ucDataBuffer[0];
    /* Do not try to send the alert */
    xDfmSessionSetCloudStrategy(DFM_CLOUD_STRATEGY_OFFLINE);
    if (xDfmAlertBegin(DFM_TYPE_HARDFAULT, "Fault Exception", &xAlertHandle) == DFM_SUCCESS)
    {
    	xDfmAlertAddSymptom(xAlertHandle, DFM_SYMPTOM_CURRENT_TASK, simplechecksum32(pcTaskGetName( xTaskGetCurrentTaskHandle() )));
    	xDfmAlertAddSymptom(xAlertHandle, DFM_SYMPTOM_STACKPTR, pInfo->sp);

#if ((CRASH_ADD_TRACE) >= 1)
    	xTracePrintCompactF0("ALERT", "Fault exception! See crash dump for details.");
        prvAddTracePayload();
#endif
        dfmAlertStarted = 1;
    }
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
    switch (elementSize)
    {
    case CRASH_CATCHER_BYTE:
        memcpy((void*)ucBufferPos, pvMemory, elementCount);
        ucBufferPos += elementCount;
        break;
    case CRASH_CATCHER_HALFWORD:
        dumpHalfWords(pvMemory, elementCount);
        break;
    case CRASH_CATCHER_WORD:
        dumpWords(pvMemory, elementCount);
        break;
    }
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
        xDfmAlertAddPayload(xAlertHandle, ucDataBuffer, size, CRASH_DUMP_NAME);
        xDfmAlertEndOffline(xAlertHandle);
    }
    CRASH_FINALIZE();
    return CRASH_CATCHER_EXIT;
}
#endif
