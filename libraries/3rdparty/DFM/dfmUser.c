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

#include "aws_clientcredential.h"

#if ((DFM_CFG_ENABLED) >= 1)

extern uint32_t hardware_rand(void);

DfmResult_t xDfmUserGetUniqueSessionID(char cUniqueSessionIdBuffer[], uint32_t ulSize, uint32_t* pulBytesWritten)
{
	// The session ID is used to provide a unique "alert key" to the DevAlert service.
	// THIS MUST NOT BE A CONSTANT DUMMY STRING.

	// The alert key must be unique across all alerts from all devices, over all time, since the DevAlert
	// cloud service ignores repeated alerts with the same alert key.

	// This since some upload methods like MQTT may produce duplicate messages, depending on QoS level.
	// Or some software bug might cause the device to resend the same alert over and over, resulting in many redundant messages.

	// The alert key includes the device ID (assumed unique), a session ID and an alert counter within the current session.

	// For production use, the session ID must not repeat within the alerts from a specific device.
	// THEREFORE, It is NOT ADVICED to use a RANDOM number like below.
	// This is only OK for demos like this, using a hardware-based true random number generator.

	// For production use, the session ID is ideally a persistent counter incremented each time the device starts up.
	// This could be stored in internal flash or in other device storage. Another way is to use the current time for
	// the session ID, if this is available in the device, e.g. using the time() function in time.h.
	// With AWS IoT Core it is also possible to store data as persistent MQTT messages, which could be
	// used for this purpose. See https://docs.aws.amazon.com/iot/latest/developerguide/mqtt.html#mqtt-retain.

	*pulBytesWritten = snprintf(cUniqueSessionIdBuffer, ulSize, "%u", (unsigned int) hardware_rand());
	return DFM_SUCCESS;
}

DfmResult_t xDfmUserGetDeviceName(char cDeviceNameBuffer[], uint32_t ulSize, uint32_t* pulBytesWritten)
{
	// Sets clientcredentialIOT_THING_NAME as device name, defined in aws_clientcredential.h
	*pulBytesWritten = snprintf(cDeviceNameBuffer, ulSize, "%s", clientcredentialIOT_THING_NAME);
    return DFM_SUCCESS;
}

#endif
