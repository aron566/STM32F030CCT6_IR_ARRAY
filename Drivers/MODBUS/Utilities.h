/*
 *  Utilities.h
 *
 *  Created on: 2020��2��17��
 *      Author: Aron566
 *
 *
 */
#ifndef UTILITIES_H_
#define UTILITIES_H_

#ifdef __cplusplus //ʹ�ã����
extern "C" {
#endif
#include "modbus_type.h"
#include "stdio.h"
/*CRC16 У��*/
uint8_t return_check_crc(uint8_t *msg ,uint16_t len);/*ֱ�ӷ���У����*/
unsigned short CRC_Return(unsigned char *Crc_Buf, unsigned short Crc_Len);

/*���Դ�ӡ*/
void debug_print(uint8_t *msg,uint8_t msg_len);

/*����ͨѶ�˿�������*/
uint8_t get_end_point(UART_HandleTypeDef *fd);

#ifdef __cplusplus //ʹ�ã����
}
#endif

#endif