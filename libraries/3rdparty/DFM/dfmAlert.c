/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * DFM Alert
 */


#include <stdio.h>
#include <string.h>

#include <dfm.h>

#if ((DFM_CFG_ENABLED) >= 1)

static DfmResult_t prvDfmAlertInitialize(DfmAlertHandle_t xAlertHandle, uint8_t ucDfmVersion, uint32_t ulProduct, const char* szFirmwareVersion);
static uint32_t prvDfmAlertCalculateChecksum(uint8_t* pxData, uint32_t ulSize);
static void prvDfmAlertReset(DfmAlert_t* pxAlert);
static DfmResult_t prvDfmProcessAlert(DfmAlertEntryCallback_t xAlertCallback, DfmAlertEntryCallback_t xPayloadCallback);
static DfmResult_t prvDfmGetAll(DfmAlertEntryCallback_t xAlertCallback, DfmAlertEntryCallback_t xPayloadCallback);

static DfmResult_t prvStoreAlert(DfmEntryHandle_t xEntryHandle);
static DfmResult_t prvStorePayloadChunk(DfmEntryHandle_t xEntryHandle);
static DfmResult_t prvSendAlert(DfmEntryHandle_t xEntryHandle);
static DfmResult_t prvSendPayloadChunk(DfmEntryHandle_t xEntryHandle);

DfmAlertData_t* pxDfmAlertData = (void*)0;

static DfmResult_t prvStoreAlert(DfmEntryHandle_t xEntryHandle)
{
	return xDfmStorageStoreAlert(xEntryHandle);
}

static DfmResult_t prvStorePayloadChunk(DfmEntryHandle_t xEntryHandle)
{
	return xDfmStorageStorePayloadChunk(xEntryHandle);
}

static DfmResult_t prvSendAlert(DfmEntryHandle_t xEntryHandle)
{
	return xDfmCloudSendAlert(xEntryHandle);
}

static DfmResult_t prvSendPayloadChunk(DfmEntryHandle_t xEntryHandle)
{
	return xDfmCloudSendPayloadChunk(xEntryHandle);
}

#if (defined(DFM_CFG_SERIAL_UPLOAD_ONLY) && DFM_CFG_SERIAL_UPLOAD_ONLY == 1)

char buf[80];

