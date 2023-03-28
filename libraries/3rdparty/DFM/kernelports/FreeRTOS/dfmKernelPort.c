/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * FreeRTOS Kernel port
 */

#include "dfm.h"

#if ((DFM_CFG_ENABLED) >= 1)

static DfmKernelPortData_t *pxKernelPortData = (void*)0;

DfmResult_t xDfmKernelPortInitialize(DfmKernelPortData_t *pxBuffer)
{
	if (pxBuffer == (void*)0)
	{
		return DFM_FAIL;
	}

	pxKernelPortData = pxBuffer;

	return DFM_SUCCESS;
}

#endif
