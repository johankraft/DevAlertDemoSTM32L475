/*******************************************************************************
 * DFM Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmCloudPort.c
 *
 * RTT cloud port implementation.
 * It pretends to be a cloud port connection but sends data over RTT.
 * Requires the SEGGER RTT files (SEGGER_RTT.c/h and SEGGER_RTT_Conf.h) as well
 * as the RTT streaming port files (trcStreamingPort.c/h).
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

#include "dfmCloudPort.h"
#include "SEGGER_RTT_Conf.h"
#include "SEGGER_RTT.h"

#include <stdio.h>
#include <string.h>

#if (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1))

#if (TRC_CFG_CLOUD_RTT_UP_BUFFER_INDEX >= SEGGER_RTT_MAX_NUM_UP_BUFFERS)
#error "TRC_CFG_RTT_UP_BUFFER_INDEX must be smaller than SEGGER_RTT_MAX_NUM_UP_BUFFERS"
#endif

#if (TRC_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX >= SEGGER_RTT_MAX_NUM_DOWN_BUFFERS)
#error "TRC_CFG_RTT_DOWN_BUFFER_INDEX must be smaller than SEGGER_RTT_MAX_NUM_DOWN_BUFFERS"
#endif

/* If index is defined as 0, the internal RTT buffers will be used instead of this. */
#if ((DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX) == 0)

static char *pcRTTUpBuffer = 0;    /* Not actually used. Ignore allocation method. */
#define DFM_CLOUD_PORT_MALLOC() /* Static allocation. Not used. */

#else /* ((DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX) == 0) */

#if ((TRC_CFG_RECORDER_BUFFER_ALLOCATION) == TRC_RECORDER_BUFFER_ALLOCATION_STATIC)
static char pcRTTUpBuffer[DFM_CFG_CLOUD_RTT_BUFFER_SIZE_UP];    /* Static allocation */
#define DFM_CLOUD_PORT_MALLOC() /* Static allocation. Not used. */
#endif /* ((TRC_CFG_RECORDER_BUFFER_ALLOCATION) == TRC_RECORDER_BUFFER_ALLOCATION_STATIC) */

#if ((TRC_CFG_RECORDER_BUFFER_ALLOCATION) == TRC_RECORDER_BUFFER_ALLOCATION_DYNAMIC)
static char* pcRTTUpBuffer = 0;    /* Dynamic allocation */
#define DFM_CLOUD_PORT_MALLOC() pcRTTUpBuffer = TRC_PORT_MALLOC(DFM_CFG_CLOUD_RTT_BUFFER_SIZE_UP);
#endif /* ((TRC_CFG_RECORDER_BUFFER_ALLOCATION) == TRC_RECORDER_BUFFER_ALLOCATION_DYNAMIC) */

#if ((TRC_CFG_RECORDER_BUFFER_ALLOCATION) == TRC_RECORDER_BUFFER_ALLOCATION_CUSTOM)
char *pcRTTUpBuffer = 0;					/* Custom allocation, user needs to extern this and assign it */
#define DFM_CLOUD_PORT_MALLOC()										/* Not used in custom mode */
#endif /* ((TRC_CFG_RECORDER_BUFFER_ALLOCATION) == TRC_RECORDER_BUFFER_ALLOCATION_CUSTOM) */

#endif /* ((DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX) == 0) */

/* Down-buffer. If index is defined as 0, the internal RTT buffers will be used instead of this. */ \
#if ((DFM_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX) == 0)
static char* pcRTTDownBuffer = 0;           /* Not actually used. Ignore allocation method. */
#else /* ((DFM_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX) == 0) */
static char pcRTTDownBuffer[DFM_CFG_CLOUD_RTT_BUFFER_SIZE_DOWN]; /* Always static allocation, since usually small. */
#endif /* ((DFM_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX) == 0) */

/* The maximum topic size (32-bit aligned, one extra byte for zero termination */
#define TOPIC_MAX_SIZE (((((DFM_CFG_CLOUD_PORT_MAX_TOPIC_SIZE) + 1) + 3) / 4) * 4)

typedef struct dfm_rtt_header
{
	uint32_t uiStartMarker;
	uint16_t usTopicSize;
	uint16_t usDataSize;
	char cTopicBuffer[TOPIC_MAX_SIZE];
} dfm_rtt_header_t;

static dfm_rtt_header_t xHeader = { 0, 0, 0, { 0 } };

uint32_t uiEnabled = 0;

static int32_t writeToRTT(void* ptrData, uint32_t size, int32_t* ptrBytesWritten);
static int32_t readFromRTT(void* ptrData, uint32_t size, int32_t* ptrBytesRead);

static int32_t writeToRTT(void* ptrData, uint32_t size, int32_t* ptrBytesWritten)
{
	uint32_t bytesWritten = SEGGER_RTT_Write((DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX), (const char*)ptrData, size);

	if (ptrBytesWritten != NULL)
		*ptrBytesWritten = (int32_t)bytesWritten;

	return 0;
}

