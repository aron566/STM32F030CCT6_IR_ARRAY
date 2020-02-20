/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-11-06 19:12     
*******************************************************************************/
#pragma once

/* Includes ------------------------------------------------------------------*/
#include "GN_uart.h"

/* Extern function prototypes ------------------------------------------------*/
void Modbus_slave_poll( UART_TTY_e tty );

int Modbus_slave_write_regVal( uint8_t *pRegBuffer_in, uint16_t reg_start, uint16_t reg_num );
int Modbus_slave_read_regVal( uint8_t *pRegBuffer_out, uint16_t reg_start, uint16_t reg_num );
