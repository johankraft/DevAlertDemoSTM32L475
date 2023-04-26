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
 * @brief This user-defined function provides the "Session ID" for each alert.
 * This is basically a "restart counter" that is combined with the "Device ID"
 * used to generate unique "alert keys" for each alert.
 *
 * THIS MUST NOT BE A CONSTANT DUMMY STRING, EVEN FOR BASIC TESTING.
 * DUPLICATE ALERT KEYS ARE IGNORED BY DEVALERT.
 *
 * The alert key follows the pattern "DevAlert/DeviceID/SessionID/AlertCounter"
 * and must be unique across all alerts from all devices, over all time.
 *
 * The Alert Counter is assumed to be reset when the device is restarted.
 *
 * For production use, it is NOT ADVICED to use a RANDOM number, like below,
 * since this may repeat.
 *
 * The recommended practice is to use one of these methods:
 * - The current time ("wall-clock" time), e.g. in seconds since a certain date
 * - A restart counter, stored in non-volatile storage.
 *
 * If using the current time as Session ID, seconds is sufficient resolution
 * unless a restart and re-initialization may occur faster than that.
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
 * @brief This user-defined function provides the "Device ID" for each alert.
 * This is combined with the "Session ID" to generate unique "alert keys" for each alert.
 *
 * THIS MUST NOT BE A CONSTANT DUMMY STRING. DUPLICATE ALERT KEYS ARE IGNORED BY DEVALERT.
 *
 * In this demo for the STM32L475, we use the processor serial number as Device ID.
 *
 * If using AWS IoT Core, note that Device ID does not need to match the Thing Name
 * (i.e. clientcredentialIOT_THING_NAME in /demos/include/aws_clientcredentials.h).
 * It is good practice and recommended to have matching Thing Name and Device ID, but that
 * was difficuly to implement using the coreMQTT library since clientcredentialIOT_THING_NAME
 * is assumed to be a constant string.
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