uint32_t prvPrintDataAsHex(uint8_t* data, int size)
{
	uint32_t checksum = 0;
	int i;

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


typedef struct{
	uint32_t startmarker;
	uint16_t keylen;
	uint16_t datalen;
} DfmSerialHeader_t;

char cKeyBuffer[120]; // Key/MQTT topic

DfmSerialHeader_t DfmSerialHeader = {0, 0, 0};

static DfmResult_t prvSerialPortUploadAlert(DfmEntryHandle_t xEntryHandle)
{
	uint32_t checksum;
	uint32_t datalen;
	void* dataptr;

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
	DfmSerialHeader.startmarker = 0x9FF91AA1;

	DfmSerialHeader.datalen = (uint16_t)datalen;

	/* Clear topic buffer before writing to it. */
	memset(cKeyBuffer, 0x00, sizeof(cKeyBuffer));
	DfmSerialHeader.keylen = 0;

	if (xDfmAlertGenerateAlertKey(cKeyBuffer, sizeof(cKeyBuffer), "", xEntryHandle) == DFM_SUCCESS)
	{
		DfmSerialHeader.keylen = strnlen(cKeyBuffer, sizeof(cKeyBuffer));
	}
    else
    {
    	DFM_PRINT_ERROR("DFM Error, could not create topic string!\n");
    	return DFM_FAIL;
	}

	DFM_CFG_LOCK_SERIAL();
	DFM_PRINT_ALERT_DATA("\n[[ DevAlert Data Begins ]]\n");
	DFM_CFG_UNLOCK_SERIAL();

	checksum = 0; // Make sure to clear this
	checksum += prvPrintDataAsHex((uint8_t*)&DfmSerialHeader, sizeof(DfmSerialHeader_t));
	checksum += prvPrintDataAsHex((uint8_t*)cKeyBuffer, DfmSerialHeader.keylen);
	checksum += prvPrintDataAsHex((uint8_t*)dataptr, DfmSerialHeader.datalen);

	/* Keep outside lock section */
	snprintf(buf, sizeof(buf), "\n[[ DevAlert Data Ended. Checksum: %d ]]\n\n", (unsigned int)checksum);

	DFM_CFG_LOCK_SERIAL();
	DFM_PRINT_ALERT_DATA(buf);
	DFM_CFG_UNLOCK_SERIAL();

	return DFM_SUCCESS;
}


static DfmResult_t prvSerialPortUploadPayloadChunk(DfmEntryHandle_t xEntryHandle)
{
	return prvSerialPortUploadAlert(xEntryHandle);
}

#endif

DfmResult_t xDfmAlertInitialize(DfmAlertData_t *pxBuffer)
{
	if (pxBuffer == (void*)0)
	{
		return DFM_FAIL;
	}

	pxDfmAlertData = pxBuffer;
	
	pxDfmAlertData->ulPayloadCount = 0;
	pxDfmAlertData->ulInitialized = 1;

	return prvDfmAlertInitialize((DfmAlertHandle_t)&pxDfmAlertData->xAlert, DFM_VERSION, DFM_CFG_PRODUCTID, DFM_CFG_FIRMWARE_VERSION);
}

DfmResult_t xDfmAlertGetVersion(DfmAlertHandle_t xAlertHandle, uint8_t* pucVersion)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pucVersion == (void*)0)
	{
		return DFM_FAIL;
	}

	*pucVersion = pxAlert->ucVersion;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertGetProduct(DfmAlertHandle_t xAlertHandle, uint32_t* pulProduct)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	*pulProduct = pxAlert->ulProduct;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertGetFirmwareVersion(DfmAlertHandle_t xAlertHandle, const char** pszFirmwareVersion)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	*pszFirmwareVersion = pxAlert->cFirmwareVersionBuffer;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertReset(DfmAlertHandle_t xAlertHandle)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	prvDfmAlertReset(pxAlert);

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertBegin(uint32_t ulAlertType, const char* szAlertDescription, DfmAlertHandle_t* pxAlertHandle)
{
	DfmAlert_t* pxAlert;
	uint32_t i;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlertHandle == (void*)0)
	{
		return DFM_FAIL;
	}

	if (szAlertDescription == (void*)0)
	{
		return DFM_FAIL;
	}

	if (szAlertDescription[0] == (char)0)
	{
		return DFM_FAIL;
	}

	pxAlert = &pxDfmAlertData->xAlert;

	prvDfmAlertReset(pxAlert);

	pxAlert->ucVersion = DFM_VERSION;

	pxAlert->ulAlertType = ulAlertType;

	if (xDfmSessionGenerateNewAlertId() == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	for (i = (uint32_t)0; i < (uint32_t)(DFM_DESCRIPTION_MAX_LEN); i++)
	{
		pxAlert->cAlertDescription[i] = szAlertDescription[i];

		if (szAlertDescription[i] == (char)0)
		{
			break;
		}
	}

	*pxAlertHandle = (DfmAlertHandle_t)pxAlert;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertAddSymptom(DfmAlertHandle_t xAlertHandle, uint32_t ulSymptomId, uint32_t ulValue)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->xAlert.ucSymptomCount >= (uint8_t)(DFM_CFG_MAX_SYMPTOMS))
	{
		(void)xDfmSessionSetStatus(DFM_STATUS_CODE_MAX_SYMPTOMS_EXCEEDED);
		return DFM_FAIL;
	}

	pxAlert->xSymptoms[pxDfmAlertData->xAlert.ucSymptomCount].ulId = ulSymptomId;
	pxAlert->xSymptoms[pxDfmAlertData->xAlert.ucSymptomCount].ulValue = ulValue;

	pxDfmAlertData->xAlert.ucSymptomCount++;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertGetSymptom(DfmAlertHandle_t xAlertHandle, uint32_t ulIndex, uint32_t* pulSymptomId, uint32_t* pulValue)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	if (ulIndex >= (uint32_t)(DFM_CFG_MAX_SYMPTOMS))
	{
		return DFM_FAIL;
	}

	if (ulIndex >= (uint32_t)pxAlert->ucSymptomCount)
	{
		return DFM_FAIL;
	}

	*pulSymptomId = pxAlert->xSymptoms[ulIndex].ulId;
	*pulValue = pxAlert->xSymptoms[ulIndex].ulValue;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertAddPayload(DfmAlertHandle_t xAlertHandle, void* pvData, uint32_t ulSize, const char* szDescription)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;
	uint32_t i;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pvData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (ulSize == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (szDescription == (void*)0)
	{
		return DFM_FAIL;
	}

	if (szDescription[0] == (char)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulPayloadCount >= (uint32_t)(DFM_CFG_MAX_PAYLOADS))
	{
		return DFM_FAIL;
	}

	pxDfmAlertData->xPayloads[pxDfmAlertData->ulPayloadCount].pvData = pvData;

	pxDfmAlertData->xPayloads[pxDfmAlertData->ulPayloadCount].ulSize = ulSize;

	for (i = (uint32_t)0; i < (uint32_t)(DFM_PAYLOAD_DESCRIPTION_MAX_LEN); i++)
	{
		pxDfmAlertData->xPayloads[pxDfmAlertData->ulPayloadCount].cDescriptionBuffer[i] = szDescription[i];

		if (szDescription[i] == (char)0)
		{
			break;
		}
	}

	pxDfmAlertData->ulPayloadCount++;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertGetPayload(DfmAlertHandle_t xAlertHandle, uint32_t ulIndex, void** ppvData, uint32_t* pulSize, char** pszDescription)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	if (ppvData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pulSize == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pszDescription == (void*)0)
	{
		return DFM_FAIL;
	}

	if (ulIndex >= (uint32_t)(DFM_CFG_MAX_PAYLOADS))
	{
		return DFM_FAIL;
	}

	if (ulIndex >= pxDfmAlertData->ulPayloadCount)
	{
		return DFM_FAIL;
	}

	*ppvData = pxDfmAlertData->xPayloads[ulIndex].pvData;

	*pulSize = pxDfmAlertData->xPayloads[ulIndex].ulSize;

	*pszDescription = pxDfmAlertData->xPayloads[ulIndex].cDescriptionBuffer;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertGetType(DfmAlertHandle_t xAlertHandle, uint32_t* pulAlertType)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pulAlertType == (void*)0)
	{
		return DFM_FAIL;
	}

	*pulAlertType = pxAlert->ulAlertType;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertGetDescription(DfmAlertHandle_t xAlertHandle, const char** pszAlertDescription)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pszAlertDescription == (void*)0)
	{
		return DFM_FAIL;
	}

	*pszAlertDescription = pxAlert->cAlertDescription;

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertEnd(DfmAlertHandle_t xAlertHandle)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	pxAlert->ulChecksum = prvDfmAlertCalculateChecksum((uint8_t*)pxAlert, sizeof(DfmAlert_t) - sizeof(uint32_t));

