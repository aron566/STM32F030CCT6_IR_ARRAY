#pragma once
#include "stm32f0xx_hal.h"
#include "cmsis_os.h"
#define UART_BUFFER_SIZE 256
#define UART_TXING_BUFFER_SIZE 128

typedef struct
{
    UART_HandleTypeDef *huart;
    int is_half_duplex;
    
    osSemaphoreId *pRX_Sem;
    
        /* 以下为接收部分 */
    uint8_t RX_temp;
    uint8_t RX_buffer[ UART_BUFFER_SIZE ];
    volatile int RX_head;   //首个有效字节所有位置
    volatile int RX_tail;   //首个无效字节所有位置，当head与tail相等时，表示缓冲区为空
    volatile int RX_notice_len;  //当接收区字节达到这么多时，发出信号量通知上层(为0时，表示无论多少都不发送)
    volatile uint32_t RX_systick_of_latest_byte; //收到最新字节的邮戳
    
    /* 以下为发送部分 */
    uint8_t TX_buffer[ UART_BUFFER_SIZE ];
    volatile int TX_head;   //首个有效字节所有位置,也作为发送的游标
    volatile int TX_tail;   //首个无效字节所有位置，当head与tail相等时，表示缓冲区为空
        
    uint8_t TXING_buffer[ UART_TXING_BUFFER_SIZE ];
    uint8_t TXING_buffer_Len;
    
}UART_descriptor_S;

typedef enum
{
    UART_TTY0 = 0,
    UART_TTY1,
    UART_TTY2,
    UART_TTY3,
    UART_TTY4,
    UART_TTY5,
    UART_TTY_COUNT,
    UART_TTY_ERROR = 0xFFFF,
}UART_TTY_e;

#define UART_J0 UART_TTY1
#define UART_J1 UART_TTY5
#define UART_J99 UART_TTY0

int uart_read( UART_TTY_e tty, uint8_t *pbuffer, int read_len, uint32_t milliseconds  );
int UART_write( UART_TTY_e tty, const uint8_t *pbuffer, int write_len );
void UART_set_single_wire_half_duplex_enable( UART_TTY_e tty );