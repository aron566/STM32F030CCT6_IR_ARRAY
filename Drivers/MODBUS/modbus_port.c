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
#include "modbus_port.h" /*�ⲿ����*/
#include "modbus_type.h"    




/*
 * ���ڲ�������
 * */
volatile uint16_t uart_mode = REC_NOT_THROUGH_MODE;//���ö���ģʽ    

void init_modbus_controlBlock(void)
{
    /*����������*/
	polling_msg[1].cb = cb_create(256);//256Byte
	polling_msg[2].cb = cb_create(256);
	polling_msg[3].cb = cb_create(256);
	polling_msg[4].cb = cb_create(256);
	polling_msg[5].cb = cb_create(256);
	polling_msg[6].cb = cb_create(256);

    /*�򿪴���*/
    __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);   //�򿪿����ж�
    HAL_UART_Receive_DMA(&huart1, (uint8_t*)UART1_BUFF, UART1_BUFF_MAX);

}


/*��ֲms��ʱ*/    
void modbusDelay(uint32_t Delayms) 
{
    osDelay(Delayms);
}

int ModbusUsart_tx(UART_HandleTypeDef *fd,unsigned char* msg , int len)
{

}
#ifdef __cplusplus //end extern c
}
#endif