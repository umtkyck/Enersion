/**
 ******************************************************************************
 * @file           : rs485_protocol.h
 * @brief          : RS485 Communication Protocol
 ******************************************************************************
 * @attention
 *
 * Protocol Layer for RS485 communication
 * USART2 configured as RS485 @ 115200 baud
 *
 ******************************************************************************
 */

#ifndef RS485_PROTOCOL_H
#define RS485_PROTOCOL_H

#include "main.h"
#include "version.h"

/* Protocol Configuration */
#define RS485_BAUD_RATE         115200
#define RS485_TIMEOUT_MS        100
#define RS485_MAX_PACKET_SIZE   256
#define RS485_RX_BUFFER_SIZE    512
#define RS485_TX_BUFFER_SIZE    512

/* MCU Address Definitions */
#define RS485_ADDR_BROADCAST    0x00
#define RS485_ADDR_CONTROLLER_420   0x01
#define RS485_ADDR_CONTROLLER_DIO   0x02
#define RS485_ADDR_CONTROLLER_OUT   0x03
#define RS485_ADDR_GUI          0x10

/* Command Codes */
typedef enum {
    CMD_PING                = 0x01,
    CMD_PING_RESPONSE       = 0x02,
    CMD_GET_VERSION         = 0x03,
    CMD_VERSION_RESPONSE    = 0x04,
    CMD_HEARTBEAT           = 0x05,
    CMD_HEARTBEAT_RESPONSE  = 0x06,
    CMD_GET_STATUS          = 0x10,
    CMD_STATUS_RESPONSE     = 0x11,
    CMD_READ_DI             = 0x20,
    CMD_DI_RESPONSE         = 0x21,
    CMD_WRITE_DO            = 0x30,
    CMD_DO_RESPONSE         = 0x31,
    CMD_READ_DO             = 0x32,
    CMD_READ_ANALOG         = 0x40,
    CMD_ANALOG_RESPONSE     = 0x41,
    CMD_ERROR_RESPONSE      = 0xFF
} RS485_Command_t;

/* Error Codes */
typedef enum {
    RS485_ERR_NONE              = 0x00,
    RS485_ERR_INVALID_CHECKSUM  = 0x01,
    RS485_ERR_INVALID_ADDRESS   = 0x02,
    RS485_ERR_INVALID_COMMAND   = 0x03,
    RS485_ERR_INVALID_LENGTH    = 0x04,
    RS485_ERR_TIMEOUT           = 0x05,
    RS485_ERR_BUSY              = 0x06
} RS485_Error_t;

/* Packet Structure */
typedef struct {
    uint8_t startByte;      // 0xAA
    uint8_t destAddr;       // Destination address
    uint8_t srcAddr;        // Source address
    uint8_t command;        // Command code
    uint8_t length;         // Data length (0-250)
    uint8_t data[250];      // Data payload
    uint16_t checksum;      // CRC16
    uint8_t endByte;        // 0x55
} __attribute__((packed)) RS485_Packet_t;

/* Status Structure */
typedef struct {
    uint8_t mcuId;
    uint8_t health;         // 0-100%
    uint32_t uptime;        // Seconds
    uint32_t errorCount;
    uint32_t rxPacketCount;
    uint32_t txPacketCount;
} RS485_Status_t;

/* Function Prototypes */
void RS485_Init(uint8_t myAddress);
void RS485_Process(void);
HAL_StatusTypeDef RS485_SendPacket(uint8_t destAddr, RS485_Command_t cmd, 
                                   const uint8_t* data, uint8_t length);
HAL_StatusTypeDef RS485_SendResponse(uint8_t destAddr, RS485_Command_t cmd, 
                                     const uint8_t* data, uint8_t length);
HAL_StatusTypeDef RS485_SendError(uint8_t destAddr, RS485_Error_t error);
void RS485_RegisterCommandHandler(RS485_Command_t cmd, 
                                  void (*handler)(const RS485_Packet_t* packet));
RS485_Status_t* RS485_GetStatus(void);
uint16_t RS485_CalculateCRC(const uint8_t* data, uint16_t length);

#endif /* RS485_PROTOCOL_H */


