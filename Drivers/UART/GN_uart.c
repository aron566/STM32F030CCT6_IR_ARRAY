#include "GN_uart.h"

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;

extern osSemaphoreId UARTBinarySem01Handle;
extern osSemaphoreId UARTBinarySem02Handle;
extern osSemaphoreId UARTBinarySem03Handle;
extern osSemaphoreId UARTBinarySem04Handle;
extern osSemaphoreId UARTBinarySem05Handle;
extern osSemaphoreId UARTBinarySem06Handle;

static int UART_driver_get_RX_buffer_len( UART_descriptor_S *pDes );
static int UART_read_copy( UART_TTY_e tty, uint8_t *pbuffer, int read_len );


UART_descriptor_S UART_des[  ] =
{
    //USART1
    {
        .huart = &huart1,
        .is_half_duplex = 0,
        .pRX_Sem = &UARTBinarySem01Handle,
    },
    
    //USART2
    {
        .huart = &huart2,
        .is_half_duplex = 0,
        .pRX_Sem = &UARTBinarySem02Handle,
    },
    
    //USART3
    {
        .huart = &huart3,
        .is_half_duplex = 0,
        .pRX_Sem = &UARTBinarySem03Handle,
    },
    
    //USART4
    {
        .huart = &huart4,
        .is_half_duplex = 0,
        .pRX_Sem = &UARTBinarySem04Handle,
    },
    
    //USART5
    {
        .huart = &huart5,
        .is_half_duplex = 0,
        .pRX_Sem = &UARTBinarySem05Handle,
    },
    
    //USART6
    {
        .huart = &huart6,
        .is_half_duplex = 0,
        .pRX_Sem = &UARTBinarySem06Handle,
    },
};


int uart_read( UART_TTY_e tty, uint8_t *pbuffer, int read_len, uint32_t milliseconds  )
{
    UART_descriptor_S *pDes = &( UART_des[ tty ] );
//    uint32_t systick1 = 0, systick2 = 0;

    if( tty >= UART_TTY_COUNT )
    {
        return -1;
    }
    
    //先检测半双工状态下是否发送完毕
    if( ( 0 != pDes->is_half_duplex ) && ( HAL_UART_STATE_BUSY_TX == pDes->huart->gState ) )
    {
        if( osOK != osSemaphoreWait( *(pDes->pRX_Sem ), milliseconds ) )
        {
            return -1;
        }
        else if( HAL_UART_STATE_BUSY_TX == pDes->huart->gState )
        {
            return -1;
        }
    }
    
    //使能接收1个字节
    if( pDes->huart->RxState == HAL_UART_STATE_READY )
    {
        HAL_UART_Receive_IT( pDes->huart, &( pDes->RX_temp ), 1 );
    }

    if( 0 == UART_driver_get_RX_buffer_len( pDes ) && 0 == milliseconds )
    {
        return 0;
    }

    //如果数量足够，就直接复制
    if( UART_driver_get_RX_buffer_len( pDes ) > read_len )
    {
        return UART_read_copy( tty, pbuffer, read_len );
    }
    else if( UART_driver_get_RX_buffer_len( pDes ) > 0 && milliseconds <= 0 )
    {
        return UART_read_copy( tty, pbuffer, read_len );
    }
    else
    {
        //先置为0,禁止发出信号量,并清除历史信号量
        pDes->RX_notice_len = 0;
        while( osOK == osSemaphoreWait( *(pDes->pRX_Sem ), 0) );

        //设置通知长度，等待信号量
        pDes->RX_notice_len = read_len;
//        systick1 = xTaskGetTickCount( );
        osSemaphoreWait( *(pDes->pRX_Sem ), milliseconds );
//        systick2 = xTaskGetTickCount( );

        //复制数据并返回
        if( 0 == UART_driver_get_RX_buffer_len( pDes ) )
        {
            return 0;
        }
        else
        {
            read_len = UART_read_copy( tty, pbuffer, read_len );
            return read_len;
        }
    }    
}