#if (defined(DFM_CFG_SERIAL_UPLOAD_ONLY) && DFM_CFG_SERIAL_UPLOAD_ONLY == 1)

	/* Output alert data to serial port using DFM_PRINT_ALERT_DATA */
	if (prvDfmProcessAlert(prvSerialPortUploadAlert, prvSerialPortUploadPayloadChunk) == DFM_SUCCESS)
	{
		prvDfmAlertReset(pxAlert);
		return DFM_SUCCESS;
	}
	return DFM_FAIL;

#endif


	/* Try to send */
	if (prvDfmProcessAlert(prvSendAlert, prvSendPayloadChunk) == DFM_SUCCESS)
	{
		prvDfmAlertReset(pxAlert);

		return DFM_SUCCESS;
	}

	/* Try to store */
	if (prvDfmProcessAlert(prvStoreAlert, prvStorePayloadChunk) == DFM_SUCCESS)
	{
		prvDfmAlertReset(pxAlert);

		return DFM_SUCCESS;
	}

	/* Could not send or store */
	prvDfmAlertReset(pxAlert);

	return DFM_FAIL;
}

DfmResult_t xDfmAlertEndOffline(DfmAlertHandle_t xAlertHandle)
{
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (ulDfmSessionIsEnabled() == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	pxAlert->ulChecksum = prvDfmAlertCalculateChecksum((uint8_t*)pxAlert, sizeof(DfmAlert_t) - sizeof(uint32_t));

#if (defined(DFM_CFG_SERIAL_UPLOAD_ONLY) && DFM_CFG_SERIAL_UPLOAD_ONLY == 1)

	/* Output alert data to serial port using configPRINT_STRING */
	if (prvDfmProcessAlert(prvSerialPortUploadAlert, prvSerialPortUploadPayloadChunk) == DFM_SUCCESS)
	{
		prvDfmAlertReset(pxAlert);
		return DFM_SUCCESS;
	}
	return DFM_FAIL;

#endif

	/* Try to store */
	if (prvDfmProcessAlert(prvStoreAlert, prvStorePayloadChunk) == DFM_SUCCESS)
	{
		prvDfmAlertReset(pxAlert);

		return DFM_SUCCESS;
	}

	/* Could not store */
	prvDfmAlertReset(pxAlert);

	return DFM_FAIL;
}

static DfmResult_t prvDfmAlertInitialize(DfmAlertHandle_t xAlertHandle, uint8_t ucDfmVersion, uint32_t ulProduct, const char* szFirmwareVersion)
{
	uint32_t i;
	DfmAlert_t* pxAlert = (DfmAlert_t*)xAlertHandle;

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxAlert == (void*)0)
	{
		return DFM_FAIL;
	}

	if (szFirmwareVersion == (void*)0) /*cstat !MISRAC2012-Rule-14.3_b C-STAT complains because DFM_CFG_FIRMWARE_VERSION refers to a static string that is never null in this project. In user projects, this is not guaranteed and must therefor be checked.*/
	{
		return DFM_FAIL;
	}

	pxDfmAlertData->xAlert.ucStartMarkers[0] = 0x50; /* 'P' */
	pxDfmAlertData->xAlert.ucStartMarkers[1] = 0x44; /* 'D' */
	pxDfmAlertData->xAlert.ucStartMarkers[2] = 0x66; /* 'f' */
	pxDfmAlertData->xAlert.ucStartMarkers[3] = 0x6D; /* 'm' */

	pxDfmAlertData->xAlert.usEndianness = 0x0FF0;
	pxDfmAlertData->xAlert.ucVersion = ucDfmVersion;
	pxDfmAlertData->xAlert.ucMaxSymptoms = (DFM_CFG_MAX_SYMPTOMS);
	pxDfmAlertData->xAlert.ucFirmwareVersionSize = (DFM_FIRMWARE_VERSION_MAX_LEN);
	pxDfmAlertData->xAlert.ucDescriptionSize = (DFM_DESCRIPTION_MAX_LEN);
	pxDfmAlertData->xAlert.ulProduct = ulProduct;

	for (i = (uint32_t)0; i < (uint32_t)(DFM_FIRMWARE_VERSION_MAX_LEN); i++)
	{
		pxAlert->cFirmwareVersionBuffer[i] = szFirmwareVersion[i];
		if (szFirmwareVersion[i] == (char)0)
		{
			break;
		}
	}

	prvDfmAlertReset(pxAlert);

	pxAlert->ucEndMarkers[0] = 0x6D; /* 'm' */
	pxAlert->ucEndMarkers[1] = 0x66; /* 'f' */
	pxAlert->ucEndMarkers[2] = 0x44; /* 'D' */
	pxAlert->ucEndMarkers[3] = 0x50; /* 'P' */

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertSendAll(void)
{
	return prvDfmGetAll(xDfmCloudSendAlert, xDfmCloudSendPayloadChunk);
}

DfmResult_t xDfmAlertGetAll(DfmAlertEntryCallback_t xCallback)
{
	return prvDfmGetAll(xCallback, xCallback);
}

static DfmResult_t prvDfmGetAll(DfmAlertEntryCallback_t xAlertCallback, DfmAlertEntryCallback_t xPayloadCallback)
{
	DfmEntryHandle_t xEntryHandle = 0;
	uint32_t i;
	const char* szSessionId = (void*)0;
	char cSessionIdBuffer[DFM_SESSION_ID_MAX_LEN] = { 0 };
	uint32_t ulAlertId = 0;
	void* pvBuffer = (void*)0;
	uint32_t ulBufferSize = 0;
	uint32_t alertCount = 0;

	if (pxDfmAlertData == (void*)0)
	{
		return DFM_FAIL;
	}

	if (pxDfmAlertData->ulInitialized == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (xAlertCallback == 0)
	{
		return DFM_FAIL;
	}

	if (xPayloadCallback == 0)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetBuffer(&pvBuffer, &ulBufferSize) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	while (xDfmStorageGetAlert(pvBuffer, ulBufferSize) == DFM_SUCCESS)
	{
		if (xDfmEntryCreateAlertFromBuffer(&xEntryHandle) == DFM_FAIL)
		{
			return DFM_FAIL;
		}

		if (xAlertCallback(xEntryHandle) == DFM_FAIL)
		{
			return DFM_FAIL;
		}

		if (xDfmEntryGetSessionId(xEntryHandle, &szSessionId) == DFM_FAIL)
		{
			return DFM_FAIL;
		}

		if (xDfmEntryGetAlertId(xEntryHandle, &ulAlertId) == DFM_FAIL)
		{
			return DFM_FAIL;
		}

		/* Create local copy of sessionId since the buffer containing it WILL be overwritten! */
		for (i = 0; i < sizeof(cSessionIdBuffer); i++)
		{
			cSessionIdBuffer[i] = szSessionId[i];

			if (cSessionIdBuffer[i] == (char)0)
			{
				break;
			}
		}

		while (xDfmStorageGetPayloadChunk(cSessionIdBuffer, ulAlertId, pvBuffer, ulBufferSize) == DFM_SUCCESS)
		{
			if (xDfmEntryCreatePayloadChunkFromBuffer(cSessionIdBuffer , ulAlertId, &xEntryHandle) == DFM_FAIL)
			{
				return DFM_FAIL;
			}

			if (xPayloadCallback(xEntryHandle) == DFM_FAIL)
			{
				return DFM_FAIL;
			}
		}

		alertCount++;
	}

	if (alertCount == 0)
	{
		/* StoragePort had no valid alerts. No obvious error.  */
		return DFM_NO_ALERTS;
	}

	return DFM_SUCCESS;
}

static void prvDfmAlertReset(DfmAlert_t* pxAlert)
{
	uint32_t i;
	
	pxAlert->ucVersion = 0;

	pxAlert->ulAlertType = 0;

	for (i = (uint32_t)0; i < (uint32_t)(DFM_CFG_MAX_SYMPTOMS); i++)
	{
		pxAlert->xSymptoms[i].ulId = 0;
		pxAlert->xSymptoms[i].ulValue = 0;
	}
	pxAlert->ucSymptomCount = 0;

	pxAlert->cAlertDescription[0] = (char)0;

	for (i = (uint32_t)0; i < (uint32_t)(DFM_CFG_MAX_PAYLOADS); i++)
	{
		pxDfmAlertData->xPayloads[i].pvData = (void*)0;
		pxDfmAlertData->xPayloads[i].ulSize = 0;
		pxDfmAlertData->xPayloads[i].cDescriptionBuffer[0] = (char)0;
	}
	pxDfmAlertData->ulPayloadCount = 0;

	pxAlert->ulChecksum = 0;
}

static DfmResult_t prvDfmProcessAlert(DfmAlertEntryCallback_t xAlertCallback, DfmAlertEntryCallback_t xPayloadCallback)
{
	uint16_t j, usChunkCount;
	uint32_t i, ulOffset, ulChunkSize;
	DfmEntryHandle_t xEntryHandle = 0;
	DfmAlert_t* pxAlert = &pxDfmAlertData->xAlert;

	if (xAlertCallback == 0) /*cstat !MISRAC2012-Rule-14.3_b This is a sanity check. It should never fail, but it will crash hard if someoone has made a mistake and we remove this check.*/
	{
		return DFM_FAIL;
	}

	if (xPayloadCallback == 0) /*cstat !MISRAC2012-Rule-14.3_b This is a sanity check. It should never fail, but it will crash hard if someoone has made a mistake and we remove this check.*/
	{
		return DFM_FAIL;
	}

	if (xDfmEntryCreateAlert((DfmAlertHandle_t)pxAlert, &xEntryHandle) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xAlertCallback(xEntryHandle) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	for (i = 0; i < pxDfmAlertData->ulPayloadCount; i++)
	{
		/* We attempt to store the payloads, but if they fail there's not much we can do about it */
		usChunkCount = (uint16_t)(((pxDfmAlertData->xPayloads[i].ulSize - 1UL) / (uint32_t)(DFM_CFG_MAX_PAYLOAD_CHUNK_SIZE)) + 1UL);
		ulOffset = 0;

		/* First we create the payload header */
		if (xDfmEntryCreatePayloadHeader((DfmAlertHandle_t)pxAlert, (uint16_t)(i + 1UL), pxDfmAlertData->xPayloads[i].ulSize, pxDfmAlertData->xPayloads[i].cDescriptionBuffer, &xEntryHandle) == DFM_FAIL)
		{
			/* Couldn't create header for this payload, continue to next */
			continue;
		}

		/* Send payload header */
		if (xPayloadCallback(xEntryHandle) == DFM_FAIL)
		{
			/* Payload header wasn't handled, skip the rest */
			return DFM_FAIL;
		}

		for (j = 0; j < usChunkCount; j++)
		{
			ulChunkSize = DFM_CFG_MAX_PAYLOAD_CHUNK_SIZE;
			if (ulChunkSize > pxDfmAlertData->xPayloads[i].ulSize - ulOffset)
			{
				ulChunkSize = pxDfmAlertData->xPayloads[i].ulSize - ulOffset;
			}

			/* TODO: 64-bit compatible */
			if (xDfmEntryCreatePayloadChunk((DfmAlertHandle_t)pxAlert, (uint16_t)(i + 1UL), j + (uint16_t)1, usChunkCount, (void*)((uint32_t)pxDfmAlertData->xPayloads[i].pvData + ulOffset), ulChunkSize, pxDfmAlertData->xPayloads[i].cDescriptionBuffer, &xEntryHandle) == DFM_FAIL) /*cstat !MISRAC2012-Rule-11.6 We need to modify the address by an offset in order to get next payload chunk*/
			{
				/* Couldn't create entry for this payload chunk, continue to next */
				continue;
			}

			if (xPayloadCallback(xEntryHandle) == DFM_FAIL)
			{
				/* Payload chunk wasn't handled, skip the rest */
				return DFM_FAIL;
			}

			ulOffset += ulChunkSize;
		}
	}

	return DFM_SUCCESS;
}

DfmResult_t xDfmAlertGenerateAlertKey(char* cTopicBuffer, uint32_t ulBufferSize, const char* szMQTTPrefix, DfmEntryHandle_t xEntryHandle)
{
	const char* szSessionId = (void*)0;
	const char* szDeviceName = (void*)0;
	uint32_t ulAlertId = (uint32_t)0;
	uint16_t usEntryId = (uint16_t)0;
	uint16_t usType = (uint16_t)0;
	uint16_t usChunkIndex = (uint16_t)0;
	uint16_t usChunkCount = (uint16_t)0;
	int32_t lRetVal;

	if (cTopicBuffer == (void*)0)
	{
		return DFM_FAIL;
	}

	if (ulBufferSize == (uint32_t)0)
	{
		return DFM_FAIL;
	}

	if (xEntryHandle == 0)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetSessionId(xEntryHandle, &szSessionId) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetDeviceName(xEntryHandle, &szDeviceName) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetAlertId(xEntryHandle, &ulAlertId) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetType(xEntryHandle, &usType) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetEntryId(xEntryHandle, &usEntryId) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetChunkIndex(xEntryHandle, &usChunkIndex) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (xDfmEntryGetChunkCount(xEntryHandle, &usChunkCount) == DFM_FAIL)
	{
		return DFM_FAIL;
	}

	if (szMQTTPrefix == (void*)0)
	{
		/* Set it to empty */
		szMQTTPrefix = "";
	}

	/* "<PREFIX>DevAlert/<DEVICE_NAME>/<UNIQUE_SESSION_ID>/<TRACE_COUNTER>/<SLICE_ID>-<TOTAL_EXPECTED_SLICES>_<PAYLOAD_TYPE>" */
	switch (usType)
	{
	case DFM_ENTRY_TYPE_ALERT:
		lRetVal = snprintf(cTopicBuffer, ulBufferSize, "%sDevAlert/%s/%s/%ld/%d-%d_da_header", szMQTTPrefix, szDeviceName, szSessionId, ulAlertId, usChunkIndex, usChunkCount);
		break;
	case DFM_ENTRY_TYPE_PAYLOAD_HEADER:
		lRetVal = snprintf(cTopicBuffer, ulBufferSize, "%sDevAlert/%s/%s/%ld/%d-%d_da_payload%d_header", szMQTTPrefix, szDeviceName, szSessionId, ulAlertId, usChunkIndex, usChunkCount, usEntryId);
		break;
	case DFM_ENTRY_TYPE_PAYLOAD:
		lRetVal = snprintf(cTopicBuffer, ulBufferSize, "%sDevAlert/%s/%s/%ld/%d-%d_da_payload%d", szMQTTPrefix, szDeviceName, szSessionId, ulAlertId, usChunkIndex, usChunkCount, usEntryId);
		break;
	default:
		return DFM_FAIL;
		break;
	}

	if ((lRetVal < (int32_t)0) || (lRetVal >= (int32_t)ulBufferSize))
	{
		return DFM_FAIL;
	}

	return DFM_SUCCESS;
}

static uint32_t prvDfmAlertCalculateChecksum(uint8_t* pxData, uint32_t ulSize)
{
	(void)pxData;
	(void)ulSize;

	return 0;
}

#endif
