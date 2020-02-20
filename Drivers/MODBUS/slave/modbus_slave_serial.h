/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-11-06 19:46     
*******************************************************************************/
#pragma once


/* Typedef -------------------------------------------------------------------*/
typedef struct
{
    uint8_t     buffer[ UART_BUFFER_SIZE ]; //报文缓冲区
    uint16_t    length;                     //报文长度
    uint16_t    continue_no_msg_cnt;        //连续未收到报文次数
}msg_t;

void slave_recv_msg_handle( UART_TTY_e tty, msg_t *pMsg );

