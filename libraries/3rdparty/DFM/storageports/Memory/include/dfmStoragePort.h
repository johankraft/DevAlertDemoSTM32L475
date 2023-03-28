/*******************************************************************************
 * DFM Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmStoragePort.h
 *
 * The Memory storage port header, not using non-volatile memory.
 *
 * NOTE! This storage port is only intended for demo, test, or local
 * development. A real storage port, using actual non-volatile memory,
 * must be used for production environments.
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

#ifndef DFM_STORAGE_PORT_H
#define DFM_STORAGE_PORT_H

#include "percepio_dfm.h"

#if (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1))

#if (defined(DFM_CFG_STORAGE_ENABLED) && ((DFM_CFG_STORAGE_ENABLED) >= 1))
    #error "Do not use the Memory storage port when #define DFM_CFG_STORAGE_ENABLED is enabled"
#else /* (defined(DFM_CFG_STORAGE_ENABLED) && ((DFM_CFG_STORAGE_ENABLED) >= 1)) */
#endif /* (defined(DFM_CFG_STORAGE_ENABLED) && ((DFM_CFG_STORAGE_ENABLED) >= 1)) */

#ifdef __cplusplus
extern "C" {
#endif

/* ulDfmStoragePortHasData() checks if the section's first uint32_t is 0 */
uint32_t ulDfmStoragePortHasData(uint32_t ulSection, uint32_t* pulFlag);

/* ulDfmStoragePortClearData() sets the section's first uint32_t to 0 */
uint32_t ulDfmStoragePortClearData(uint32_t ulSection);

/* ulDfmStoragePortWriteFlash() writes data to non-volatile flash */
uint32_t ulDfmStoragePortWrite(uint32_t ulSection, void* pvData, uint32_t ulSize, uint32_t ulOffset, uint32_t* pulBytesWritten);

/* ulDfmStoragePortReadFlash() reads data from non-volatile flash */
uint32_t ulDfmStoragePortRead(uint32_t ulSection, void** ppvData, uint32_t ulSize, uint32_t ulOffset, uint32_t* pulBytesRead);

/*******************************************************************************
 * DFM_STORAGE_PORT_INIT
 *
 * Will be called during initialization of DFM.
 * Map it to a function if something needs to be done.
 *
 * NOTE: Define as (0) if nothing should to be done.
 ******************************************************************************/
#define DFM_STORAGE_PORT_INIT() (0)

/*******************************************************************************
 * DFM_STORAGE_PORT_BEGIN_WRITE
 *
 * Map it to a function that prepares for writing (i.e. opening file handle).
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ulSection The section that will be written.
 ******************************************************************************/
#define DFM_STORAGE_PORT_BEGIN_WRITE(ulSection) ((void)ulSection, 0)

/*******************************************************************************
 * DFM_STORAGE_PORT_END_WRITE
 *
 * Map it to a function that cleans up after writing (i.e. closing file handle).
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ulSection The section that was written.
 ******************************************************************************/
#define DFM_STORAGE_PORT_END_WRITE(ulSection) ((void)ulSection, 0)

/*******************************************************************************
 * DFM_STORAGE_PORT_BEGIN_READ
 *
 * Map it to a function that prepares for reading (i.e. opening file handle).
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ulSection The section that will be read.
 ******************************************************************************/
#define DFM_STORAGE_PORT_BEGIN_READ(ulSection) ((void)ulSection, 0)

/*******************************************************************************
 * DFM_STORAGE_PORT_END_READ
 *
 * Map it to a function that cleans up after reading (i.e. closing file handle).
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ulSection The section that was read.
 ******************************************************************************/
#define DFM_STORAGE_PORT_END_READ(ulSection) ((void)ulSection, 0)

/*******************************************************************************
 * DFM_STORAGE_PORT_HAS_DATA
 *
 * Map to a function that tells if Non-Volatile data is available.
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ulSection The section to check for data.
 ******************************************************************************/
#define DFM_STORAGE_PORT_HAS_DATA(ulSection, pulFlag) ulDfmStoragePortHasData(ulSection, pulFlag)

/*******************************************************************************
 * DFM_STORAGE_PORT_CLEAR_DATA
 *
 * Map to a function that cleans up Non-Volatile data (delete file, set flash
 * area to 0).
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ulSection The section to clear.
 ******************************************************************************/
#define DFM_STORAGE_PORT_CLEAR_DATA(ulSection) ulDfmStoragePortClearData(ulSection)

/*******************************************************************************
 * DFM_STORAGE_PORT_WRITE_NONVOLATILE_MEMORY
 *
 * Map to a function that writes data to Non-Volatile memory.
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ulSection The section to write.
 * @param pvData The data.
 * @param ulSize The data size.
 * @param ulOffset Offset that can be used to determine where in memory to
 * write the data. If data is written to file, this parameter can be ignored.
 * @param pulBytesWritten The amount of bytes that were written.
 ******************************************************************************/
#define DFM_STORAGE_PORT_WRITE_NONVOLATILE_MEMORY(ulSection, pvData, ulSize, ulOffset, pulBytesWritten) ulDfmStoragePortWrite(ulSection, pvData, ulSize, ulOffset, pulBytesWritten)

/*******************************************************************************
 * DFM_STORAGE_PORT_READ_NONVOLATILE_MEMORY
 *
 * Map to a function that reads data from Non-Volatile memory. If a buffer is 
 * needed to hold the data (data is read from file), call ulDfmGetBuffer(...), 
 * read to that buffer, and set *ppvData to that address.
 *
 * NOTE: Define as (0) if nothing should to be done.
 *
 * @param ulSection The section to write.
 * @param ppvData Set this to a data buffer (*ppvData = pvReadDataBuffer).
 * @param ulSize The requested data size.
 * @param ulOffset Offset that can be used to determine where in memory to read
 * the data. If data is read from file, this parameter can be ignored.
 * @param pulBytesRead The amount of bytes that were read.
 ******************************************************************************/
#define DFM_STORAGE_PORT_READ_NONVOLATILE_MEMORY(ulSection, ppvData, ulSize, ulOffset, pulBytesRead) ulDfmStoragePortRead(ulSection, ppvData, ulSize, ulOffset, pulBytesRead)

#ifdef __cplusplus
}
#endif

#endif /* (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1)) */

#endif /* DFM_STORAGE_PORT_H */
