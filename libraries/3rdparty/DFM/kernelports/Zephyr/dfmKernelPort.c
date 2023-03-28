/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Zephyr Kernel port
 */

#include <init.h>
#include <kernel.h>
#include <dfm.h>

#if ((DFM_CFG_ENABLED) == 1)

DfmKernelPortData_t* pxKernelPortData;

DfmResult_t xDfmKernelPortInitialize(DfmKernelPortData_t *pxBuffer)
{
	if (pxBuffer == (void*)0)
	{
		return DFM_FAIL;
	}

	pxKernelPortData = pxBuffer;

	return DFM_SUCCESS;
}

/**
 * @brief Initialize aspects of the devalert module that must 
 * preceed the kernel initialization (scheduling, threads, etc.).
 * 
 * @param[in] arg
 */
static int devalert_pre_kernel_init(const struct device *arg)
{
	ARG_UNUSED(arg);

	return 0;
}

/**
 * @brief Initialize aspects of the devalert module that depends on
 * the kernel being initialized.
 * 
 * @param[in] arg
 */
static int devalert_post_kernel_init(const struct device *arg)
{
	ARG_UNUSED(arg);

	xDfmInitialize();

	xDfmSessionSetDeviceName("MY_DEVICE");

	return 0;
}

/* Specify recorder module initialization stages */
SYS_INIT(devalert_pre_kernel_init, PRE_KERNEL_2, 0);
SYS_INIT(devalert_post_kernel_init, POST_KERNEL, 99);

#endif
