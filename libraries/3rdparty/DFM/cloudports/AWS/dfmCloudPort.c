/*******************************************************************************
 * DFM Library v989.878.767
 * Percepio AB, www.percepio.com
 *
 * dfmCloudPort.c
 *
 * The AWS cloud port implementation.
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

#include "dfmCloudPort.h"

#if (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1))

#include "aws_clientcredential.h"
/* MQTT library includes. */
#include "core_mqtt.h"
#include "transport_secure_sockets.h"

/* TODO: Can we expect a string.h to be present? */
#include <string.h>

#define RETRY_BACKOFF_MS			   (500U)
#define MILLISECONDS_PER_TICK		   (1000U / configTICK_RATE_HZ)
#define KEEP_ALIVE_TIMEOUT_SECONDS	   (1800U) /* KEEP_ALIVE = 0 means disabled */
#define CONNACK_RECV_TIMEOUT_MS		   (1000U)
#define TRANSPORT_SEND_RECV_TIMEOUT_MS (500U)
/**
 * @brief Size of the network buffer for MQTT packets.
 */
#define NETWORK_BUFFER_SIZE (1024U)

char cTopicBuffer[DFM_CFG_CLOUD_PORT_MAX_TOPIC_SIZE] = {0};
static uint32_t ulGlobalEntryTimeMs;

static MQTTContext_t xMQTTContext = {0};
static NetworkContext_t xNetworkContext = {0};

/**
 * @brief Static buffer used to hold MQTT messages being sent and received.
 */
static uint8_t ucSharedBuffer[NETWORK_BUFFER_SIZE];
/** @brief Static buffer used to hold MQTT messages being sent and received. */
static MQTTFixedBuffer_t xBuffer =
	{
		ucSharedBuffer,
		NETWORK_BUFFER_SIZE};

static uint32_t prvMqttConnect();
static uint8_t prvCheckConnection();

/**
 * @brief The timer query function provided to the MQTT context.
 *
 * @return Time in milliseconds.
 */
static uint32_t prvGetTimeMs(void)
{
	TickType_t xTickCount = 0;
	uint32_t ulTimeMs = 0UL;

	/* Get the current tick count. */
	xTickCount = xTaskGetTickCount();

	/* Convert the ticks to milliseconds. */
	ulTimeMs = (uint32_t)xTickCount * MILLISECONDS_PER_TICK;

	/* Reduce ulGlobalEntryTimeMs from obtained time so as to always return the
	 * elapsed time in the application. */
	ulTimeMs = (uint32_t)(ulTimeMs - ulGlobalEntryTimeMs);

	return ulTimeMs;
}

/**
 * @brief The dummy event function provided to the MQTT context.
 *
 */
static void prvEventCallback(MQTTContext_t* pMqttContext,
							 MQTTPacketInfo_t* pPacketInfo,
							 MQTTDeserializedInfo_t* pDeserializedInfo)
{
	(void)pMqttContext;
	(void)pPacketInfo;
	(void)pDeserializedInfo;
}

uint32_t ulDfmAWSPortInit()
{
	ulGlobalEntryTimeMs = 0;
	ulGlobalEntryTimeMs = prvGetTimeMs();

	MQTTStatus_t xResult;
	TransportInterface_t xTransport;
	ServerInfo_t xServerInfo = {0};
	SocketsConfig_t xSocketsConfig = {0};
	/* Set the credentials for establishing a TLS connection. */
	/* Initializer server information. */
	xServerInfo.pHostName = clientcredentialMQTT_BROKER_ENDPOINT;
	xServerInfo.hostNameLength = strlen(clientcredentialMQTT_BROKER_ENDPOINT);
	xServerInfo.port = clientcredentialMQTT_BROKER_PORT;

	/* Configure credentials for TLS mutual authenticated session. */
	xSocketsConfig.enableTls = true;
	xSocketsConfig.pAlpnProtos = NULL;
	xSocketsConfig.maxFragmentLength = 0;
	xSocketsConfig.disableSni = false;
	xSocketsConfig.pRootCa = NULL;
	xSocketsConfig.rootCaSize = 0;
	xSocketsConfig.sendTimeoutMs = TRANSPORT_SEND_RECV_TIMEOUT_MS;
	xSocketsConfig.recvTimeoutMs = TRANSPORT_SEND_RECV_TIMEOUT_MS;
	TransportSocketStatus_t xNetworkStatus = TRANSPORT_SOCKET_STATUS_SUCCESS;
	uint8_t retries = 3;
	/* Attempt to connect to MQTT broker. If connection fails, retry after
	 * a timeout.
	 */
	do
	{
		/* Establish a TLS session with the MQTT broker. */
		xNetworkStatus = SecureSocketsTransport_Connect(&xNetworkContext,
														&xServerInfo,
														&xSocketsConfig);

		if (xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS)
		{
			if (--retries == 0)
			{
				return -2;
			}
			/* As the connection attempt failed, we will retry the connection */
			DFM_KERNEL_PORT_DELAY(RETRY_BACKOFF_MS);
		}
	} while ((xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS));

	/* Fill in Transport Interface send and receive function pointers. */
	xTransport.pNetworkContext = &xNetworkContext;
	xTransport.send = SecureSocketsTransport_Send;
	xTransport.recv = SecureSocketsTransport_Recv;

	/* Initialize MQTT library. */
	xResult = MQTT_Init(&xMQTTContext, &xTransport, prvGetTimeMs, prvEventCallback, &xBuffer);

	if (xResult != MQTTSuccess)
	{
		return -1;
	}
	return prvMqttConnect();
}

