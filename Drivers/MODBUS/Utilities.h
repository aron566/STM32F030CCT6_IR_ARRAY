/*
 *  Utilities.h
 *
 *  Created on: 2020年2月17日
 *      Author: Aron566
 *
 *
 */
#ifndef UTILITIES_H_
#define UTILITIES_H_

#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif
#include "modbus_type.h"
#include "stdio.h"
/*CRC16 校验*/
uint8_t return_check_crc(uint8_t *msg ,uint16_t len);/*直接返回校验结果*/
unsigned short CRC_Return(unsigned char *Crc_Buf, unsigned short Crc_Len);

/*调试打印*/
void debug_print(uint8_t *msg,uint8_t msg_len);

/*查找通讯端口索引号*/
uint8_t get_end_point(UART_HandleTypeDef *fd);

#ifdef __cplusplus //使用ｃ编译
}
#endif

#endif