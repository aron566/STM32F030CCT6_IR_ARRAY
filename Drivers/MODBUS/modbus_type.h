/*
 *  modbus_type.h
 *
 *  Created on: 2020年1月11日
 *      Author: Aron566
 */


#ifndef MODBUS_TYPE_H_
#define MODBUS_TYPE_H_

#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "modbus_port.h"
    
typedef uint16_t (*p_modbus_process)(void *data);
    
typedef struct modbus_process_
{
	uint16_t register_num;
	p_modbus_process func;
}modbus_process_t;

typedef struct modbus_polling_slave
{
	uint16_t access_reg_addr;
	uint16_t access_num;
}modbus_polling_slave_t;

typedef struct
{
	volatile uint16_t read_count;
	volatile uint16_t read_reg;
	volatile uint16_t read_channel;
	CQ_handleTypeDef   *cb;
    UART_HandleTypeDef *fd;/*通讯端口号*/
}polling_msg_t;


typedef struct modbus_master_rec_
{
	int     rec_data;     //实时数据
	uint8_t *data_addr;   //数据指针-处理多个寄存器使用
	uint16_t channnel;    //数据来自通道
	uint8_t cmd;          //modbus命令
	uint8_t *buff_addr;   //配合0x13功能码
	uint8_t channel_count;//配合0x13功能码
}modbus_master_rec_t;

/*填充映射寄存器地址与处理方法    本机作为slave*/
extern modbus_process_t SlaveReg_process_map[]; 
/*填充映射寄存器地址与处理方法    本机作为master*/
extern modbus_process_t MasterReg_process_map[];


/*填充主机主动轮寻从站寄存器列表*/
extern modbus_polling_slave_t access_reg_arr[];

/*本机轮寻*/
extern polling_msg_t polling_msg[];

#if USE_TCP_SEQUENS_NUM
/*通讯序号*/
extern volatile uint16_t msg_sequens_num;
#endif



   


#ifdef __cplusplus //使用ｃ编译
}
#endif


#endif