static int32_t readFromRTT(void* ptrData, uint32_t size, int32_t* ptrBytesRead)
{
	uint32_t bytesRead = 0;

	if (SEGGER_RTT_HASDATA(DFM_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX))
	{
		bytesRead = SEGGER_RTT_Read((DFM_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX), (char*)ptrData, size);

		if (ptrBytesRead != NULL)
			*ptrBytesRead = (int32_t)bytesRead;

	}

	return 0;
}

uint32_t ulGetUniqueSessionID(char* cUniqueSessionBuffer, uint32_t ulBufferSize, int32_t *plBytesRead)
{
	int32_t lBytesWritten = 0;
	uint32_t i;

	*plBytesRead = 0;
	
	writeToRTT("REQ_SESSIONID", 13, &lBytesWritten);

	if (lBytesWritten == 0)
	{
		return 1;
	}

#if ((DFM_CFG_CLOUD_RTT_CONNECTION_TYPE) == DFM_CLOUD_RTT_CONNECTION_WAIT)
	/* Keep trying until it succeeds */
	(void)i;
	for (;;)
#else /* ((DFM_CFG_CLOUD_RTT_CONNECTION_TYPE) == DFM_CLOUD_RTT_CONNECTION_WAIT)*/
	/* Since the response isn't immediate, we need to try a few times */
	for (i = 0; i < (DFM_CFG_CLOUD_RTT_MAX_SESSIONID_ATTEMPTS); i++)
#endif /* ((DFM_CFG_CLOUD_RTT_CONNECTION_TYPE) == DFM_CLOUD_RTT_CONNECTION_WAIT) */
	{
		readFromRTT(cUniqueSessionBuffer, ulBufferSize, plBytesRead);
		if (*plBytesRead != 0)
		{
			uiEnabled = 1;
			return 0;
		}
	}

#if ((DFM_CFG_CLOUD_RTT_CONNECTION_TYPE) == DFM_CLOUD_RTT_CONNECTION_FAIL)
	return 2; /* We failed. Let the world know. */
#elif ((DFM_CFG_CLOUD_RTT_CONNECTION_TYPE) == DFM_CLOUD_RTT_CONNECTION_CONTINUE)
	return 0; /* We failed, but we do so silently. */
#endif /* ((DFM_CFG_CLOUD_RTT_CONNECTION_TYPE) == DFM_CLOUD_RTT_CONNECTION_FAIL) */
}

uint32_t ulDfmRTTPortInit(void)
{
	DFM_CLOUD_PORT_MALLOC(); /*Dynamic allocation or empty if static */ \
	SEGGER_RTT_ConfigUpBuffer((DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX), "TzData", pcRTTUpBuffer, (DFM_CFG_CLOUD_RTT_BUFFER_SIZE_UP), (DFM_CFG_CLOUD_RTT_MODE));
	SEGGER_RTT_ConfigDownBuffer((DFM_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX), "TzCtrl", pcRTTDownBuffer, (DFM_CFG_CLOUD_RTT_BUFFER_SIZE_DOWN), (DFM_CFG_CLOUD_RTT_MODE));

	return 0;
}

uint32_t ulDfmRTTPortSend(const void* pvData, uint32_t ulSize, const char* szUniqueSessionID, uint32_t ulTraceCounter, const char* szType, uint32_t ulSlice, uint32_t ulSlices, uint32_t* pulBytesSent)
{
	uint32_t uiBytesWritten = 0;

	if (uiEnabled == 0)
	{
		return 1; /* RTT connection wasn't established at startup */
	}

	xHeader.uiStartMarker = 0x9FF91AA1;
	xHeader.usDataSize = ulSize;

	snprintf(xHeader.cTopicBuffer, TOPIC_MAX_SIZE - 1, "DevAlert/%s/%lu/%lu-%lu_%s", szUniqueSessionID, ulTraceCounter, ulSlice, ulSlices, szType);
	xHeader.cTopicBuffer[TOPIC_MAX_SIZE - 1] = 0; /* Make sure it is zero terminated */
	xHeader.usTopicSize = strlen(xHeader.cTopicBuffer);

	uiBytesWritten = SEGGER_RTT_Write((DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX), (const char*)&xHeader, sizeof(xHeader) - TOPIC_MAX_SIZE + xHeader.usTopicSize);

	if (uiBytesWritten == 0)
	{
		return 2;
	}
	
	uiBytesWritten = SEGGER_RTT_Write((DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX), (const char*)pvData, ulSize);

	/* We only return how much of the actual data was sent, not including the topic */
	if (pulBytesSent != NULL)
		*pulBytesSent = (int32_t)uiBytesWritten;

	return 0;
}

#endif /* (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1)) */
