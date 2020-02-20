/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-11-07 16:34     
*******************************************************************************/

/* Function Description ********************************************************

    Modbus从机读寄存器报文处理

*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "debugPrintf.h"
#include "GN_uart.h"
#include "modbus_slave.h"
#include "modbus_slave_serial.h"
#include "modbus_CRC.h"


/* Private function prototypes -----------------------------------------------*/
static int check_read_reg_msg( msg_t *pMsg );
static int Modbus_slave_fill_read_msg( msg_t *pMsg );
int Modbus_slave_read_regVal( uint8_t *pRegBuffer_out, uint16_t reg_start, uint16_t reg_num );

/* Exported functions --------------------------------------------------------*/
//Modbus从机读寄存器报文处理
int Modbus_slave_read_reg_msg_handle( msg_t *pMsg )
{
    //校验报文
    if ( check_read_reg_msg( pMsg ) < 0 )
    {
        return -1;
    }
    //填充回复报文
    if ( Modbus_slave_fill_read_msg( pMsg ) < 0 )
    {
        return -1;
    }
    return 0;
}

//校验写寄存器报文
static int check_read_reg_msg( msg_t *pMsg )
{
    if ( pMsg->length < 8 ) //长度不够
    {
        return -1;
    }
    if ( ( pMsg->length != 8 )  //长度错误
        || ( 0 != Modbus_CRC16( pMsg->buffer, pMsg->length ) ) )   //CRC错误
    {
        pMsg->length = 0;   //清空当前报文
        return -1;
    }
    return 0;
}

//Modbus从机填充读寄存器回复报文
static int Modbus_slave_fill_read_msg( msg_t *pMsg )
{
    uint16_t reg_start = 0; //寄存器起始地址
    uint16_t reg_num = 0;   //寄存器数量
    uint8_t byte_num = 0;   //字节数量
    uint8_t *pRegBuffer = 0;//寄存器值缓冲区
    uint16_t crc = 0;       //CRC校验码
    
    reg_num = (uint16_t)pMsg->buffer[4] << 8 | pMsg->buffer[5];
    if ( reg_num * 2 + 10 > UART_BUFFER_SIZE ) //寄存器数量过多
    {
        pMsg->length = 0;
        return -1;
    }
    reg_start = (uint16_t)pMsg->buffer[2] << 8 | pMsg->buffer[3];
    pRegBuffer = &pMsg->buffer[ 3 ];
    Modbus_slave_read_regVal( pRegBuffer, reg_start, reg_num );
    byte_num = 2 * reg_num;
    pMsg->buffer[ 2 ] = byte_num;
    pMsg->length = 3 + byte_num;
    crc = Modbus_CRC16( pMsg->buffer, pMsg->length );
    pMsg->buffer[ pMsg->length++ ] = crc;
    pMsg->buffer[ pMsg->length++ ] = crc >> 8;
    return 0;
}

//Modbus读寄存器值
//int Modbus_slave_read_regVal( uint8_t *pRegBuffer_out, uint16_t reg_start, uint16_t reg_num )
//{
//    return 0;
//}
