/*
 * Percepio DFM v2.0.0
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief DFM User API
 */

#ifndef DFM_USER_H
#define DFM_USER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if ((DFM_CFG_ENABLED) >= 1)

/**
 * @defgroup dfm_user_apis DFM User API
 * @ingroup dfm_apis
 * @{
 */

/**
* @brief User implemented function that is expected to write a Device unique SessionId to cUniqueSessionIdBuffer
*
* @param[in] cUniqueSessionIdBuffer The buffer that will contain the SessionId.
* @param[in] ulSize Buffer size.
* @param[out] pulBytesWritten The number of bytes written to the buffer.
*
* @retval DFM_FAIL Failure
* @retval DFM_SUCCESS Success
*/
DfmResult_t xDfmUserGetUniqueSessionID(char cUniqueSessionIdBuffer[], uint32_t ulSize, uint32_t* pulBytesWritten);

/**
* @brief User implemented function that is expected to write a Device name to cDeviceNameBuffer
*
* @param[in] cDeviceNameBuffer The buffer that will contain the SessionId.
* @param[in] ulSize Buffer size.
* @param[out] pulBytesWritten The number of bytes written to the buffer.
*
* @retval DFM_FAIL Failure
* @retval DFM_SUCCESS Success
*/
DfmResult_t xDfmUserGetDeviceName(char cDeviceNameBuffer[], uint32_t ulSize, uint32_t* pulBytesWritten);

#endif

#ifdef __cplusplus
}
#endif

#endif
