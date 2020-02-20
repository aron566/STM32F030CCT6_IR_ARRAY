/*
 *  modbus_type.h
 *
 *  Created on: 2020��1��11��
 *      Author: Aron566
 */


#ifndef MODBUS_TYPE_H_
#define MODBUS_TYPE_H_

#ifdef __cplusplus //ʹ�ã����
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
    UART_HandleTypeDef *fd;/*ͨѶ�˿ں�*/
}polling_msg_t;


typedef struct modbus_master_rec_
{
	int     rec_data;     //ʵʱ����
	uint8_t *data_addr;   //����ָ��-�������Ĵ���ʹ��
	uint16_t channnel;    //��������ͨ��
	uint8_t cmd;          //modbus����
	uint8_t *buff_addr;   //���0x13������
	uint8_t channel_count;//���0x13������
}modbus_master_rec_t;

/*���ӳ��Ĵ�����ַ�봦����    ������Ϊslave*/
extern modbus_process_t SlaveReg_process_map[]; 
/*���ӳ��Ĵ�����ַ�봦����    ������Ϊmaster*/
extern modbus_process_t MasterReg_process_map[];


/*�������������Ѱ��վ�Ĵ����б�*/
extern modbus_polling_slave_t access_reg_arr[];

/*������Ѱ*/
extern polling_msg_t polling_msg[];

#if USE_TCP_SEQUENS_NUM
/*ͨѶ���*/
extern volatile uint16_t msg_sequens_num;
#endif



   


#ifdef __cplusplus //ʹ�ã����
}
#endif


#endif