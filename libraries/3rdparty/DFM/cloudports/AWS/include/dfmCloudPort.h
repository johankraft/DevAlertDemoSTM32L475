/*******************************************************************************
 * DFM Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmCloudPort.h
 *
 * The AWS cloud port header.
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

#ifndef DFM_CLOUD_PORT_H
#define DFM_CLOUD_PORT_H

#include "dfmCloudPortConfig.h"

#include "percepio_dfm.h"

#if (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED))

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ulDfmAWSPortInit(void);

uint32_t ulDfmAWSPortSend(const void* pvData, uint32_t ulSize, const char* szUniqueSessionID, uint32_t ulTraceCounter, const char* szType, uint32_t ulSlice, uint32_t ulSlices, uint32_t* pulBytesSent);

/*******************************************************************************
 * DFM_CLOUD_PORT_INIT
 *
 * Will be called during initialization of DFM.
 * Map it to a function if something needs to be done.
 *
 * NOTE: Define as (0) if nothing should to be done.
 ******************************************************************************/
#define DFM_CLOUD_PORT_INIT() ulDfmAWSPortInit()

/*******************************************************************************
 * DFM_CLOUD_PORT_SEND
 *
 * Map it to a function that sends data to the cloud.
 *
 * @param pvData The data.
 * @param ulSize The data size.
 * @param szUniqueSessionID A unique Session ID to tell "reboots" apart.
 * @param ulTraceCounter The trace counter, since many traces may be generated
 * by a single Session.
 * @param szType The type of slice to be sent.
 * @param ulSlice The slice ID.
 * @param ulSlices The number of slices to expect.
 * @param pulBytesSent The number of bytes that were sent.
 ******************************************************************************/
#define DFM_CLOUD_PORT_SEND(pvData, ulSize, szUniqueSessionID, ulTraceCounter, szType, ulSlice, ulSlices, pulBytesSent) ulDfmAWSPortSend(pvData, ulSize, szUniqueSessionID, ulTraceCounter, szType, ulSlice, ulSlices, pulBytesSent)

#ifdef __cplusplus
}
#endif

#endif /* (defined(DFM_CFG_ENABLED) && (DFM_CFG_ENABLED >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && (DFM_TRACE_RECORDER_IS_ENABLED)) */

#endif /* DFM_CLOUD_PORT_H */
