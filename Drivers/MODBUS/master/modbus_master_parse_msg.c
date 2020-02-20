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

/* Private function prototypes -----------------------------------------------*/
static int check_read_reg_msg( uint8_t msg[], uint16_t length, uint16_t reg_num );

/* Exported functions --------------------------------------------------------*/
//解析读寄存器的回复报文
int modbus_read_reg_parse( uint8_t msg[], uint16_t length, uint16_t *pReg_buffer_out, uint16_t reg_num )
{
    if ( check_read_reg_msg( msg, length, reg_num ) < 0 )   //检查报文是否错误
    {
        return -1;
    }
    for ( uint16_t i = 0; i < reg_num; ++i )
    {
        pReg_buffer_out[ i ] = (uint16_t)msg[ 3 + 2*i ] << 8 ;
        pReg_buffer_out[ i ] |= msg[ 4 + 2*i ] & 0xFF;
    }
    return 0;
}

//检查写寄存器的回复报文
int check_write_reg_msg( uint8_t msg[], uint16_t length, uint16_t reg_start_addr, uint16_t reg_num )
{
    uint16_t reg_addr_in_msg = 0;
    uint16_t reg_num_in_msg = 0;
    
    if ( 0x10 != msg[ 1 ] ) //功能码错误
    {
        LOG( DEBUG_LEVEL_WARNING, "write_msg function code error" );
        return -1;
    }
    reg_addr_in_msg = (uint16_t)msg[ 2 ] << 8;
    reg_addr_in_msg |= msg[ 3 ];
    if ( reg_addr_in_msg != reg_start_addr )
    {
        LOG( DEBUG_LEVEL_WARNING, "write_msg reg_start_addr error" );
        return -1;
    }
    reg_num_in_msg = (uint16_t)msg[ 4 ] << 8;
    reg_num_in_msg |= msg[ 5 ];
    if ( reg_num_in_msg != reg_num )
    {
        LOG( DEBUG_LEVEL_WARNING, "write_msg reg_num error" );
        return -1;
    }
    return 0;
}

/* Private functions ---------------------------------------------------------*/
//检查读寄存器的回复报文
static int check_read_reg_msg( uint8_t msg[], uint16_t length, uint16_t reg_num )
{
    uint8_t bytes = 0;  //字节数
    
    if ( 0x03 != msg[ 1 ] ) //功能码错误
    {
        LOG( DEBUG_LEVEL_WARNING, "read_msg function code error" );
        return -1;
    }
    bytes = msg[ 2 ];
    if ( 2 * reg_num != bytes )   //字节数错误
    {
        LOG( DEBUG_LEVEL_WARNING, "read_msg bytes error" );
        return -1;
    }
    if ( bytes + 5 != length )  //报文长度错误
    {
        LOG( DEBUG_LEVEL_WARNING, "read_msg l error" );
        return -1;
    }
    if ( 0 != Modbus_CRC16( msg, length ) ) //CRC校验错误
    {
        LOG( DEBUG_LEVEL_WARNING, "read_msg crc error" );
        return -1;
    }
    return 0;
}