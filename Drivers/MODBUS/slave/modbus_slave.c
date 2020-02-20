/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-11-06 19:12     
*******************************************************************************/

/* Function Description ********************************************************

    Modbus从机代码
    实现Modbus（RTU）的读、写寄存器功能

*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "debugPrintf.h"
#include "GN_uart.h"
#include "peripheral.h"
#include "modbus_slave.h"
#include "modbus_slave_serial.h"
#include "modbus_CRC.h"


/* Private variables ---------------------------------------------------------*/
static msg_t    modbus_msg;

/* Extern function prototypes ------------------------------------------------*/
int Modbus_slave_read_reg_msg_handle( msg_t *pMsg );
int Modbus_slave_write_reg_msg_handle( msg_t *pMsg );

/* Exported functions --------------------------------------------------------*/
void Modbus_slave_poll( UART_TTY_e tty )
{
    msg_t *pMsg = &modbus_msg;
//    uint8_t slave_addr = 0;     //从机地址
    uint8_t function_code = 0;  //Modbus功能码
    
    //接收报文
    slave_recv_msg_handle( tty, pMsg );
    if ( pMsg->length < 8 ) //长度不够
    {
        return;
    }
    
    //报文解析，并填充回复报文
//    slave_addr = pMsg->buffer[ 0 ];
    function_code = pMsg->buffer[ 1 ];
    switch ( function_code )
    {
    case 0x03:      //读寄存器
        if ( Modbus_slave_read_reg_msg_handle( pMsg ) < 0 )
        {
            return;
        }
        break;
    case 0x10:      //写寄存器
        if ( Modbus_slave_write_reg_msg_handle( pMsg ) < 0 )
        {
            return;
        }
        break;
    default:        //未知功能码
        pMsg->length = 0;
        return;
    }
    
    //发送回复报文
    if ( pMsg->length > 0 )
    {
        UART_write( tty, pMsg->buffer, pMsg->length );
        pMsg->length = 0;
    }
}
