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
#include "CircularQueue.h"/*����modbusͨѶ֡�洢*/ 
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#define UART_NUM_MAX            6U      /*��󴮿��� 1-6*/
#define REC_THROUGH_MODE        0x01U   /*͸��ģʽ*/
#define REC_NOT_THROUGH_MODE    0x00U   /*��͸��ģʽ*/
extern volatile uint16_t uart_mode;

/*modbus��ʼ��*/
void init_modbus_controlBlock(void);

/*��ʱʵ��*/
void modbusDelay(uint32_t Delayms);    
    
/**/    
#ifdef __cplusplus //end extern c
}
#endif
#endif