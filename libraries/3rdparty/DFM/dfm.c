/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * DFM
 */

#include <dfm.h>

#if ((DFM_CFG_ENABLED) >= 1)

#if (!defined(DFM_VERSION) || (DFM_VERSION < DFM_VERSION_INITIAL) || (DFM_VERSION > DFM_VERSION_2_0))
#error "Invalid DFM_VERSION!"
#endif

#include <assert.h>
#include <string.h>
#include <stdint.h>

static DfmData_t xDfmData;
static DfmData_t* pxDfmData = &xDfmData;

/* Used for snprintf calls */
char dfmPrintBuffer[128];

DfmResult_t xDfmInitialize()
{
	if (pxDfmData == (void*)0)
	{
		return DFM_FAIL;
	}

	(void)memset(pxDfmData, 0, sizeof(DfmData_t));

	if (xDfmSessionInitialize(&pxDfmData->xSessionData) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmKernelPortInitialize(&pxDfmData->xKernelPortData) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmAlertInitialize(&pxDfmData->xAlertData) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryInitialize(&pxDfmData->xEntryData) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

/* Cloud port not needed if using serial port upload */
#if (DFM_CFG_SERIAL_UPLOAD_ONLY != 1)
	if (xDfmCloudInitialize(&pxDfmData->xCloudData) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmStorageInitialize(&pxDfmData->xStorageData) == DFM_FAIL)
	{
		return DFM_FAIL;
	}
#endif

	pxDfmData->ulDfmInitialized = 1;

	(void)xDfmSessionEnable(0); /* Try to enable, but don't override if disabled */

	return DFM_SUCCESS;
}

uint32_t ulDfmIsInitialized(void)
{
	return xDfmData.ulDfmInitialized;
}

#endif
