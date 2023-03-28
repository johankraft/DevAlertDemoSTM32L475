/*******************************************************************************
 * DFM Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmStoragePort.c
 *
 * The Memory storage port header, not using non-volatile memory.
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

#include "dfmStoragePort.h"
#include "dfmCloudPortConfig.h"

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED))

#include "percepio_dfm.h"
#include <string.h>

DfmSettings_t STORAGE_SETTINGS = { 0 };
DfmSession_t STORAGE_SESSION = { 0 };
DfmAlertHeader_t STORAGE_ALERT_HEADER = { 0 };

uint32_t ulDfmStoragePortHasData(uint32_t ulSection, uint32_t* pulFlag)
{
	uint32_t ulDummy = 0;
	uint32_t* pulValue = 0;
	*pulFlag = 0;
	/* Retrieve first first uint32_t of section data */
	if (ulDfmStoragePortRead(ulSection, (void**)&pulValue, sizeof(uint32_t), 0, &ulDummy) != 0)
	{
		return 1;
	}

	/* Check if it is non-zero */
	if (*pulValue != 0)
	{
		*pulFlag = 1;
	}

	return 0;
}

/* Clears first uint32_t of section data. */
uint32_t ulDfmStoragePortClearData(uint32_t ulSection)
{
	switch (ulSection)
	{
	case DFM_SECTION_SETTINGS:
        memset(&STORAGE_SETTINGS, 0, sizeof(STORAGE_SETTINGS));
		break;
	case DFM_SECTION_SESSION:
        memset(&STORAGE_SESSION, 0, sizeof(STORAGE_SESSION));
		break;
	case DFM_SECTION_ALERT_HEADER:
        memset(&STORAGE_ALERT_HEADER, 0, sizeof(STORAGE_ALERT_HEADER));
		break;
	case DFM_SECTION_TRACE_DATA:
		return DFM_STATUS_CODE_STORAGE_PORT_NOT_SUPPORTED;
	}

    return 0;
}

/* TODO: Describe usage */
uint32_t ulDfmStoragePortWrite(uint32_t ulSection, void* pvData, uint32_t ulSize, uint32_t ulOffset, uint32_t* pulBytesWritten)
{
	void* ulDst = NULL;

	/* Get destination address */
	switch (ulSection)
	{
	case DFM_SECTION_SETTINGS:
		ulDst = (void*)&STORAGE_SETTINGS + ulOffset;
		break;
	case DFM_SECTION_SESSION:
		ulDst = (void*)&STORAGE_SESSION + ulOffset;
		break;
	case DFM_SECTION_ALERT_HEADER:
		ulDst = (void*)&STORAGE_ALERT_HEADER + ulOffset;
		break;
	case DFM_SECTION_TRACE_DATA:
		return DFM_STATUS_CODE_STORAGE_PORT_NOT_SUPPORTED;
	}

	/* We assume everything goes well. */
	*pulBytesWritten = ulSize;

	/* Write to destination */
    return memcpy(ulDst, pvData, ulSize) == ulDst;
}

/* TODO: Describe usage */
uint32_t ulDfmStoragePortRead(uint32_t ulSection, void** ppvData, uint32_t ulSize, uint32_t ulOffset, uint32_t* pulBytesRead)
{
	/* Since this is local variables, don't do a read to a buffer, just modify the pointer by ulOffset */
	switch (ulSection)
	{
	case DFM_SECTION_SETTINGS:
		*ppvData = (void*)((uint32_t)&STORAGE_SETTINGS + ulOffset);
		break;
	case DFM_SECTION_SESSION:
		*ppvData = (void*)((uint32_t)&STORAGE_SESSION + ulOffset);
		break;
	case DFM_SECTION_ALERT_HEADER:
		*ppvData = (void*)((uint32_t)&STORAGE_ALERT_HEADER + ulOffset);
		break;
	case DFM_SECTION_TRACE_DATA:
		return DFM_STATUS_CODE_STORAGE_PORT_NOT_SUPPORTED;
	}

	*pulBytesRead = ulSize;
	
	return 0;
}

#endif /* (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1)) */
