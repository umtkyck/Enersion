/**
 ******************************************************************************
 * @file           : rs485_protocol.c
 * @brief          : RS485 Communication Protocol Implementation
 ******************************************************************************
 */

#include "rs485_protocol.h"
#include "debug_uart.h"
#include <string.h>

/* External UART Handle */
extern UART_HandleTypeDef huart2;

/* Protocol Constants */
#define RS485_START_BYTE    0xAA
#define RS485_END_BYTE      0x55

/* Private Variables */
static uint8_t myAddress = RS485_ADDR_CONTROLLER_420;
static uint8_t rxBuffer[RS485_RX_BUFFER_SIZE];
static uint16_t rxIndex = 0;
static RS485_Status_t status = {0};

/* Command Handler Array */
typedef void (*CommandHandler_t)(const RS485_Packet_t*);
static CommandHandler_t commandHandlers[256] = {0};

/* Private Function Prototypes */
static void RS485_ProcessReceivedByte(uint8_t byte);
static void RS485_ProcessPacket(const RS485_Packet_t* packet);
static void RS485_HandlePing(const RS485_Packet_t* packet);
static void RS485_HandleGetVersion(const RS485_Packet_t* packet);
static void RS485_HandleHeartbeat(const RS485_Packet_t* packet);
static void RS485_HandleGetStatus(const RS485_Packet_t* packet);

/**
 * @brief  Initialize RS485 protocol
 * @param  myAddr: This MCU's address
 * @retval None
 */
void RS485_Init(uint8_t myAddr)
{
    myAddress = myAddr;
    rxIndex = 0;
    memset(&status, 0, sizeof(status));
    
    status.mcuId = myAddress;
    status.health = 100;
    
    /* Register default command handlers */
    RS485_RegisterCommandHandler(CMD_PING, RS485_HandlePing);
    RS485_RegisterCommandHandler(CMD_GET_VERSION, RS485_HandleGetVersion);
    RS485_RegisterCommandHandler(CMD_HEARTBEAT, RS485_HandleHeartbeat);
    RS485_RegisterCommandHandler(CMD_GET_STATUS, RS485_HandleGetStatus);
    
    /* Start receiving in interrupt mode */
    HAL_UART_Receive_IT(&huart2, rxBuffer, 1);
    
    DEBUG_INFO("RS485 Protocol initialized, Address: 0x%02X", myAddress);
}

/**
 * @brief  Process RS485 communication (call in main loop)
 * @retval None
 */
void RS485_Process(void)
{
    /* Update uptime */
    status.uptime = HAL_GetTick() / 1000;
}

/**
 * @brief  Send RS485 packet
 * @param  destAddr: Destination address
 * @param  cmd: Command code
 * @param  data: Data payload
 * @param  length: Data length
 * @retval HAL status
 */
HAL_StatusTypeDef RS485_SendPacket(uint8_t destAddr, RS485_Command_t cmd, 
                                   const uint8_t* data, uint8_t length)
{
    if (length > 250) {
        return HAL_ERROR;
    }
    
    RS485_Packet_t packet;
    packet.startByte = RS485_START_BYTE;
    packet.destAddr = destAddr;
    packet.srcAddr = myAddress;
    packet.command = cmd;
    packet.length = length;
    
    if (length > 0 && data != NULL) {
        memcpy(packet.data, data, length);
    }
    
    /* Calculate CRC over header and data */
    uint8_t crcBuffer[5 + length];
    crcBuffer[0] = packet.destAddr;
    crcBuffer[1] = packet.srcAddr;
    crcBuffer[2] = packet.command;
    crcBuffer[3] = packet.length;
    memcpy(&crcBuffer[4], packet.data, length);
    
    packet.checksum = RS485_CalculateCRC(crcBuffer, 4 + length);
    packet.endByte = RS485_END_BYTE;
    
    /* Calculate total packet size */
    uint16_t packetSize = 5 + length + 2 + 1; // header + data + crc + end
    
    /* Transmit packet */
    HAL_StatusTypeDef result = HAL_UART_Transmit(&huart2, (uint8_t*)&packet, 
                                                  packetSize, RS485_TIMEOUT_MS);
    
    if (result == HAL_OK) {
        status.txPacketCount++;
        DEBUG_DEBUG("TX: Addr=0x%02X Cmd=0x%02X Len=%d", destAddr, cmd, length);
    } else {
        status.errorCount++;
        DEBUG_ERROR("TX Failed: Addr=0x%02X Cmd=0x%02X", destAddr, cmd);
    }
    
    return result;
}

