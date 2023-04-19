/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * DFM Cloud
 */

#include <dfm.h>

#if ((DFM_CFG_ENABLED) >= 1)

static DfmCloudData_t* pxCloudData = (void*)0;

DfmResult_t xDfmCloudInitialize(DfmCloudData_t* pxBuffer)
{
	if (pxBuffer == (void*)0)
	{
		return DFM_FAIL;
	}

	if (xDfmCloudPortInitialize(&pxBuffer->xCloudPortData) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	pxCloudData = pxBuffer;

	pxCloudData->ulInitialized = 1;

	return DFM_SUCCESS;
}

DfmResult_t xDfmCloudSendAlert(DfmEntryHandle_t xEntryHandle)
{
	DfmCloudStrategy_t xCloudStrategy = DFM_CLOUD_STRATEGY_OFFLINE;

	if (pxCloudData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxCloudData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (xEntryHandle == 0)
	{
		return DFM_FAIL;
	}

	if (xDfmSessionGetCloudStrategy(&xCloudStrategy) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xCloudStrategy == DFM_CLOUD_STRATEGY_OFFLINE)
	{
		return DFM_FAIL;
	}

	return xDfmCloudPortSendAlert(xEntryHandle);
}

DfmResult_t xDfmCloudSendPayloadChunk(DfmEntryHandle_t xEntryHandle)
{
	DfmCloudStrategy_t xCloudStrategy = DFM_CLOUD_STRATEGY_OFFLINE;

	if (pxCloudData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxCloudData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (xEntryHandle == 0)
	{
		return DFM_FAIL;
	}

	if (xDfmSessionGetCloudStrategy(&xCloudStrategy) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xCloudStrategy == DFM_CLOUD_STRATEGY_OFFLINE)
	{
		return DFM_FAIL;
	}

	return xDfmCloudPortSendPayloadChunk(xEntryHandle);
}



#endif
