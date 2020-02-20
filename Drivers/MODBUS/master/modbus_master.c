/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-10-29 10:54     
*******************************************************************************/

/* Function Description ********************************************************

    Modbus主机代码
    实现Modbus（RTU）的读、写寄存器功能

*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "debugPrintf.h"
#include "GN_uart.h"
#include "peripheral.h"
#include "modbus_master.h"

/* Extern function prototypes ------------------------------------------------*/
uint8_t fill_read_msg( uint8_t msg_buffer[], uint8_t slave_id, uint16_t reg_start_addr, uint16_t reg_num ); 
uint8_t fill_write_msg( uint8_t msg_buffer[], uint8_t slave_id, uint16_t reg_start_addr, uint16_t reg_num, const uint16_t reg_value[] );
int modbus_read_reg_parse( uint8_t msg[], uint16_t length, uint16_t *pReg_buffer_out, uint16_t reg_num );
int check_write_reg_msg( uint8_t msg[], uint16_t length, uint16_t reg_start_addr, uint16_t reg_num );

/* Private function prototypes -----------------------------------------------*/
static int receive_msg( UART_TTY_e tty, uint8_t buffer[], uint16_t buffer_size, uint16_t response_timeout, uint16_t frame_interval );

/* Exported functions --------------------------------------------------------*/
/*******************************************************************************
 * 
 * Modbus主机读寄存器（0x03）
 * 函数会阻塞到读寄存器结束
 * parameter:   tty             串口号
 *              slave_id        从机设备id
 *              reg_start_addr  寄存器起始地址
 *              reg_num         寄存器数量
 *              pReg_buffer_out 读到的寄存器值
 *              timeout_ms      无应答超时时间（毫秒）
 * return:  0   成功; 
 *         <0   失败;
 * 
*******************************************************************************/
int Modbus_master_read_regs( UART_TTY_e tty, uint8_t slave_id, uint16_t reg_start_addr, uint16_t reg_num, uint16_t *pReg_buffer_out, uint16_t timeout_ms )
{
    uint8_t msg[ UART_BUFFER_SIZE ] = {0};
    int length = 0;
    uint16_t wait_time_cnt = 0;
    
    //填充Modbus报文，包括CRC16
    length = fill_read_msg( msg, slave_id, reg_start_addr, reg_num );
    if ( length <= 0 )
    {
        LOG( DEBUG_LEVEL_ERROR, "fill_read_msg error" );
        return -1;
    }
    
    //发送报文
    if ( UART_write( tty, msg, length ) < 0 )
    {
        LOG( DEBUG_LEVEL_ERROR, "send read msg error" );
        return -1;
    }
    while ( !tty_is_ready( tty ) )   //等待发送完成
    {
        osDelay(2);
        if ( ++wait_time_cnt > 1000 )
        {
            break;
        }
    }
    
    //等待回复报文
    length = receive_msg( tty, msg, sizeof(msg), timeout_ms, 20 );
    if ( length <= 0 )
    {
        LOG( DEBUG_LEVEL_NORMAL, "wait read reg msg err(%d)", length );
        return -1;
    }
    
    //解析出寄存器值
    if ( modbus_read_reg_parse( msg, length, pReg_buffer_out, reg_num ) < 0 )
    {
        return -1;
    }
    return 0;
}

/*******************************************************************************
 * 
 * Modbus主机写寄存器（0x10）
 * 函数会阻塞到写寄存器结束
 * parameter:   tty             串口号
 *              slave_id        从机设备id
 *              reg_start_addr  寄存器起始地址
 *              reg_num         寄存器数量
 *              pReg_buffer_in  要写的寄存器值
 *              timeout_ms      无应答超时时间（毫秒）
 * return:  0   成功; 
 *         <0   失败;
 * 
*******************************************************************************/
int Modbus_master_write_regs( UART_TTY_e tty, uint8_t slave_id, uint16_t reg_start_addr, uint16_t reg_num, const uint16_t *pReg_buffer_in, uint16_t timeout_ms )
{
    uint8_t msg[ UART_BUFFER_SIZE ] = {0};
    int length = 0;
    uint16_t wait_time_cnt = 0;
    
    //填充Modbus报文，包括CRC16
    length = fill_write_msg( msg, slave_id, reg_start_addr, reg_num, pReg_buffer_in );
    if ( length <= 0 )
    {
        LOG( DEBUG_LEVEL_ERROR, "fill_write_msg error" );
        return -1;
    }
    
    //发送报文
    if ( UART_write( tty, msg, length ) < 0 )
    {
        LOG( DEBUG_LEVEL_ERROR, "send write msg error" );
        return -1;
    }
    while ( !tty_is_ready( tty ) )   //等待发送完成
    {
        osDelay(2);
        if ( ++wait_time_cnt > 1000 )
        {
            break;
        }
    }
    
    //等待回复报文
    length = receive_msg( tty, msg, sizeof(msg), timeout_ms, 20 );
    if ( length <= 0 )
    {
        LOG( DEBUG_LEVEL_NORMAL, "wait read reg msg err(%d)", length );
        return -1;
    }
    
    //解析出寄存器值
    if ( check_write_reg_msg( msg, length, reg_start_addr, reg_num ) < 0 )
    {
        return -1;
    }
    return 0;
}

/*******************************************************************************
 * 
 * 接收报文
 * parameter:   tty                 串口号
 *              msg_out             报文缓冲区
 *              size                缓冲区大小
 *              response_timeout    应答超时（毫秒）
 *              frame_interval      帧间隔（毫秒）
 * return:  收到的报文长度;
 * 
*******************************************************************************/
static int receive_msg( UART_TTY_e tty, uint8_t msg_out[], uint16_t size, uint16_t response_timeout, uint16_t frame_interval )
{
    int tmp_len;
    int length = 0; //报文长度
    uint16_t timeout_ms = 0;
    
    while (1)
    {
        tmp_len = uart_read( tty, &msg_out[ length ], size - length, 5 );
        if ( tmp_len > 0 )
        {
            length += tmp_len;
            if ( length >= size )   //报文长度超出缓冲区大小
            {
                break;
            }
            timeout_ms = 0;
            continue;
        }
        timeout_ms += 5;
        if ( ( 0 == length )
            && ( timeout_ms > response_timeout ) )  //应答超时
        {
            break;
        }
        if ( ( 0 != length )
            && ( timeout_ms > frame_interval ) )    //帧间隔超时
        {
            break;
        }
    }
    return length;
}
