/*
 *  FILE: modbus_port.h
 *
 *  Created on: 2020/2/19
 *
 *         Author: aron66
 *
 *  DESCRIPTION:--
 */

#ifndef MODBUS_PORT_H_
#define MODBUS_PORT_H_
#ifdef __cplusplus //use C compiler
extern "C" {
#endif
#include "CircularQueue.h"/*包含modbus通讯帧存储*/ 
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
    
#define UART_NUM_MAX            6U      /*最大串口数 1-6*/
#define REC_THROUGH_MODE        0x01U   /*透传模式*/
#define REC_NOT_THROUGH_MODE    0x00U   /*非透传模式*/
#define UART_BUFF_MAX           256U    /*最大缓冲大小*/
extern volatile uint16_t uart_mode;

/*modbus初始化*/
void init_modbus_controlBlock(void);

/*延时实现*/
void modbusDelay(uint32_t Delayms);    

/*串口数据发送实现*/
int ModbusUsart_tx(UART_HandleTypeDef *fd,uint8_t *msg , uint16_t len);

/*串口数据发送至主机*/
int ModbusUsart_tx_Master(uint8_t *msg , uint16_t len);

#ifdef __cplusplus //end extern c
}
#endif
#endif