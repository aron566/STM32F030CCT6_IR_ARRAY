/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-10-29 11:07     
*******************************************************************************/

#pragma once


/* Extern function prototypes ------------------------------------------------*/
int Modbus_master_read_regs( UART_TTY_e tty, uint8_t slave_id, uint16_t reg_start_addr, uint16_t reg_num, uint16_t *pReg_buffer_out, uint16_t time_out_ms );
int Modbus_master_write_regs( UART_TTY_e tty, uint8_t slave_id, uint16_t reg_start_addr, uint16_t reg_num, const uint16_t *pReg_buffer_in, uint16_t time_out_ms );

