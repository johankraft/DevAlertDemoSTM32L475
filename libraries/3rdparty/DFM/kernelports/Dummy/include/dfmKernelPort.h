/*******************************************************************************
 * DFM Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmKernelPort.h
 *
 * The DUMMY kernel port header.
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

#ifndef DFM_KERNEL_PORT_H
#define DFM_KERNEL_PORT_H

#include "percepio_dfmConfig.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DFM_TRACE_RECORDER_IS_ENABLED (1)

#if (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1))

/*******************************************************************************
 * DFM_KERNEL_PORT_INIT
 *
 * Will be called during initialization of DFM.
 * Map it to a function if something needs to be done.
 *
 * NOTE: Define as (0) if nothing should to be done.
 ******************************************************************************/
#define DFM_KERNEL_PORT_INIT() (0)

/*******************************************************************************
 * DFM_KERNEL_PORT_DELAY
 *
 * Map it to a function that calls on the kernel Delay service.
 *
 * @param ulDelay The time to delay.
 ******************************************************************************/
#define DFM_KERNEL_PORT_DELAY(ulDelay) ((void)ulDelay, 0)

/*******************************************************************************
 * DFM_KERNEL_PORT_GET_TRACE_SIZE
 *
 * Map it to a function that retrieves the trace data size.
 *
 * @param pulSize Set to trace data size.
 ******************************************************************************/
#define DFM_KERNEL_PORT_GET_TRACE_SIZE(pulSize) ((void)pulSize, 0)

/*******************************************************************************
 * DFM_KERNEL_PORT_READ_TRACE
 *
 * Map it to a function that reads the trace data to a buffer and returns said
 * buffer via ppvData. If a buffer is needed to hold the data (data is read
 * from file), call ulDfmGetBuffer(...), read to that buffer, and set *ppvData
 * to that address.
 *
 * NOTE: Define as (0) if nothing should be done.
 *
 * @param ppvData Set to the buffer holding the read trace data.
 * @param ulSize The requested data size.
 * @param ulOffset The read offset. Can be used to offset a memory read pointer.
 * Ignore this parameter if data is read from a file.
 * @param pulBytesRead The amount of trace data that was read.
 ******************************************************************************/
#define DFM_KERNEL_PORT_READ_TRACE(ppvData, ulSize, ulOffset, pulBytesRead) ((void)ppvData, (void)ulSize, (void)ulOffset, (void)pulBytesRead, 0)

/*******************************************************************************
 * DFM_KERNEL_PORT_BEGIN_READ_TRACE
 *
 * Map it to a function that begins trace reading (i.e. opening a file handle).
 *
 * NOTE: Define as (0) if nothing should be done.
 ******************************************************************************/
#define DFM_KERNEL_PORT_BEGIN_READ_TRACE() (0)

/*******************************************************************************
 * DFM_KERNEL_PORT_END_READ_TRACE
 *
 * Map it to a function that ends trace reading (i.e. closing a file handle).
 *
 * NOTE: Define as (0) if nothing should be done.
 ******************************************************************************/
#define DFM_KERNEL_PORT_END_READ_TRACE() (0)

/*******************************************************************************
 * DFM_KERNEL_PORT_TRACE_SUSPEND
 *
 * Suspend trace before sending alert.
 ******************************************************************************/
#define DFM_KERNEL_PORT_TRACE_SUSPEND() (0)

/*******************************************************************************
 * DFM_KERNEL_PORT_TRACE_RESUME
 *
 * Resume trace after sending alert.
 ******************************************************************************/
#define DFM_KERNEL_PORT_TRACE_RESUME() (0)

#endif /* (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1)) */

#ifdef __cplusplus
}
#endif

#endif /* DFM_KERNEL_PORT_H */
