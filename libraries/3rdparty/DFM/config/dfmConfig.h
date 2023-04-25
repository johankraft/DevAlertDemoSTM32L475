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
 * @brief DFM Configuration
 */

#ifndef DFM_CONFIG_H
#define DFM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Global flag used to completely exclude all DFM functionality from compilation
 */
#define DFM_CFG_ENABLED (1)

/**
 * @brief The firmware version. This needs to be set to differentiate the alerts between versions.
 */

// The revision/version name is provided to DevAlert Dispatcher when downloading an alert. This is used in the "fetch_elf_file.bat" script to fetch the right elf file from the "elf-files" folder.
//#define DFM_CFG_FIRMWARE_VERSION "aws_demos-2023-04-21"

// This revision name has a special meaning in the "fetch_elf_file.bat" script. When using this label, it loads the latest ELF file from eclipse instead of using an archived elf file.
#define DFM_CFG_FIRMWARE_VERSION "v1.0-LatestDevBuild"

/**
 * @brief An identifier of the product type.
 */
#define DFM_CFG_PRODUCTID (12)

#if DFM_CFG_PRODUCTID == 0
#error "DFM_CFG_PRODUCTID needs to be set."
#endif


/* How many bytes to dump from the stack (relative to current stack pointer)*/
#define DFM_CFG_STACKDUMP_SIZE 350

/* How long trace to keep for Tracealyzer */
#define DFM_CFG_TRACEBUFFER_SIZE (1500)

/* How much space is needed to store the alert to flash.
 * This should be a multiple of the flash page size (= 2048 on STM32L475). */
#define DFM_CFG_FLASHSTORAGE_SIZE (3 * 2048)

/* If 1, the alert is not sent to the storage or cloud, but instead written to the serial port using DFM_PRINT_ALERT_DATA.*/
#define DFM_CFG_SERIAL_UPLOAD_ONLY 0

/* If to print diagnostic messages in the console. Any errors are printed in either case. */
#define DFM_CFG_USE_DEBUG_LOGGING 0

/* Prototype for the print function */
extern void vMainUARTPrintString( char * pcString );

/* Update this to match your serial console print function */
#define DFM_PRINT_ERROR(msg) vMainUARTPrintString(msg)

#include "FreeRTOS.h"

/* Should not be needed as prints are from exception handler */
#define DFM_CFG_LOCK_SERIAL()  /* vTaskSuspendAll() */
#define DFM_CFG_UNLOCK_SERIAL()  /* xTaskResumeAll() */

/* Update this to match your serial console print function - Make sure to avoid task-switches here! */
#define DFM_PRINT_ALERT_DATA(msg) vMainUARTPrintString(msg)

#if (DFM_CFG_USE_DEBUG_LOGGING == 1)
	/* Update this to match your serial printf function */
	#define DFM_DEBUG_PRINT(msg) vMainUARTPrintString(msg)
#else
	#define DFM_DEBUG_PRINT(msg)
#endif

extern uint32_t ucDfmUserChecksum(char *ptr, size_t maxlen);

/* This function can be modified if another RTOS is used, see dfmUser.c */
extern char* xcDfmUserGetTaskName(void);

/**
 * Provides the filename (e.g "foo.c") from a full file name with path.
 * This is used for __FILE__ strings (__FILENAME__ isn't available on all compilers)
 * The "+1" is to exclude the last '/' character.
 */
#define DFM_CFG_GET_FILENAME_FROM_PATH(str) strrchr(str, '/')+1

/** 
 * @brief Calculates a checksum from __FILE__ strings, where the checksum calculation ends at zero termination. 
 * Max length 256 just in case. This is intended for the DFM symptoms (numerical). 
 * Using the task address as a symptom isn't recommended as it may change in between builds.
 */
#define DFM_CFG_GET_FILENAME_CHECKSUM(filename) ucDfmUserChecksum(filename, 256)

/**
 * @brief Calculates a checksum from the task name, since symptoms should be numerical (the TCB address may change in between builds). 
 */
#define DFM_CFG_GET_TASKNAME_CHECKSUM ucDfmUserChecksum(xcDfmUserGetTaskName(), configMAX_TASK_NAME_LEN)

/**
 * @brief The maximum size of a "chunk" that will be stored or sent.
 */
#define DFM_CFG_MAX_PAYLOAD_CHUNK_SIZE (1000)

/**
 * @brief The maximum length of the device name.
 */
#define DFM_CFG_DEVICE_NAME_MAX_LEN (32)

/**
 * @brief The size of the buffer to store all entry data.
 */
#define DFM_CFG_ENTRY_BUFFER_SIZE (1200)

/**
 * @brief The maximum number of payloads that can be attached to an alert.
 */
#define DFM_CFG_MAX_PAYLOADS (8)

/**
 * @brief The max number of symptoms for each alert
 */
#define DFM_CFG_MAX_SYMPTOMS (8)

/**
 * @brief The max firmware version string length
 */
#define DFM_CFG_FIRMWARE_VERSION_MAX_LEN (64)

/**
 * @brief The max description string length
 */
#define DFM_CFG_DESCRIPTION_MAX_LEN (64)

/**
 * @brief A value that will be used to create a delay between transfers. Was necessary in certain situations.
 */
#define DFM_CFG_DELAY_BETWEEN_SEND (0)

/**
 * @brief The strategy used for storing alerts/payload. Possible values are:
 *	DFM_STORAGE_STRATEGY_IGNORE			Never store alerts/payloads
 *	DFM_STORAGE_STRATEGY_OVERWRITE		Overwrite old alerts/payloads if full
 *	DFM_STORAGE_STRATEGY_SKIP			Skip if full
 */
#define DFM_CFG_STORAGE_STRATEGY DFM_STORAGE_STRATEGY_IGNORE

 /**
  * @brief The strategy used for sending alerts/payload. Possible values are:
 *	DFM_CLOUD_STRATEGY_OFFLINE			Will not attempt to send alerts/payloads
 *	DFM_CLOUD_STRATEGY_ONLINE			Will attempt to send alerts/payloads
 */
#define DFM_CFG_CLOUD_STRATEGY DFM_CLOUD_STRATEGY_ONLINE

 /**
  * @brief The strategy used for acquiring the unique session ID. Possible values are:
 *	DFM_SESSIONID_STRATEGY_ONSTARTUP	Acquires the unique session ID at startup
 *	DFM_SESSIONID_STRATEGY_ONALERT		Acquires the unique session ID the first time an alert is generated
 */
#define DFM_CFG_SESSIONID_STRATEGY DFM_SESSIONID_STRATEGY_ONALERT

 /**
  * @brief The strategy used for acquiring the device name. Possible values are:
 * 	DFM_DEVICE_NAME_STRATEGY_SKIP		Some devides don't know their names, skip it
 *	DFM_DEVICE_NAME_STRATEGY_ONDEVICE	This device knows its' name, get it
 */
#define DFM_CFG_DEVICENAME_STRATEGY DFM_DEVICE_NAME_STRATEGY_ONDEVICE


#ifdef __cplusplus
}
#endif

#endif /* DFM_CONFIG_H */