/**
 * @brief  Send response packet
 * @param  destAddr: Destination address
 * @param  cmd: Response command code
 * @param  data: Data payload
 * @param  length: Data length
 * @retval HAL status
 */
HAL_StatusTypeDef RS485_SendResponse(uint8_t destAddr, RS485_Command_t cmd, 
                                     const uint8_t* data, uint8_t length)
{
    return RS485_SendPacket(destAddr, cmd, data, length);
}

/**
 * @brief  Send error response
 * @param  destAddr: Destination address
 * @param  error: Error code
 * @retval HAL status
 */
HAL_StatusTypeDef RS485_SendError(uint8_t destAddr, RS485_Error_t error)
{
    uint8_t errorData[2];
    errorData[0] = error;
    errorData[1] = myAddress;
    
    return RS485_SendPacket(destAddr, CMD_ERROR_RESPONSE, errorData, 2);
}

/**
 * @brief  Register command handler
 * @param  cmd: Command code
 * @param  handler: Handler function
 * @retval None
 */
void RS485_RegisterCommandHandler(RS485_Command_t cmd, 
                                  void (*handler)(const RS485_Packet_t* packet))
{
    commandHandlers[cmd] = handler;
}

/**
 * @brief  Get protocol status
 * @retval Pointer to status structure
 */
RS485_Status_t* RS485_GetStatus(void)
{
    return &status;
}

/**
 * @brief  Calculate CRC16 checksum
 * @param  data: Data buffer
 * @param  length: Data length
 * @retval CRC16 value
 */