/************************************************************************************
* ProcName    :
* Description : 
* Arguments   : 
* Returns     :             
************************************************************************************/
int UART_write( UART_TTY_e tty, const uint8_t *pbuffer, int write_len )
{
    int tail_temp = 0;
    int head_temp = 0;
    int buffer_len = 0;
    int blank_size = 0;
    int send_len = 0;
    int i;
    UART_descriptor_S *pDes = &( UART_des[ tty ] );

    if( tty >= UART_TTY_COUNT )
    {
        return -1;
    }

    if( ( ( pDes->TX_tail + 1 )%UART_BUFFER_SIZE ) == pDes->TX_head )
    {
        return -1;
    }

    if( pDes->is_half_duplex != 0 )
    {
        //UART_channel_set_dir_TX( tty );
    }

    //填充缓冲区
    tail_temp = pDes->TX_tail;
    buffer_len = ( tail_temp + UART_BUFFER_SIZE - pDes->TX_head ) % UART_BUFFER_SIZE;
    blank_size = UART_BUFFER_SIZE - buffer_len - 1;
    if( blank_size < write_len ) return -1;
    
    for( i = 0; i < write_len; i++ )
    {
        pDes->TX_buffer[ tail_temp ] = pbuffer[ i ];
        tail_temp++;
        tail_temp = tail_temp % UART_BUFFER_SIZE;
    }
    
    pDes->TX_tail = tail_temp;
    
    //开始发送
    if( HAL_UART_STATE_READY == pDes->huart->gState )
    {
        //从大缓冲区搬到小缓冲区
        tail_temp = pDes->TX_tail;
        buffer_len = ( tail_temp + UART_BUFFER_SIZE - pDes->TX_head ) % UART_BUFFER_SIZE;
        send_len = buffer_len;
        if( send_len > UART_TXING_BUFFER_SIZE ) send_len = UART_TXING_BUFFER_SIZE;
        
        head_temp = pDes->TX_head;
        for( i = 0; i < send_len; i++ )
        {
            pDes->TXING_buffer[ i ] = pDes->TX_buffer[ head_temp ];
            head_temp = ( head_temp + 1) % UART_BUFFER_SIZE;
        }
        
        pDes->TX_head = head_temp;
        pDes->TXING_buffer_Len = send_len;
        
        //如果是半双工，即先清空信号量，并配置成发送状态
        //改成先发送信号量通知读结束操作 2018-12-12
        if( 0 != pDes->is_half_duplex )
        {
            osSemaphoreRelease( *( pDes->pRX_Sem ) );
            HAL_HalfDuplex_EnableTransmitter( pDes->huart );
        }
        
        //开始发送
        HAL_UART_Transmit_IT( pDes->huart, pDes->TXING_buffer, pDes->TXING_buffer_Len );
    }
    
    return write_len;
}

/************************************************************************************
* ProcName    : UART_read_copy
* Description : 仅供应用进程使用
* Arguments   : 
* Returns     :             
************************************************************************************/
static int UART_read_copy( UART_TTY_e tty, uint8_t *pbuffer, int read_len )
{
    UART_descriptor_S *pDes = &( UART_des[ tty ] );
    int i = pDes->RX_head;
    int copyed_len = 0;
    int next_head;

    for( copyed_len = 0; copyed_len < read_len; )
    {
        if( i == pDes->RX_tail )
        {
            break;
        }

        //复制一个
        pbuffer[ copyed_len ] = pDes->RX_buffer[ i ];
        copyed_len++;

        //计算next head
        i = (i+1)%UART_BUFFER_SIZE;
    }
    //计算下次读取起点
    next_head = (pDes->RX_head + copyed_len)%UART_BUFFER_SIZE;

    pDes->RX_head = next_head;
    return copyed_len;
}


/************************************************************************************
* ProcName    :获取缓冲区剩余空间长度
* Description : 
* Arguments   : 
* Returns     :             
************************************************************************************/
static int UART_driver_get_RX_buffer_len( UART_descriptor_S *pDes )
{
    int temp_tail = pDes->RX_tail;

    if( temp_tail == pDes->RX_head )
    {
        return 0;
    }
    else if( temp_tail > pDes->RX_head )
    {
        return temp_tail - pDes->RX_head;
    }
    else
    {
        return temp_tail + UART_BUFFER_SIZE - pDes->RX_head;
    }
}


/************************************************************************************
* ProcName    :
* Description : 
* Arguments   : 
* Returns     :             
************************************************************************************/
void UART_driver_RX_byte( UART_descriptor_S *pDes, uint8_t ch )
{
    int next_tail;

    //若新数据入队，计算一下新和tail将会到达哪边
    next_tail = pDes->RX_tail + 1;
    if( UART_BUFFER_SIZE == next_tail )
    {
        next_tail = 0;
    }

    //判断Buffer是否已经满了
    if( next_tail == pDes->RX_head )
    {
        return;
    }

    //插入数据到最后
    pDes->RX_buffer[ pDes->RX_tail ] = ch;
    pDes->RX_tail = next_tail;

    //判断是否要通知应用程序
    if( pDes->RX_notice_len )
    {
        if( pDes->RX_notice_len <= UART_driver_get_RX_buffer_len( pDes ) )
        {
            if( pDes->pRX_Sem )
            {
                osSemaphoreRelease( *( pDes->pRX_Sem ) );
            }

            pDes->RX_notice_len = 0;
        }
    }
}

/************************************************************************************
* ProcName    :
* Description : 
* Arguments   : 
* Returns     :             
************************************************************************************/
//void UART_set_single_wire_half_duplex_enable( UART_TTY_e tty )
//{
//    UART_descriptor_S *pDes = &( UART_des[ tty ] );
//    
//    if( tty >= UART_COMMON_COUNT )
//    {
//        return;
//    }
//    
//    SET_BIT( pDes->huart->Instance->CR3, ( UART_HALF_DUPLEX_ENABLE | USART_CR3_HDSEL ));
//}

