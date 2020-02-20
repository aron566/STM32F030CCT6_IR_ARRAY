/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-11-06 19:12     
*******************************************************************************/

/* Function Description ********************************************************

    Modbus从机写寄存器报文处理

*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "debugPrintf.h"
#include "GN_uart.h"
#include "peripheral.h"
#include "modbus_slave.h"
#include "modbus_slave_serial.h"
#include "modbus_CRC.h"


/* Private function prototypes -----------------------------------------------*/
static int check_write_reg_msg( msg_t *pMsg );
int Modbus_slave_write_regVal( uint8_t *pRegBuffer_in, uint16_t reg_start, uint16_t reg_num );
static void Modbus_slave_fill_write_msg( msg_t *pMsg );

/* Exported functions --------------------------------------------------------*/
//Modbus从机写寄存器报文处理
int Modbus_slave_write_reg_msg_handle( msg_t *pMsg )
{
    uint16_t reg_start = 0; //寄存器起始地址
    uint16_t reg_num = 0;   //寄存器数量
    uint8_t *pRegBuffer = 0;//寄存器值缓冲区
    
    //校验报文
    if ( check_write_reg_msg( pMsg ) < 0 )
    {
        return -1;
    }
    
    //解析报文
    reg_start = (uint16_t)pMsg->buffer[2] << 8 | pMsg->buffer[3];
    reg_num = (uint16_t)pMsg->buffer[4] << 8 | pMsg->buffer[5];
    pRegBuffer = &pMsg->buffer[ 7 ];
    Modbus_slave_write_regVal( pRegBuffer, reg_start, reg_num );
    
    //填充回复报文
    Modbus_slave_fill_write_msg( pMsg );
    return 0;
}

//校验写寄存器报文
static int check_write_reg_msg( msg_t *pMsg )
{
    uint16_t reg_num = 0;    //寄存器数量
    uint8_t byte_num = 0;   //字节数量
    
    //校验报文
    reg_num = (uint16_t)pMsg->buffer[4] << 8 | pMsg->buffer[5];
    byte_num = pMsg->buffer[ 6 ];
    if ( pMsg->length < 9 + 2 * reg_num ) //长度不够
    {
        return -1;
    }
    if ( (pMsg->length != 9 + byte_num )    //长度错误
        || ( byte_num != reg_num * 2 )      //字节数错误
            || ( 0 != Modbus_CRC16( pMsg->buffer, pMsg->length ) ) )   //CRC校验错误
    {
        pMsg->length = 0;
        return -1;
    }
    return 0;
}

//Modbus从机写寄存器值
//int Modbus_slave_write_regVal( uint8_t *pRegBuffer_in, uint16_t reg_start, uint16_t reg_num )
//{
//    return 0;
//}

//Modbus从机填充写寄存器回复报文
static void Modbus_slave_fill_write_msg( msg_t *pMsg )
{
    uint16_t crc = 0;
    
    crc = Modbus_CRC16( pMsg->buffer, 6 );
    pMsg->buffer[ 6 ] = crc;
    pMsg->buffer[ 7 ] = crc >> 8;
    pMsg->length = 8;
}
