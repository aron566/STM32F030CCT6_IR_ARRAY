/*
 *  FILE: modbus_port.c
 *
 *  Created on: 2020/2/19
 *
 *         Author: aron66
 *
 *  DESCRIPTION:--
 */
#ifdef __cplusplus //use C compiler
extern "C" {
#endif
#include "modbus_port.h" /*外部方法*/
#include "modbus_type.h"    
#include "Utilities.h"
polling_msg_t polling_msg[UART_NUM_MAX+1];/*创建轮询数据结构*/


/*
 * 串口参数设置
 * */
volatile uint16_t uart_mode = REC_NOT_THROUGH_MODE;//配置独立模式    

void init_modbus_controlBlock(void)
{
    /*建立缓冲区*/
	polling_msg[1].cb = cb_create(UART_BUFF_MAX);//256Byte
	polling_msg[2].cb = cb_create(UART_BUFF_MAX);
	polling_msg[3].cb = cb_create(UART_BUFF_MAX);
	polling_msg[4].cb = cb_create(UART_BUFF_MAX);
	polling_msg[5].cb = cb_create(UART_BUFF_MAX);
	polling_msg[6].cb = cb_create(UART_BUFF_MAX);

    /*打开串口接收中断*/
    polling_msg[1].fd = &huart1;
    polling_msg[2].fd = &huart2;
    polling_msg[3].fd = &huart3;
    polling_msg[4].fd = &huart4;
    polling_msg[5].fd = &huart5;
    polling_msg[6].fd = &huart6;
    for(uint8_t i = 1; i < UART_NUM_MAX+1; i++)
    {
        //打开空闲中断
        __HAL_UART_ENABLE_IT(polling_msg[i].fd ,UART_IT_IDLE);   
       if(i != 5)
       {
          HAL_UART_Receive_IT(polling_msg[i].fd ,(uint8_t*)polling_msg[i].cb->dataBufer, UART_BUFF_MAX);
       }
       else
       {
           /*使能DMA接收中断*/
          HAL_UART_Receive_DMA(polling_msg[i].fd ,(uint8_t*)polling_msg[i].cb->dataBufer, UART_BUFF_MAX);
       }
    }

}


/*移植ms延时*/    
void modbusDelay(uint32_t Delayms) 
{
    osDelay(Delayms);
}

int ModbusUsart_tx(UART_HandleTypeDef *fd,uint8_t *msg , uint16_t len)
{
    uint8_t index = get_end_point(fd);
    if(index == 0)
    {
        return -1;
    }
    if(index != 5)
    {
        HAL_UART_Transmit_IT(fd ,msg ,len);
    }
    else
    {
        HAL_UART_Transmit_DMA(fd ,msg ,len);
    }
    return 0;
}

int ModbusUsart_tx_Master(uint8_t *msg , uint16_t len)
{
    HAL_UART_Transmit_DMA(polling_msg[5].fd ,msg ,len);
    return 0;
}

#ifdef __cplusplus //end extern c
}
#endif