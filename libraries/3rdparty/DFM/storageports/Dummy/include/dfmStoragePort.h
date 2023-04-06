#ifndef DFM_STORAGE_PORT_H
#define DFM_STORAGE_PORT_H

#include <stdint.h>
#include <dfmConfig.h>
#include <dfmTypes.h>

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DfmStoragePortData
{
	uint32_t ulInitialized;
} DfmStoragePortData_t;

DfmResult_t xDfmStoragePortInitialize(DfmStoragePortData_t *pxBuffer);

DfmResult_t xDfmStoragePortStoreSession(void* pvData, uint32_t ulSize);
DfmResult_t xDfmStoragePortGetSession(void* pvBuffer, uint32_t ulBufferSize);

DfmResult_t xDfmStoragePortStoreAlert(DfmEntryHandle_t xEntryHandle, uint32_t ulOverwrite);
DfmResult_t xDfmStoragePortGetAlert(void* pvBuffer, uint32_t ulBufferSize);

DfmResult_t xDfmStoragePortStorePayloadChunk(DfmEntryHandle_t xEntryHandle, uint32_t ulOverwrite);
DfmResult_t xDfmStoragePortGetPayloadChunk(char* szSessionId, uint32_t ulAlertId, void* pvBuffer, uint32_t ulBufferSize);

DfmResult_t dfmStoragePortReset(int isNewAlert);

#ifdef __cplusplus
}
#endif

#endif

#endif
