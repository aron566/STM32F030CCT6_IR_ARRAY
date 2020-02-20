/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-11-06 19:39     
*******************************************************************************/

/* Function Description ********************************************************

    Modbus从机的串口接收

*******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "debugPrintf.h"
#include "GN_uart.h"
#include "peripheral.h"
#include "modbus_slave.h"
#include "modbus_slave_serial.h"

/* Exported functions --------------------------------------------------------*/
/*******************************************************************************
 * 
 * 从机接收报文处理
 * 
*******************************************************************************/
void slave_recv_msg_handle( UART_TTY_e tty, msg_t *pMsg )
{
    int length;
    
    length = uart_read( tty, &pMsg->buffer[pMsg->length], sizeof(pMsg->buffer) - pMsg->length, 2 );
    if ( length > 0 )
    {
        pMsg->length += length;
        pMsg->continue_no_msg_cnt = 0;
    }
    else
    {
        //丢弃超时报文
        if ( pMsg->length > 0 )
        {
            if ( pMsg->continue_no_msg_cnt > 10 )
            {
                pMsg->length = 0;
                pMsg->continue_no_msg_cnt = 0;
            }
            else
            {
                pMsg->continue_no_msg_cnt++;
            }
        }
    }
}