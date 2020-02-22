/*
 *  FILE: misc_task.h
 *
 *  Created on: 2020/2/20
 *
 *         Author: aron66
 *
 *  DESCRIPTION:--
 */
#ifndef MISC_TASK_H_
#define MISC_TASK_H_

#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif
#include "peripheral.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include "cmsis_os.h"
#include "iwdg.h"
#include "rtc.h" 

#define NORMAL_TEMPERATURE      30*100U //放大100倍
#define MAX_TEMPERATURE         45*100U
#define MAX_CHECK_INTERVAL      3U      //检测间隔秒
#define KEY_DELAY_DEFAULT_VAL   100U  
typedef enum
{
    MAX_VALUE_MODE = 0,
    AVERAGE_VALUE_MODE,
    MIN_VALUE_MODE,
    MEDIAN_VALUE_MODE  
}Calc_Mode_t;

void start_misc_task_loop(void);
void millsec_IRQ_handler(void);
void clear_people_num_day(uint16_t command);
uint8_t Check_Data_Sum(uint8_t *data ,uint16_t len);
uint8_t Key_State_Check(GPIO_TypeDef* GPIOx ,uint16_t GPIO_Pin ,GPIO_PinState state ,uint16_t check_time ,uint16_t *time);
void Read_IR_Temperature_loop(void);

uint32_t get_time_sec();
uint32_t get_time_ms();  
  
#ifdef __cplusplus //use C compiler
extern "C" {
#endif

#ifdef __cplusplus //end extern c
}
#endif
#endif
