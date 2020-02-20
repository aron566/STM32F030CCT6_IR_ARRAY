/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-10-29 13:16     
*******************************************************************************/

/* Function Description ********************************************************

    填充modbus报文

*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "debugPrintf.h"
#include "modbus_CRC.h"


/* Exported functions --------------------------------------------------------*/
/*******************************************************************************
 * 
 * Modbus填充读寄存器报文
 * 
*******************************************************************************/
uint8_t fill_read_msg( uint8_t msg_buffer[], uint8_t slave_id, uint16_t reg_start_addr, uint16_t reg_num )
{
    uint8_t length = 0;
    uint16_t crc = 0;
    
    msg_buffer[ length++ ] = slave_id;
    msg_buffer[ length++ ] = 0x03;
    msg_buffer[ length++ ] = reg_start_addr >> 8;
    msg_buffer[ length++ ] = reg_start_addr;
    msg_buffer[ length++ ] = reg_num >> 8;
    msg_buffer[ length++ ] = reg_num;
    crc = Modbus_CRC16( msg_buffer, length );
    msg_buffer[ length++ ] = crc;
    msg_buffer[ length++ ] = crc >> 8;
    return length;
}

/*******************************************************************************
 * 
 * Modbus填充写寄存器报文
 * 
*******************************************************************************/
uint8_t fill_write_msg( uint8_t msg_buffer[], uint8_t slave_id, uint16_t reg_start_addr, uint16_t reg_num, const uint16_t reg_value[] )
{
    uint8_t length = 0;
    uint16_t crc = 0;
    
    msg_buffer[ length++ ] = slave_id;
    msg_buffer[ length++ ] = 0x10;
    msg_buffer[ length++ ] = reg_start_addr >> 8;
    msg_buffer[ length++ ] = reg_start_addr;
    msg_buffer[ length++ ] = reg_num >> 8;
    msg_buffer[ length++ ] = reg_num;
    msg_buffer[ length++ ] = reg_num * 2;
    for ( uint16_t i = 0; i < reg_num; ++i )
    {
        msg_buffer[ length++ ] = reg_value[ i ] >> 8;
        msg_buffer[ length++ ] = reg_value[ i ];
    }
    crc = Modbus_CRC16( msg_buffer, length );
    msg_buffer[ length++ ] = crc;
    msg_buffer[ length++ ] = crc >> 8;
    return length;
}

