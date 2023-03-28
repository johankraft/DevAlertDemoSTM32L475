/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * FLASH Example implementation
 */

#include "dfmStoragePort.h"
#include <string.h>

#if (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1))

#include <dfm.h>

/* Include your specific flash header file */
#include "flash.h"
#include "stm32l4xx_hal.h"

#define DFM_STORAGE_PORT_ALERT_TYPE		0x34561842
#define DFM_STORAGE_PORT_PAYLOAD_TYPE	0x82713124

uint8_t FLASH_ALERT_DATA[11000] __attribute__( ( section( ".dfm_alert" ), aligned (8) ) ) = { 0 };
uint32_t ulWrOffset = 0;
uint32_t ulRdOffset = 0;

static DfmResult_t prvDfmStoragePortWrite(DfmEntryHandle_t xEntryHandle, uint32_t ulType, uint32_t ulOverwrite);

 /**
  * @internal DFM Flash entry structure
  */
typedef struct DfmStorageMetadata {
	uint32_t dummy;
} DfmStorageMetadata_t;

DfmResult_t xDfmStoragePortInitialize(DfmStoragePortData_t *pxBuffer)
{
	return DFM_SUCCESS;
}

DfmResult_t xDfmStoragePortStoreSession(void* pvData, uint32_t ulSize)
{
	return DFM_FAIL;
}

DfmResult_t xDfmStoragePortGetSession(void* pvBuffer, uint32_t ulBufferSize)
{
	return DFM_FAIL;
}


DfmResult_t xDfmStoragePortStoreAlert(DfmEntryHandle_t xEntryHandle, uint32_t ulOverwrite)
{
	return prvDfmStoragePortWrite(xEntryHandle, DFM_STORAGE_PORT_ALERT_TYPE, ulOverwrite);
}

DfmResult_t xDfmStoragePortGetAlert(void* pvBuffer, uint32_t ulBufferSize)
{
	if (*((uint32_t*)&FLASH_ALERT_DATA[ulRdOffset]) == 0xffffffff)
	{
		// Not written
		return DFM_FAIL;
	}
	uint32_t ulSize;
	if (xDfmEntryGetSize((DfmEntryHandle_t)&FLASH_ALERT_DATA[ulRdOffset], &ulSize) == DFM_FAIL)
	{
		return DFM_FAIL;
	}
	// Read data into buffer
	if (ulSize > ulBufferSize)
	{
		ulSize = ulBufferSize;
	}
	memcpy(pvBuffer, &FLASH_ALERT_DATA[ulRdOffset], ulSize);
	ulRdOffset += ulSize;
	ulRdOffset += (8 - (ulSize % 8));
	return DFM_SUCCESS;
}

DfmResult_t xDfmStoragePortStorePayloadChunk(DfmEntryHandle_t xEntryHandle, uint32_t ulOverwrite)
{
	return prvDfmStoragePortWrite(xEntryHandle, DFM_STORAGE_PORT_PAYLOAD_TYPE, ulOverwrite);
}

DfmResult_t xDfmStoragePortGetPayloadChunk(char* szSessionId, uint32_t ulAlertId, void* pvBuffer, uint32_t ulBufferSize)
{
	return xDfmStoragePortGetAlert(pvBuffer, ulBufferSize);
}

static DfmResult_t prvDfmStoragePortWrite(DfmEntryHandle_t xEntryHandle, uint32_t ulType, uint32_t ulOverwrite)
{
	if (xEntryHandle == 0)
	{
		return DFM_FAIL;
	}
	uint32_t ulSize;
	if (xDfmEntryGetSize(xEntryHandle, &ulSize) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if ((ulType != DFM_STORAGE_PORT_ALERT_TYPE) && (ulType != DFM_STORAGE_PORT_PAYLOAD_TYPE))
	{
		return DFM_FAIL;
	}

	uint32_t ulDst = (uint32_t)&FLASH_ALERT_DATA[ulWrOffset];
	/* Write to destination */
	if (ulWrOffset == 0)
	{
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
		if (FLASH_unlock_erase(ulDst, sizeof(FLASH_ALERT_DATA)) != 0)
		{
			return DFM_FAIL;
		}
	}
	ulWrOffset += ulSize;
	ulWrOffset += (8 - (ulSize % 8));
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
	if (FLASH_write_at(ulDst, xEntryHandle, ulSize) == 0)
	{
		return DFM_SUCCESS;
	}
	return DFM_FAIL;
}
#endif