static uint32_t prvMqttConnect()
{
	MQTTStatus_t xResult;
	MQTTConnectInfo_t xConnectInfo;
	/* Some fields are not used so start with everything at 0. */
	(void)memset((void*)&xConnectInfo, 0x00, sizeof(xConnectInfo));

	/* Start with a clean session i.e. direct the MQTT broker to discard any
	 * previous session data. Also, establishing a connection with clean session
	 * will ensure that the broker does not store any data when this client
	 * gets disconnected. */
	xConnectInfo.cleanSession = true;

	/* The client identifier is used to uniquely identify this MQTT client to
	 * the MQTT broker. In a production device the identifier can be something
	 * unique, such as a device serial number. */
	xConnectInfo.pClientIdentifier = clientcredentialIOT_THING_NAME;
	xConnectInfo.clientIdentifierLength = (uint16_t)strlen(clientcredentialIOT_THING_NAME);

	/* Set MQTT keep-alive period. If the application does not send packets at an interval less than
	 * the keep-alive period, the MQTT library will send PINGREQ packets. */
	xConnectInfo.keepAliveSeconds = KEEP_ALIVE_TIMEOUT_SECONDS;

	bool xSessionPresent;
	/* Send MQTT CONNECT packet to broker. LWT is not used, so it
	 * is passed as NULL. */
	xResult = MQTT_Connect(&xMQTTContext,
						   &xConnectInfo,
						   NULL,
						   CONNACK_RECV_TIMEOUT_MS,
						   &xSessionPresent);

	if (xResult != MQTTSuccess)
	{
		return -2;
	}
	return 0;
}

uint32_t ulDfmAWSPortSend(const void* pvData, uint32_t ulSize, const char* szUniqueSessionID, uint32_t ulTraceCounter, const char* szType, uint32_t ulSlice, uint32_t ulSlices, uint32_t* pulBytesSent)
{
	if (prvCheckConnection())
	{
		/* Disconnect to make sure the session is closed */
		if (MQTT_Disconnect(&xMQTTContext) != MQTTSuccess)
		{
			return -1;
		}
		(void)memset((void*)&xMQTTContext, 0x00, sizeof(xMQTTContext));

		/* Disconnect the TLS session */
		if (SecureSocketsTransport_Disconnect(&xNetworkContext) != TRANSPORT_SOCKET_STATUS_SUCCESS)
		{
			return -3;
		}
		(void)memset((void*)&xNetworkContext, 0x00, sizeof(xNetworkContext));

		if (ulDfmAWSPortInit() != 0)
		{
			return -2;
		}
	}

	snprintf(cTopicBuffer, DFM_CFG_CLOUD_PORT_MAX_TOPIC_SIZE - 1, "DevAlert/%s/%lu/%lu-%lu_%s", szUniqueSessionID, ulTraceCounter, ulSlice, ulSlices, szType);
	cTopicBuffer[DFM_CFG_CLOUD_PORT_MAX_TOPIC_SIZE - 1] = '\0';

	MQTTStatus_t xResult;
	MQTTPublishInfo_t xMQTTPublishInfo;
	uint32_t xStatus = 0;

	/* Some fields are not used so start with everything at 0. */
	(void)memset((void*)&xMQTTPublishInfo, 0x00, sizeof(xMQTTPublishInfo));

	xMQTTPublishInfo.qos = MQTTQoS0;
	xMQTTPublishInfo.retain = false;
	xMQTTPublishInfo.pTopicName = cTopicBuffer;
	xMQTTPublishInfo.topicNameLength = (uint16_t)strlen(cTopicBuffer);
	xMQTTPublishInfo.pPayload = pvData;
	xMQTTPublishInfo.payloadLength = ulSize;

	/* Get a unique packet id. */
	uint16_t usPublishPacketIdentifier = MQTT_GetPacketId(&xMQTTContext);

	/* Send PUBLISH packet. Packet ID is not used for a QoS1 publish. */
	xResult = MQTT_Publish(&xMQTTContext, &xMQTTPublishInfo, usPublishPacketIdentifier);

	if (xResult != MQTTSuccess)
	{
		xStatus = -1;
	}
	else
	{
		*pulBytesSent = ulSize;
	}

	return xStatus;
}

/*******************************************************************************
 * prvCheckConnection
 *
 * Check if connection is closed or will soon be closed. Call this
 * function to determine if a new connection should be initiated.
 *
 * @return 0 if connected 1 if not connected.
 ******************************************************************************/
static uint8_t prvCheckConnection()
{
	if (xMQTTContext.connectStatus == MQTTNotConnected)
	{
		return 1;
	}
	if (KEEP_ALIVE_TIMEOUT_SECONDS > 0)
	{
		if (prvGetTimeMs() > KEEP_ALIVE_TIMEOUT_SECONDS * 1000)
		{
			return 1;
		}
	}
	return 0;
}

#endif /* (defined(DFM_CFG_ENABLED) && ((DFM_CFG_ENABLED) >= 1) && defined(DFM_TRACE_RECORDER_IS_ENABLED) && ((DFM_TRACE_RECORDER_IS_ENABLED) >= 1)) */
