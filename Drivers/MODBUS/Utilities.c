/*
 *  Utilities.c
 *
 *  Created on: 2020年2月17日
 *      Author: Aron566
 *
 *
 */

#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif

#include "Utilities.h"
#include "modbus_port.h" 
    
static uint16_t Crc_Cal(unsigned short Data, unsigned short GenPoly, unsigned short CrcData);



/*
 *
 * fd所属端口
 *
 * 返回所属索引号
 * */
uint8_t get_end_point(UART_HandleTypeDef *fd)
{
	uint8_t uart_endpoint = 0;
	for(uint8_t i = 0;i < UART_NUM_MAX+1;i++)
	{
		if(polling_msg[i].fd == fd)
		{
			uart_endpoint = i;
			break;
		}
	}
	if(uart_endpoint == 0)
	{
		printf("end_pointer error FD:%p\n",fd);
	}
	return uart_endpoint;
}


/*
 * modbus CRC校验
 * */
static uint16_t Crc_Cal(unsigned short Data, unsigned short GenPoly, unsigned short CrcData)
{
	unsigned short TmpI;
	Data *= 2;
	for (TmpI = 8; TmpI > 0; TmpI--) {
		Data = Data / 2;
		if ((Data ^ CrcData) & 1)
			CrcData = (CrcData / 2) ^ GenPoly;
		else
			CrcData /= 2;
	}
	return CrcData;
}

unsigned short CRC_Return(uint8_t *Crc_Buf, unsigned short Crc_Len)
{
        unsigned short temp;
        unsigned short CRC_R = 0xFFFF;
        for (temp = 0; temp < Crc_Len; temp++)
        {
                CRC_R = Crc_Cal(Crc_Buf[temp], 0xA001, CRC_R);
        }
        return CRC_R;
}

/*
 * 校验CRC 16
 *
 * 返回1 正确 返回0 错误
 * */
uint8_t return_check_crc(uint8_t *msg ,uint16_t len)
{
	uint8_t CRC_value_L,CRC_value_H,CRC_value_L_temp,CRC_value_H_temp;
	uint16_t crc_ret = 0;
	crc_ret = CRC_Return(msg,len);
	CRC_value_L_temp = *(msg+len);
	CRC_value_H_temp = *(msg+len+1);
	CRC_value_L =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
	CRC_value_H = (uint8_t)((crc_ret>>8)&0x00FF);
	if(CRC_value_L == CRC_value_L_temp && CRC_value_H == CRC_value_H_temp)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void debug_print(uint8_t *msg,uint8_t msg_len)
{
	for (int i = 0; i < msg_len; i++)
	printf("%02X ",
			msg[i]); //打印2位的16进制数，不足位0补齐。
	printf("\n");
}

#ifdef __cplusplus //使用ｃ编译
}
#endif
