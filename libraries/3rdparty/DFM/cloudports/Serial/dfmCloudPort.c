/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * DFM serial port Cloud port
 */

#include <stddef.h>
#include <dfmCloudPort.h>
#include <dfmCloudPortConfig.h>
#include <dfm.h>
#include <string.h>
#include <stdio.h>

#if (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1))

/* Prototype for the print function */
extern void vMainUARTPrintString( char * pcString );

#define DFM_PRINT_SERIAL_DATA(msg) vMainUARTPrintString(msg)

	
static DfmCloudPortData_t *pxCloudPortData = (void*)0;

static uint32_t prvPrintDataAsHex(uint8_t* data, int size);
static DfmResult_t prvSerialPortUploadEntry(DfmEntryHandle_t xEntryHandle);

static uint32_t prvPrintDataAsHex(uint8_t* data, int size)
{
	uint32_t checksum = 0;
	int i;
	char buf[10];

    for (i = 0; i < size; i++)
    {
    	uint8_t byte = data[i];
    	checksum += byte;
        snprintf(buf, sizeof(buf), " %02X", (unsigned int)byte);

        if (i % 20 == 0)
        {
            DFM_CFG_LOCK_SERIAL();
            DFM_PRINT_ALERT_DATA(("[[ DATA:"));
        }

        DFM_PRINT_ALERT_DATA(buf);

        if ( (i+1) % 20 == 0)
        {
            DFM_PRINT_ALERT_DATA((" ]]\n"));
            DFM_CFG_UNLOCK_SERIAL();
        }
    }

    if (i % 20 != 0)
    {
        DFM_PRINT_ALERT_DATA((" ]]\n"));
        DFM_CFG_UNLOCK_SERIAL();
    }

    return checksum;
}

static DfmResult_t prvSerialPortUploadEntry(DfmEntryHandle_t xEntryHandle)
{
	void* dataptr;
	uint32_t checksum;
	uint32_t datalen;

	if (pxCloudPortData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (xEntryHandle == 0)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetData(xEntryHandle, &dataptr) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetDataSize(xEntryHandle, &datalen) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (datalen > 0xFFFF)
	{
		return DFM_FAIL;
	}

	// Magic value, also used to reveal data encoding (big/little endian)
	pxCloudPortData->xDfmSerialHeader.startmarker = 0x9FF91AA1;

	pxCloudPortData->xDfmSerialHeader.datalen = (uint16_t)datalen;

	/* Clear topic buffer before writing to it. */
	memset(pxCloudPortData->cKeyBuffer, 0x00, sizeof(pxCloudPortData->cKeyBuffer));
	pxCloudPortData->xDfmSerialHeader.keylen = 0;

	/* Prefix not needed since the host-side upload goes directly to the cloud storage (for AWS MQTT this provides an IoT Rule to execute. */
	if (xDfmCloudGenerateMQTTTopic(pxCloudPortData->cKeyBuffer, sizeof(pxCloudPortData->cKeyBuffer), "", xEntryHandle) == DFM_FAIL)
	{
		DFM_ERROR_PRINT("DFM Error, could not create topic string!\n");
		return DFM_FAIL;
	}

	pxCloudPortData->xDfmSerialHeader.keylen = strnlen(pxCloudPortData->cKeyBuffer, sizeof(pxCloudPortData->cKeyBuffer));

	DFM_CFG_LOCK_SERIAL();
	DFM_PRINT_SERIAL_DATA("\n[[ DevAlert Data Begins ]]\n");
	DFM_CFG_UNLOCK_SERIAL();

	checksum = 0; // Make sure to clear this
	checksum += prvPrintDataAsHex((uint8_t*)&pxCloudPortData->xDfmSerialHeader, sizeof(DfmSerialHeader_t));
	checksum += prvPrintDataAsHex((uint8_t*)pxCloudPortData->cKeyBuffer, pxCloudPortData->xDfmSerialHeader.keylen);
	checksum += prvPrintDataAsHex((uint8_t*)dataptr, pxCloudPortData->xDfmSerialHeader.datalen);

	snprintf(pxCloudPortData->buf, sizeof(pxCloudPortData->buf), "[[ DevAlert Data Ended. Checksum: %d ]]\n", (unsigned int)checksum);

	DFM_CFG_LOCK_SERIAL();
	DFM_PRINT_SERIAL_DATA(pxCloudPortData->buf);
	DFM_CFG_UNLOCK_SERIAL();

	return DFM_SUCCESS;
}

DfmResult_t xDfmCloudPortInitialize(DfmCloudPortData_t* pxBuffer)
{
	pxCloudPortData = pxBuffer;

	return DFM_SUCCESS;
}

DfmResult_t xDfmCloudPortSendAlert(DfmEntryHandle_t xEntryHandle)
{
	return prvSerialPortUploadEntry(xEntryHandle);
}

DfmResult_t xDfmCloudPortSendPayloadChunk(DfmEntryHandle_t xEntryHandle)
{
	return prvSerialPortUploadEntry(xEntryHandle);
}

#endif