uint16_t RS485_CalculateCRC(const uint8_t* data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief  Process received packet
 * @param  packet: Received packet
 * @retval None
 */
static void RS485_ProcessPacket(const RS485_Packet_t* packet)
{
    /* Verify checksum */
    uint8_t crcBuffer[4 + packet->length];
    crcBuffer[0] = packet->destAddr;
    crcBuffer[1] = packet->srcAddr;
    crcBuffer[2] = packet->command;
    crcBuffer[3] = packet->length;
    memcpy(&crcBuffer[4], packet->data, packet->length);
    
    uint16_t calculatedCRC = RS485_CalculateCRC(crcBuffer, 4 + packet->length);
    
    if (calculatedCRC != packet->checksum) {
        DEBUG_ERROR("CRC Error: Expected 0x%04X, Got 0x%04X", 
                   calculatedCRC, packet->checksum);
        status.errorCount++;
        RS485_SendError(packet->srcAddr, RS485_ERR_INVALID_CHECKSUM);
        return;
    }
    
    /* Check if packet is for us */
    if (packet->destAddr != myAddress && packet->destAddr != RS485_ADDR_BROADCAST) {
        return; // Not for us
    }
    
    status.rxPacketCount++;
    DEBUG_DEBUG("RX: From=0x%02X Cmd=0x%02X Len=%d", 
               packet->srcAddr, packet->command, packet->length);
    
    /* Call command handler if registered */
    if (commandHandlers[packet->command] != NULL) {
        commandHandlers[packet->command](packet);
    } else {
        DEBUG_WARNING("Unhandled command: 0x%02X", packet->command);
        RS485_SendError(packet->srcAddr, RS485_ERR_INVALID_COMMAND);
    }
}

/**
 * @brief  Handle PING command
 * @param  packet: Received packet
 * @retval None
 */
static void RS485_HandlePing(const RS485_Packet_t* packet)
{
    DEBUG_INFO("PING received from 0x%02X", packet->srcAddr);
    RS485_SendResponse(packet->srcAddr, CMD_PING_RESPONSE, NULL, 0);
}

/**
 * @brief  Handle GET_VERSION command
 * @param  packet: Received packet
 * @retval None
 */
static void RS485_HandleGetVersion(const RS485_Packet_t* packet)
{
    uint8_t versionData[8];
    versionData[0] = FW_VERSION_MAJOR;
    versionData[1] = FW_VERSION_MINOR;
    versionData[2] = FW_VERSION_PATCH;
    versionData[3] = FW_BUILD_NUMBER;
    versionData[4] = myAddress;
    versionData[5] = 0; // Reserved
    versionData[6] = 0; // Reserved
    versionData[7] = 0; // Reserved
    
    RS485_SendResponse(packet->srcAddr, CMD_VERSION_RESPONSE, versionData, 8);
}

/**
 * @brief  Handle HEARTBEAT command
 * @param  packet: Received packet
 * @retval None
 */
static void RS485_HandleHeartbeat(const RS485_Packet_t* packet)
{
    uint8_t heartbeatData[2];
    heartbeatData[0] = myAddress;
    heartbeatData[1] = status.health;
    
    RS485_SendResponse(packet->srcAddr, CMD_HEARTBEAT_RESPONSE, heartbeatData, 2);
}

/**
 * @brief  Handle GET_STATUS command
 * @param  packet: Received packet
 * @retval None
 */
static void RS485_HandleGetStatus(const RS485_Packet_t* packet)
{
    uint8_t statusData[16];
    statusData[0] = status.mcuId;
    statusData[1] = status.health;
    memcpy(&statusData[2], &status.uptime, 4);
    memcpy(&statusData[6], &status.errorCount, 4);
    memcpy(&statusData[10], &status.rxPacketCount, 4);
    memcpy(&statusData[14], &status.txPacketCount, 2);
    
    RS485_SendResponse(packet->srcAddr, CMD_STATUS_RESPONSE, statusData, 16);
}

/**
 * @brief  UART Receive Complete Callback
 * @param  huart: UART handle
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        RS485_ProcessReceivedByte(rxBuffer[0]);
        HAL_UART_Receive_IT(&huart2, rxBuffer, 1);
    }
}

/**
 * @brief  Process received byte
 * @param  byte: Received byte
 * @retval None
 */
static void RS485_ProcessReceivedByte(uint8_t byte)
{
    static uint8_t packetBuffer[RS485_MAX_PACKET_SIZE];
    static uint16_t packetIndex = 0;
    static uint8_t expectedLength = 0;
    
    if (packetIndex == 0 && byte != RS485_START_BYTE) {
        return; // Wait for start byte
    }
    
    packetBuffer[packetIndex++] = byte;
    
    /* Get expected length from packet header */
    if (packetIndex == 5) {
        expectedLength = packetBuffer[4]; // Length field
    }
    
    /* Check if we have complete packet */
    if (packetIndex >= 8 && packetIndex >= (5 + expectedLength + 3)) {
        /* Verify end byte */
        if (packetBuffer[packetIndex - 1] == RS485_END_BYTE) {
            RS485_ProcessPacket((const RS485_Packet_t*)packetBuffer);
        } else {
            DEBUG_ERROR("Invalid end byte");
            status.errorCount++;
        }
        packetIndex = 0;
        expectedLength = 0;
    }
    
    /* Prevent buffer overflow */
    if (packetIndex >= RS485_MAX_PACKET_SIZE) {
        packetIndex = 0;
        expectedLength = 0;
        DEBUG_ERROR("RX buffer overflow");
        status.errorCount++;
    }
}


