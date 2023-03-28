/*******************************************************************************
 * DFM Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmCloudPortConfig.h
 *
 * The AWS cloud port configuration.
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

#ifndef DFM_CLOUD_PORT_CONFIG_H
#define DFM_CLOUD_PORT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Define represents RTT payload size  */
#define DFM_CFG_CLOUD_PORT_MAX_PAYLOAD_SIZE (1000)

/* The maximum topic size */
#define DFM_CFG_CLOUD_PORT_MAX_TOPIC_SIZE (64)

/*******************************************************************************
 * DFM_CFG_CLOUD_RTT_BUFFER_SIZE_UP
 *
 * Defines the size of the "up" RTT buffer (target -> host) to use for writing
 * the trace data, for RTT buffer 1 or higher.
 *
 * This setting is ignored for RTT buffer 0, which can't be reconfigured
 * in runtime and therefore hard-coded to use the defines in SEGGER_RTT_Conf.h.
 *
 * Default buffer size for Tracealyzer is 5000 bytes.
 *
 * If you have a stand-alone J-Link probe, the can be decreased to around 1 KB.
 * But integrated J-Link OB interfaces are slower and needs about 5-10 KB,
 * depending on the amount of data produced.
 ******************************************************************************/
#define DFM_CFG_CLOUD_RTT_BUFFER_SIZE_UP 5000

/*******************************************************************************
 * DFM_CFG_CLOUD_RTT_BUFFER_SIZE_DOWN
 *
 * Defines the size of the "down" RTT buffer (host -> target) to use for reading
 * commands from Tracealyzer, for RTT buffer 1 or higher.
 *
 * Default buffer size for Tracealyzer is 32 bytes.
 *
 * This setting is ignored for RTT buffer 0, which can't be reconfigured
 * in runtime and therefore hard-coded to use the defines in SEGGER_RTT_Conf.h.
 ******************************************************************************/
#define DFM_CFG_CLOUD_RTT_BUFFER_SIZE_DOWN 64

/*******************************************************************************
 * DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX
 *
 * Defines the RTT buffer to use for writing the trace data from target to PC.
 *
 * Default: 1
 *
 * We don't recommend using RTT buffer 0, since mainly intended for terminals.
 * If you prefer to use buffer 0, it must be configured in SEGGER_RTT_Conf.h.
 ******************************************************************************/
#define DFM_CFG_CLOUD_RTT_UP_BUFFER_INDEX 1

/*******************************************************************************
 * DFM_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX
 *
 * Defines the RTT buffer to use for sending configuration data from PC to target.
 *
 * Default: 1
 *
 * We don't recommend using RTT buffer 0, since mainly intended for terminals.
 * If you prefer to use buffer 0, it must be configured in SEGGER_RTT_Conf.h.
 ******************************************************************************/
#define DFM_CFG_CLOUD_RTT_DOWN_BUFFER_INDEX 1

/*******************************************************************************
 * DFM_CFG_CLOUD_RTT_MODE
 * This stream port for J-Link streaming relies on SEGGER RTT, that contains an
 * internal RAM buffer read by the J-Link probes during execution.
 *
 * Possible values:
 *	SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL
 *	SEGGER_RTT_MODE_NO_BLOCK_SKIP (default)
 *
 * Using SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL ensure that you get a
 * complete and valid trace. This may however cause blocking if your streaming
 * interface isn't fast enough, which may disturb the real-time behavior.
 *
 * We therefore recommend SEGGER_RTT_MODE_NO_BLOCK_SKIP. In this mode,
 * Tracealyzer will report lost events if the transfer is not
 * fast enough. In that case, try increasing the size of the "up buffer".
 ******************************************************************************/
#define DFM_CFG_CLOUD_RTT_MODE SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL

/*******************************************************************************
 * DFM_CFG_CLOUD_RTT_CONNECTION_TYPE
 * Defines how to react when an Alert is triggered and there is no RTT
 * connection.
 * Possible values:
 *	DFM_CLOUD_RTT_CONNECTION_WAIT       When an alert is triggered, it will hang
 *                                		until an RTT connection is confirmed.
 *	DFM_CLOUD_RTT_CONNECTION_FAIL       When an alert is triggered, it will fail
 *										if no RTT connection is confirmed. This
 *										will disable DFM for future alerts.
 *	DFM_CLOUD_RTT_CONNECTION_CONTINUE	When an alert is triggered, it will
 *										silently fail if no RTT connection is
 *										confirmed. Will try again upon next
 *										alert.
 ******************************************************************************/
#define DFM_CFG_CLOUD_RTT_CONNECTION_TYPE DFM_CLOUD_RTT_CONNECTION_WAIT

/*******************************************************************************
 * DFM_CFG_CLOUD_RTT_MAX_SESSIONID_ATTEMPTS
 * The number of times it will attempt to read a sessionID from RTT before 
 * giving up. Will only be used if DFM_CFG_CLOUD_RTT_CONNECTION_TYPE is
 * not set to DFM_CLOUD_RTT_CONNECTION_WAIT.
 ******************************************************************************/
#define DFM_CFG_CLOUD_RTT_MAX_SESSIONID_ATTEMPTS 1000000

#ifdef __cplusplus
}
#endif

#endif /* DFM_CLOUD_PORT_CONFIG_H */
