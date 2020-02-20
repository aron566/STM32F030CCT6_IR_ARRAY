#include "GN_uart.h"

extern UART_descriptor_S UART_des[  ];
void UART_driver_RX_byte( UART_descriptor_S *pDes, uint8_t ch );


UART_TTY_e GN_UART_get_tty_NO( UART_HandleTypeDef *huart )
{
    int i;
    for( i = UART_TTY0; i < UART_TTY_COUNT; i++ )
    {
        if( UART_des[ i ].huart == huart ) 
        {
            return (UART_TTY_e)i;
        }
    }
    
    return UART_TTY_ERROR;
}



/**
  * @brief Rx Transfer completed callback.
  * @param huart UART handle.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  UART_TTY_e tty;
  UART_descriptor_S *pUART_des = NULL;
  
  tty = GN_UART_get_tty_NO( huart );
  pUART_des = &( UART_des[ tty ] );
  
  UART_driver_RX_byte( pUART_des, pUART_des->RX_temp );
  
  HAL_UART_Receive_IT( huart, &( pUART_des->RX_temp ), 1 );
  
}


/**
  * @brief Tx Transfer completed callback.
  * @param huart UART handle.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    UART_TTY_e tty;
    UART_descriptor_S *pDes = NULL;
    int send_len = 0;
    int head_temp;
    int i;
    
    tty = GN_UART_get_tty_NO( huart );
    pDes = &( UART_des[ tty ] );
    
    //检测大缓冲区的大小
    send_len = ( pDes->TX_tail + UART_BUFFER_SIZE - pDes->TX_head ) % UART_BUFFER_SIZE;
    if( send_len > UART_TXING_BUFFER_SIZE ) send_len = UART_TXING_BUFFER_SIZE;
    
    
    //从大缓冲区搬到小缓冲区
    if( send_len > 0 )
    {    
        head_temp = pDes->TX_head;
        for( i = 0; i < send_len; i++ )
        {
            pDes->TXING_buffer[ i ] = pDes->TX_buffer[ head_temp ];
            head_temp = ( head_temp + 1 )%UART_BUFFER_SIZE;
        }
        
        pDes->TX_head = head_temp;
        pDes->TXING_buffer_Len = send_len;
        
        //开始发送
        HAL_UART_Transmit_IT( pDes->huart, pDes->TXING_buffer, pDes->TXING_buffer_Len );
    }
    else
    {
        //如果是半双工，在发送结束时，先配置为接收状态，再抛出信号量
        if( 0 != pDes->is_half_duplex )
        {
            HAL_HalfDuplex_EnableReceiver( pDes->huart );
            if( pDes->pRX_Sem )
            {
                osSemaphoreRelease( *( pDes->pRX_Sem ) );
            }
        }
    }
}


/**
  * @brief UART error callback.
  * @param huart UART handle.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_ErrorCallback can be implemented in the user file.
   */
}



