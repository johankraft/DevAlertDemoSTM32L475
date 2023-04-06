/*******************************************************************************
 * DFM Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmStoragePort.c
 *
 * The FLASH storage port implementation.
 * Modify this to work with the intended hardware.
 *
 * Terms of Use
 * This file is part of the Device Firmware Monitor library (SOFTWARE), which
 * is the intellectual property of Percepio AB (PERCEPIO) and provided under a
 * license as follows.
 * The SOFTWARE may be used free of charge for the purpose of collecting and
 * transferring data to the DFM service. It may not be used or modified for
 * other purposes without explicit permission from PERCEPIO.
 * You may distribute the SOFTWARE in its original source code form, assuming
 * this text (terms of use, disclaimer, copyright notice) is unchanged. You are
 * allowed to distribute the SOFTWARE with minor modifications intended for
 * configuration or porting of the SOFTWARE, e.g., to allow using it on a 
 * specific processor, processor family or with a specific communication
 * interface. Any such modifications should be documented directly below
 * this comment block.  
 *
 * Disclaimer
 * The SOFTWARE is being delivered to you AS IS and PERCEPIO makes no warranty
 * as to its use or performance. PERCEPIO does not and cannot warrant the 
 * performance or results you may obtain by using the SOFTWARE or documentation.
 * PERCEPIO make no warranties, express or implied, as to noninfringement of
 * third party rights, merchantability, or fitness for any particular purpose.
 * In no event will PERCEPIO, its technology partners, or distributors be liable
 * to you for any consequential, incidental or special damages, including any
 * lost profits or lost savings, even if a representative of PERCEPIO has been
 * advised of the possibility of such damages, or for any claim by any third
 * party. Some jurisdictions do not allow the exclusion or limitation of
 * incidental, consequential or special damages, or the exclusion of implied
 * warranties or limitations on how long an implied warranty may last, so the
 * above limitations may not apply to you.
 *
 * Tabs are used for indent in this file (1 tab = 4 spaces)
 *
 * Copyright Percepio AB, 2021.
 * www.percepio.com
 ******************************************************************************/

#include <string.h>

#include <dfm.h>
#include "dfmStoragePort.h"

#if (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1))

/* Include your specific flash header file */
#include "FreeRTOS.h"
#include "flash.h"
#include "stm32l4xx_hal.h"


#define DFM_STORAGE_PORT_ALERT_TYPE		0x34561842
#define DFM_STORAGE_PORT_PAYLOAD_TYPE	0x82713124

dfmFlashData_t dfmFlashData __attribute__( ( section( ".dfm_alert" ), aligned (8) ) ) = { {0}, 0 };

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
	if (*((uint32_t*)&dfmFlashData.data[ulRdOffset]) == 0xffffffff)
	{
		// Not written
		return DFM_FAIL;
	}
	uint32_t ulSize;
	if (xDfmEntryGetSize((DfmEntryHandle_t)&dfmFlashData.data[ulRdOffset], &ulSize) == DFM_FAIL)
	{
		return DFM_FAIL;
	}
	// Read data into buffer
	if (ulSize > ulBufferSize)
	{
		ulSize = ulBufferSize;
	}
	memcpy(pvBuffer, &dfmFlashData.data[ulRdOffset], ulSize);
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
	DFM_DEBUG_PRINT("prvDfmStoragePortWrite\n");

	uint32_t preserve_alert_storage_counter = dfmFlashData.alert_storage_counter;
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

	uint32_t ulDst = (uint32_t)&dfmFlashData.data[ulWrOffset];

	/* Write to destination */
	if (ulWrOffset == 0)
	{
		preserve_alert_storage_counter++;

		// This erases the flash pages (might be needed) and updates the alert storage counter.
		if (dfmStoragePortReset(1) != DFM_SUCCESS)
		{
			return DFM_FAIL;
		}
	}

	ulWrOffset += ulSize;
	ulWrOffset += (8 - (ulSize % 8));

	DFM_DEBUG_PRINTF("  Write: 0x%08X - 0x%08X (%d)\n", (unsigned int)ulDst, (unsigned int)(ulDst + ulSize), (unsigned int)ulSize);

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

	if (FLASH_write_at(ulDst, xEntryHandle, ulSize) != 0)
	{
		return DFM_FAIL;
	}

    if (preserve_alert_storage_counter != dfmFlashData.alert_storage_counter)
	{
    	DFM_DEBUG_PRINT("TODO: Update alert storage counter!"); // But not here...


    	/* 	TODO: This part below updates the needs to be done, but should probably be the last flash operation, ideally by the caller function (xDfmAlertEnd, xDfmAlertEndOffline)
    	 *  The point is to increment the alert storage counter, that is needed to run the right test case (remember between restarts) and can be used as sessionID perhaps.*/

    	/*
		DFM_DEBUG_PRINTF("  Alert storage counter (at 0x%08X) before erase: %d\n", &dfmFlashData.alert_storage_counter, dfmFlashData.alert_storage_counter);

		if (FLASH_write_at( (uint32_t)&dfmFlashData.alert_storage_counter, &preserve_alert_storage_counter, 4) != 0)
		{
				return DFM_FAIL;
		}

		DFM_DEBUG_PRINTF("  Alert storage counter after update: %d\n", dfmFlashData.alert_storage_counter);*/
	}

	return DFM_SUCCESS;
}


DfmResult_t dfmStoragePortReset(int isNewAlert)
{
	//uint32_t preserve_alert_storage_counter = dfmFlashData.alert_storage_counter;

	DFM_DEBUG_PRINTF("dfmStoragePortReset(%d)\n", isNewAlert);

	DFM_DEBUG_PRINTF("  Erase: 0x%08X - 0x%08X (%d)\n", (unsigned int)&dfmFlashData, (unsigned int)((int)&dfmFlashData + sizeof(dfmFlashData)), sizeof(dfmFlashData));



	// This function is called both when creating a new alert and also from main, after sending stored alerts.
	// In the latter case, we only want to clear the stored alert(s), and should not increment the alert storage counter (but perhaps a flash erase counter)
	// In the former case, we should increment the counter, i.e. when storing a new alert before the restart.
	/*if (isNewAlert == 1)
	{
		preserve_alert_storage_counter++;
	}*/

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

	if (FLASH_unlock_erase((uint32_t)&dfmFlashData, sizeof(dfmFlashData)) != 0)
	{
		return DFM_FAIL;
	}

	// The flash counter might be wiped by the erase, so we copy it before the erase and write it back
	/*if (FLASH_write_at( (uint32_t)&dfmFlashData.alert_storage_counter, &preserve_alert_storage_counter, 4) != 0)
	{
		return DFM_FAIL;
	}

	DFM_DEBUG_PRINTF("  Alert storage counter after erase: %d\n", dfmFlashData.alert_storage_counter);
*/
	return DFM_SUCCESS;

}

#endif
