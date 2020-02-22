/*
 *  FILE: UART_Port.h
 *
 *  Created on: 2020/2/22
 *
 *         Author: aron66
 *
 *  DESCRIPTION:--
 */
#ifndef UART_PORT_H
#define UART_PORT_H
#ifdef __cplusplus //use C compiler
extern "C" {
#endif
/*��ӿ�*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
/*�ⲿ�ӿ�*/
#include "usart.h"
#include "cmsis_os.h"
/*�ڲ��ӿ�*/
#include "CircularQueue.h"
    
#define UART_MAX_NUM    6
    
typedef enum
{
    UART0 = 0,
    UART1,
    UART2,
    UART3,
    UART4,
    UART5,
    UART6,
}Uart_num_t;

typedef struct
{
    UART_HandleTypeDef *phuart;      //uart�˿�
    DMA_HandleTypeDef  *phdma_rx;
    CQ_handleTypeDef *cb;           //���ζ���
    uint8_t *RX_Buff_Temp;          //���ջ���
    uint16_t MAX_RX_Temp;           //����������
    int Is_Half_Duplex;             //��˫��ģʽ
    osSemaphoreId *pRX_Sem;         //���ն�ֵ�ź���
}Uart_Dev_info_t;

void Uart_Port_Init(void);
#ifdef __cplusplus //end extern c
}
#endif
#endif