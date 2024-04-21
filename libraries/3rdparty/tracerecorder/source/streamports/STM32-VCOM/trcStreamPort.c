/*
 * Trace Recorder for Tracealyzer v4.8.2
 * Copyright 2023 Percepio AB
 * www.percepio.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Supporting functions for trace streaming, used by the "stream ports" 
 * for reading and writing data to the interface.
 * Existing ports can easily be modified to fit another setup, e.g., a 
 * different TCP/IP stack, or to define your own stream port.
 *
 * This stream port is for UART streaming, tested with the VCOM port on STM32/STLINK.
 */

#include <trcRecorder.h>

#if (TRC_USE_TRACEALYZER_RECORDER == 1)

#if (TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)

static TraceStreamPortBuffer_t* pxStreamPortITM TRC_CFG_RECORDER_DATA_ATTRIBUTE;

/* ST HAL API include */
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_uart.h"
#include "stm32l475e_iot01.h"

/* common-io UART include */
#include "iot_uart.h"

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

extern IotUARTHandle_t xConsoleUart;

traceResult xTraceStreamPortInitialize(TraceStreamPortBuffer_t* pxBuffer)
{
	TRC_ASSERT_EQUAL_SIZE(TraceStreamPortBuffer_t, TraceStreamPortITM_t);

	TRC_ASSERT(pxBuffer != 0);

	pxStreamPortITM = (TraceStreamPortBuffer_t*)pxBuffer;

#if (TRC_USE_INTERNAL_BUFFER == 1)
	return xTraceInternalEventBufferInitialize(pxStreamPortITM->bufferInternal, sizeof(pxStreamPortITM->bufferInternal));
#else
        /* Prevents a warning for set but not used when internal buffer
         * isn't used. */
        (void)pxStreamPortITM;
        
	return TRC_SUCCESS;
#endif
}



/* This is assumed to execute from within the recorder, with interrupts disabled */
traceResult prvTraceItmWrite(void* ptrData, uint32_t size, int32_t* ptrBytesWritten)
{
	uint32_t* ptr32 = (uint32_t*)ptrData;
	int32_t status;

	TRC_ASSERT(size % 4 == 0);
	TRC_ASSERT(ptrBytesWritten != 0);

	*ptrBytesWritten = 0;

	if (size == 0)
		return TRC_SUCCESS;

	status = iot_uart_write_async( xConsoleUart, (uint8_t * const) ptrData, size);

	if (status == IOT_UART_SUCCESS)
	{
		extern void iot_uart_wait_johan(IotUARTHandle_t const pxUartPeripheral);

		iot_uart_wait_johan(xConsoleUart);

		*ptrBytesWritten = size;
		return TRC_SUCCESS;
	}

	if (status == IOT_UART_BUSY)
	{
		*ptrBytesWritten = 0; // Assuming TzCtrl will try again...
		return TRC_SUCCESS;
	}

	if (status == IOT_UART_WRITE_FAILED)
	{
		for(;;);
	}

	/* This works, both with "direct mode" and with internal buffer. Baud rate 2000000 works. Allows for up to 60 KB/s in Tz.
	 * But trying iot_uart_write_async to reduce CPU load. Tz is getting data but not getting the header, doesn't show the trace.
	 * Next step: Capture data from xTraceEnable in TeraTerm, to a binary file.
	 * Compare data between iot_uart_write_sync and iot_uart_write_async
	 *
	 * Conclusions:
	 * - No difference between headers when running slow, 115200.
	 * - STLINK generally unstable when pushing the limit (2000000 baud) - debugger shuts down after a while
	 * - 921600 seems to work, also with the async transfer.
	 * - Also need to restart Tz in between each run to ensure it starts properly.
	 * - 1497600 works quite well, super stable up to 60-70 KB/s. At 90 KB/s there was a single error after 30 min or so.
	 *
	 * */

	return TRC_SUCCESS;
}

/* This reads "command" data from a RAM buffer, written by a host macro in the debugger */
traceResult prvTraceItmRead(void* ptrData, uint32_t uiSize, int32_t* piBytesRead)
{
	int32_t i;
	uint8_t* bytesBuffer = (uint8_t*)ptrData;

	TRC_ASSERT(piBytesRead != 0);

	*piBytesRead = 0;

	return TRC_SUCCESS;
}




#endif

#endif
