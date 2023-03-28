/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * DFM User
 */

#include <dfm.h>
#include <stdio.h>

#if ((DFM_CFG_ENABLED) >= 1)

extern uint32_t hardware_rand(void);

DfmResult_t xDfmUserGetUniqueSessionID(char cUniqueSessionIdBuffer[], uint32_t ulSize, uint32_t* pulBytesWritten)
{
	*pulBytesWritten = snprintf(cUniqueSessionIdBuffer, ulSize, "%u", (unsigned int) hardware_rand());
	return DFM_SUCCESS;
}

DfmResult_t xDfmUserGetDeviceName(char cDeviceNameBuffer[], uint32_t ulSize, uint32_t* pulBytesWritten)
{
//    *pulBytesWritten = snprintf(cDeviceNameBuffer, ulSize, "%s", "...");
    #error
    return DFM_SUCCESS;
}

#endif
