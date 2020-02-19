/*
 *  task_master.c
 *
 *  Created on: 2020年2月17日
 *      Author: Aron566
 *
 *
 */

#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif
    
#include "modbus_type.h"
#include "CircularQueue.h"/*包含modbus通讯帧存储*/    

static void broker_send_message_to_slave(uint8_t msg_device_addr ,uint8_t* msg ,int len);


/* modbus解码的LOOP
 *
 * 接收master包并解析【本机作为slave】
 *
 * uart_mode:０正常解析模式,解析master命令,配置运行模式,返回数据，１透传模式，转发master包至下级slave
 *
 * 缓冲区Uart【1】_Send_slave_BUFF
 * */
void* modbus_slave_decode_start(void* data) {
	circular_buffer *cb = (circular_buffer *) (data);
	uint8_t msg_device_addr ,msg_cmd ,data_len;
	uint16_t msg_len = 0;
	uint32_t read_offset = 0;
	uint8_t tmp_buff[128];
	//独立运行模式
	uint16_t reg_start_addr = 0;
	uint16_t reg_len = 0;
	modbus_master_rec_t rec_data_temp;
	while (1) {
#if USE_TCP_SEQUENS_NUM
		if(cb_bytes_can_read(cb) >= 9)
		{
			//解析序列号
			msg_sequens_num = *(((uint8_t*)cb->ptr + (cb->read_offset%cb->count)));
			read_offset = ((cb->read_offset + 1)%cb->count);
			msg_sequens_num <<= 8;
			msg_sequens_num |= *(((uint8_t*)cb->ptr + read_offset));
			//解析地址
			read_offset = ((cb->read_offset + 2)%cb->count);
			msg_device_addr = *(((uint8_t*)cb->ptr + read_offset)); //第3个字节为设备地址
			//解析命令
			read_offset = ((cb->read_offset + 3)%cb->count);
			msg_cmd = *(((uint8_t*)cb->ptr + read_offset)); //第4个字节为功能码
			rec_data_temp.cmd = msg_cmd;

			switch(msg_cmd)
			{
				case 0x03:
					msg_len = 6;//不含CRC字节数
					//判断数据长度 + 消息
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2+2))
					{
						CQ_get_buff_Data(cb, tmp_buff, msg_len+2+2);
						if(return_check_crc(tmp_buff+2 ,msg_len))
						{
							/*获取控制起始地址*/
							reg_start_addr = tmp_buff[2+2];
							reg_start_addr <<= 8;
							reg_start_addr |= tmp_buff[3+2];
							reg_len = tmp_buff[4+2];
							reg_len <<= 8;
							reg_len |= tmp_buff[5+2];
							if(reg_start_addr >= SET_UART_MODE && (reg_start_addr+reg_len) < MODBUS_PROTOL)
							{
								goto _read_self_par;
							}
							if(uart_mode == REC_THROUGH_MODE)
							{
								//透传模式－－代理发送给slave
								debug_print(tmp_buff,msg_len+2+2);
								broker_rw_message(msg_device_addr,tmp_buff+2,msg_len);
							}
							if(uart_mode == REC_NOT_THROUGH_MODE)
							{
								//本机解析模式
								_read_self_par:
								reg_start_addr = tmp_buff[2+2];
								reg_start_addr <<= 8;
								reg_start_addr |= tmp_buff[3+2];
								reg_len = tmp_buff[4+2];
								reg_len <<= 8;
								reg_len |= tmp_buff[5+2];
								rec_data_temp.channnel = msg_device_addr;
								rec_data_temp.cmd = msg_cmd;
								printf("读取寄存器:%u-%u个\n",reg_start_addr,reg_len);
								printf("接收读取命令:");
								debug_print(tmp_buff,msg_len+2+2);
								rec_master_data(&rec_data_temp ,reg_start_addr ,reg_len);
							}
							CQ_read_offset_inc(cb, msg_len+2+2);//偏移一帧报文
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//丢弃１字节
							printf("slave : modbus_0x03　return　CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
					}
					break;
				case 0x13:
					//解析需读取的寄存器数目--非连续
					read_offset = ((cb->read_offset + 4)%cb->count);
					reg_len = (*((uint8_t*)cb->ptr + read_offset));
					//用于CRC校验数据长度
					msg_len = 3+(reg_len*2)+3;
					//判断数据长度
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2+2))
					{
						CQ_get_buff_Data(cb ,tmp_buff ,msg_len+2+2);
						//CRC校验
						if(return_check_crc(tmp_buff+2 ,msg_len))
						{
							rec_data_temp.channnel = tmp_buff[msg_len-3+2];
							rec_data_temp.channnel <<= 8;
							rec_data_temp.channnel |= tmp_buff[msg_len-2+2];
							rec_data_temp.channel_count = tmp_buff[msg_len-1+2];
							rec_data_temp.buff_addr = tmp_buff;
							rec_master_data(&rec_data_temp ,0 ,reg_len);
							CQ_read_offset_inc(cb, msg_len+2+2);//偏移一帧报文
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//丢弃１字节
							printf("slave : modbus_0x13　return　CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
					}
					break;
				case 0x10:
					read_offset = ((cb->read_offset + 6+2)%cb->count);
					data_len = (*((uint8_t*)cb->ptr + read_offset));
					//计算字节：第七元素数据字节数＋前面７个字节数
					msg_len = data_len+7;

					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2+2))
					{
						CQ_get_buff_Data(cb, tmp_buff, msg_len+2+2);
						if(return_check_crc(tmp_buff+2 ,msg_len))//校验正确
						{
							/*获取控制起始地址*/
							reg_start_addr = tmp_buff[2+2];
							reg_start_addr <<= 8;
							reg_start_addr |= tmp_buff[3+2];
							if(reg_start_addr >= SET_UART_MODE && (reg_start_addr+data_len/2) < MODBUS_PROTOL)
							{
								goto _set_self_par;
							}
							if(uart_mode == REC_THROUGH_MODE)//透传模式－－修改ID CRC计算后发送给master
							{
								broker_rw_message(msg_device_addr,tmp_buff+2,msg_len);
							}
							if(uart_mode == REC_NOT_THROUGH_MODE)//本机解析模式
							{
								_set_self_par:
//								pthread_mutex_lock(&GNNC_REC_Data_mutex_lock);
								reg_start_addr = tmp_buff[2+2];
								reg_start_addr <<= 8;
								reg_start_addr |= tmp_buff[3+2];
								reg_len = tmp_buff[4+2];
								reg_len <<= 8;
								reg_len |= tmp_buff[5+2];
								rec_data_temp.channnel = msg_device_addr;
								rec_data_temp.cmd = msg_cmd;
								rec_data_temp.rec_data = 0;
								rec_data_temp.data_addr = &tmp_buff[7+2];
								printf("控制数据:0x%02X-0x%02X\n",tmp_buff[7+2],tmp_buff[8+2]);
								printf("接收控制命令:");
								debug_print(tmp_buff,msg_len+2+2);
								rec_master_data(&rec_data_temp ,reg_start_addr ,reg_len);
								CQ_read_offset_inc(cb, msg_len+2+2);//偏移一帧报文
//								pthread_mutex_unlock(&GNNC_REC_Data_mutex_lock);
							}
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//丢弃１字节
							printf("slave : modbus_0x10　return　CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
					}
					break;
				default:
					debug_print((uint8_t*)((uint8_t*)cb->ptr + cb->read_offset),8+2);
					CQ_read_offset_inc(cb, 1);//什么都不是－>丢弃１字节
					printf("slave : modbus_0x**　return　CRC error!\n");
					break;
			}
		}
		modbusDelay(100);
#else
		//比较当前可读数据长度=当前写入长度-已读长度）大于7即可进入,符合modbus协议最小长度
		if(cb_bytes_can_read(cb) >= 7)
		{
			msg_device_addr = *(((uint8_t*)cb->ptr + (cb->read_offset%cb->count))); //第一个字节为设备地址
			read_offset = ((cb->read_offset + 1)%cb->count);
			msg_cmd = *(((uint8_t*)cb->ptr + read_offset)); //第二个字节为功能码
			rec_data_temp.cmd = msg_cmd;
			switch(msg_cmd)
			{
				case 0x03:
					msg_len = 6;//不含CRC字节数
					//判断数据长度
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2))
					{
						CQ_get_buff_Data(cb, tmp_buff, msg_len+2);
						if(return_check_crc(tmp_buff ,msg_len))
						{
							/*获取控制起始地址*/
							reg_start_addr = tmp_buff[2];
							reg_start_addr <<= 8;
							reg_start_addr |= tmp_buff[3];
							reg_len = tmp_buff[4];
							reg_len <<= 8;
							reg_len |= tmp_buff[5];
							if(reg_start_addr >= SET_UART_MODE && (reg_start_addr+reg_len) < MODBUS_PROTOL)
							{
								goto _read_self_par;
							}
							if(uart_mode == REC_THROUGH_MODE)
							{
								//透传模式－－代理发送给slave
								broker_rw_message(msg_device_addr,tmp_buff,msg_len);
							}
							if(uart_mode == REC_NOT_THROUGH_MODE)
							{
								//本机解析模式
								_read_self_par:
								reg_start_addr = tmp_buff[2];
								reg_start_addr <<= 8;
								reg_start_addr |= tmp_buff[3];
								reg_len = tmp_buff[4];
								reg_len <<= 8;
								reg_len |= tmp_buff[5];
								rec_data_temp.channnel = msg_device_addr;
								rec_data_temp.cmd = msg_cmd;
								printf("读取寄存器:%u-%u个\n",reg_start_addr,reg_len);
								printf("接收读取命令:");
								debug_print(tmp_buff,msg_len+2);
								rec_master_data(&rec_data_temp ,reg_start_addr ,reg_len);
							}
							CQ_read_offset_inc(cb, msg_len+2);//偏移一帧报文
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//丢弃１字节
							printf("slave : modbus_0x03　return　CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
						error_flag++;
					}
					break;
				case 0x13:
					//解析需读取的寄存器数目--非连续
					read_offset = ((cb->read_offset + 2)%cb->count);
					reg_len = (*((uint8_t*)cb->ptr + read_offset));
					//CRC校验
					msg_len = 3+(reg_len*2)+3;
					//判断数据长度
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2))
					{
						CQ_get_buff_Data(cb, (uint8_t *)tmp_buff, msg_len+2);
						if(return_check_crc(tmp_buff ,msg_len))//校验正确
						{
							rec_data_temp.channnel = tmp_buff[msg_len-3];
							rec_data_temp.channnel <<= 8;
							rec_data_temp.channnel |= tmp_buff[msg_len-2];
							rec_data_temp.channel_count = tmp_buff[msg_len-1];
							rec_data_temp.buff_addr = tmp_buff;
							rec_master_data(&rec_data_temp ,0 ,reg_len);
							CQ_read_offset_inc(cb, msg_len+2);//偏移一帧报文
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//丢弃１字节
							printf("slave : modbus_0x13　return　CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
						error_flag++;
					}
					break;
				case 0x10:
					read_offset = ((cb->read_offset + 6)%cb->count);
					data_len = (*((uint8_t*)cb->ptr + read_offset));
					//计算字节：第七元素数据字节数＋前面７个字节数
					msg_len = data_len+7;
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2))
					{
						CQ_get_buff_Data(cb, (uint8_t *)tmp_buff, msg_len+2);
						if(return_check_crc(tmp_buff ,msg_len))//校验正确
						{
							/*获取控制起始地址*/
							reg_start_addr = tmp_buff[2];
							reg_start_addr <<= 8;
							reg_start_addr |= tmp_buff[3];
							if(reg_start_addr >= SET_UART_MODE && (reg_start_addr+data_len/2) < MODBUS_PROTOL)
							{
								goto _set_self_par;
							}
							if(uart_mode == REC_THROUGH_MODE)//透传模式－－修改ID CRC计算后发送给master
							{
								broker_rw_message(msg_device_addr,tmp_buff,msg_len);
							}
							if(uart_mode == REC_NOT_THROUGH_MODE)//本机解析模式
							{
								_set_self_par:
								reg_start_addr = tmp_buff[2];
								reg_start_addr <<= 8;
								reg_start_addr |= tmp_buff[3];
								reg_len = tmp_buff[4];
								reg_len <<= 8;
								reg_len |= tmp_buff[5];
								rec_data_temp.channnel = msg_device_addr;
								rec_data_temp.cmd = msg_cmd;
								rec_data_temp.rec_data = 0;
								rec_data_temp.data_addr = &tmp_buff[7];
								printf("控制数据:0x%02X-0x%02X\n",tmp_buff[7],tmp_buff[8]);
								printf("接收控制命令:");
								debug_print(tmp_buff,msg_len+2);
								rec_master_data(&rec_data_temp ,reg_start_addr ,reg_len);
								CQ_read_offset_inc(cb, msg_len+2);//偏移一帧报文
							}
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//丢弃１字节
							printf("slave : modbus_0x10　return　CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
						error_flag++;
					}
					break;
				default:
					debug_print((uint8_t*)((uint8_t*)cb->ptr + cb->read_offset),8);
					CQ_read_offset_inc(cb, 1);//什么都不是－>丢弃１字节
					printf("slave : modbus_0x**　return　CRC error!\n");
					break;
			}
		}
		modbusDelay(100);
#endif
	}//end while
	printf("ERROR !!!\n");
	return NULL;
}


/*
 * 本机作为slave独立解析主站数据包 --响应 控制和读取操作
 * 参数：rec_struct 需要将 写入数据的起始地址传入（当为写入命令） | 命令类型 | 通道号（当需要读取指定通道时）
 * 参数:reg_begin_addr 起始寄存器地址
 * 参数:len 寄存器数量长度
 * */
static void rec_master_data(modbus_master_rec_t *rec_struct ,uint16_t reg_begin_addr ,uint16_t len)
{
	uint8_t ack_data_buff[256] = {0};
	uint16_t crc_ret = 0;
	uint16_t read_offset = 0;
	uint16_t reg_addr = reg_begin_addr;
	uint16_t reg_end = reg_addr+len;
	uint8_t *data_addr = rec_struct->data_addr;
#if USE_TCP_SEQUENS_NUM
	//准备带序号头部信息
	ack_data_buff[0] = (msg_sequens_num>>8)&0xFF;
	ack_data_buff[1] = msg_sequens_num&0xFF;
	ack_data_buff[0+2] = rec_struct->channnel&0xFF;
	ack_data_buff[1+2] = rec_struct->cmd;
	//写入命令
	if(ack_data_buff[1+2] == 0x10)
	{
		if(data_addr == NULL || len == 0)
		{
			printf("参数错误!\n");
			return;
		}
#if ENABLE_MODBUS_DEBUG
		printf("通道:%02X-命令:%02X-数据:%02X-数据:%02X\n",rec_struct->channnel,rec_struct->cmd,*rec_struct->data_addr,*(rec_struct->data_addr+1));
		printf("===处理寄存器值:%d,数量:%d===\n",reg_addr,len);
#endif
		for(;reg_addr < reg_end ;reg_addr++)
		{
			rec_struct->rec_data = *(data_addr+read_offset)&0xFF;
			rec_struct->rec_data <<= 8;
			rec_struct->rec_data |= (*(data_addr+read_offset+1))&0xFF;
#if ENABLE_MODBUS_DEBUG
			printf("==写入寄存器值:%04X,寄存器:%d==\n",rec_struct->rec_data,reg_addr);
#endif
			reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
			read_offset += 2;
		}
		//寄存器地址
		ack_data_buff[2+2] = ((reg_begin_addr>>8)&0xFF);
		ack_data_buff[3+2] = (reg_begin_addr&0xFF);
		//寄存器数量
		uint16_t reg_num = len;
		ack_data_buff[4+2] = ((reg_num>>8)&0xFF);
		ack_data_buff[5+2] = reg_num&0xFF;
		//校验CRC
		crc_ret = CRC_Return(ack_data_buff+2,6);
		ack_data_buff[6+2] =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
		ack_data_buff[7+2] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK发送至上位机
		tcp_client_tx(sockfd, ack_data_buff, 8+2);//发送至本机上级
	}
	//读取命令
	if(ack_data_buff[1+2] == 0x03)
	{
		if(len == 0)
		{
			printf("参数错误,长度为0!\n");
			return;
		}
		uint16_t ret_data = 0;
		//寄存器数据从第4+2个字节开始填充 索引为3+2
		read_offset = 3+2;
		//字节数
		ack_data_buff[2+2] = len*2;

		//读取寄存器数据 填充
		for(reg_addr = reg_begin_addr;reg_addr < reg_end ;reg_addr++)
		{
			ret_data = reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
//			printf("读取寄存器值:%d,值:%d\n",reg_addr,ret_data);
			ack_data_buff[read_offset++] = ((ret_data>>8)&0xFF);
			ack_data_buff[read_offset++] = (ret_data&0xFF);
		}

		//校验CRC
		crc_ret = CRC_Return(ack_data_buff+2,read_offset-2);
		ack_data_buff[read_offset++] =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
		ack_data_buff[read_offset++] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK发送至上位机
		tcp_client_tx(sockfd, ack_data_buff ,read_offset);//发送至本机上级
	}
	if(ack_data_buff[1+2] == 0x13)
	{
		if(len == 0)
		{
			printf("参数错误,长度为0!\n");
			return;
		}
		//重构头部
		ack_data_buff[0+2] = rec_struct->buff_addr[2];
		//回复字节数 = 寄存器数量*通道数量
		ack_data_buff[2+2] = (len*2*rec_struct->channel_count)&0xFF;
		read_offset = 3+2;
		uint16_t ret_data = 0;
		uint16_t reg_addr = 0;

		//按查询通道 填充
		uint16_t channel_end = rec_struct->channnel+rec_struct->channel_count;
		for(;rec_struct->channnel < channel_end;rec_struct->channnel++)
		{
			//查询len个寄存器 填充数据 提供首个寄存器索引号
			uint8_t reg_addr_index = 3+2;
			for(uint8_t i = 3;i < (len+3);i++)
			{
				reg_addr = rec_struct->buff_addr[reg_addr_index];
				reg_addr <<= 8;
				reg_addr |=  rec_struct->buff_addr[reg_addr_index+1];
				ret_data = reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
#if ENABLE_MODBUS_DEBUG
				printf("读取通道%d--寄存器:%d,值:%04X\n",rec_struct->channnel,reg_addr,ret_data);
#endif
				ack_data_buff[read_offset++] = ((ret_data>>8)&0xFF);
				ack_data_buff[read_offset++] = (ret_data&0xFF);
				//查询下一个寄存器
				reg_addr_index += 2;
			}
		}
		//校验CRC
		crc_ret = CRC_Return(ack_data_buff+2,read_offset-2);
		ack_data_buff[read_offset++] =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
		ack_data_buff[read_offset++] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK发送至上位机
		tcp_client_tx(sockfd, ack_data_buff ,read_offset);//发送至本机上级

	}
#else
	//准备通用头部应答消息
	ack_data_buff[0] = rec_struct->channnel&0xFF;
	ack_data_buff[1] = rec_struct->cmd;
	//写入命令
	if(ack_data_buff[1] == 0x10)
	{
		if(data_addr == NULL || len == 0)
		{
			return;
		}
#if ENABLE_MODBUS_DEBUG
		printf("通道:%02X-命令:%02X-数据:%02X-数据:%02X\n",rec_struct->channnel,rec_struct->cmd,*rec_struct->data_addr,*(rec_struct->data_addr+1));
		printf("===处理寄存器值:%d,数量:%d===\n",reg_addr,len);
#endif
		for(;reg_addr < reg_end ;reg_addr++)
		{
			rec_struct->rec_data = *(data_addr+read_offset)&0xFF;
			rec_struct->rec_data <<= 8;
			rec_struct->rec_data |= (*(data_addr+read_offset+1))&0xFF;
			printf("==写入寄存器值:%04X,寄存器:%d==\n",rec_struct->rec_data,reg_addr);
			reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
			read_offset += 2;
		}
		//寄存器地址
		ack_data_buff[2] = ((reg_begin_addr>>8)&0xFF);
		ack_data_buff[3] = (reg_begin_addr&0xFF);
		//寄存器数量
		uint16_t reg_num = len;
		ack_data_buff[4] = ((reg_num>>8)&0xFF);
		ack_data_buff[5] = reg_num&0xFF;
		//校验CRC
		crc_ret = CRC_Return(ack_data_buff,6);
		ack_data_buff[6] =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
		ack_data_buff[7] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK发送至上位机
		tcp_client_tx(sockfd, ack_data_buff, 8);//发送至本机上级
	}
	//读取命令
	if(ack_data_buff[1] == 0x03)
	{
		if(len == 0)
		{
			return;
		}
		uint16_t ret_data = 0;
		//寄存器数据从第4个字节开始填充 索引为3
		read_offset = 3;
		//字节数
		ack_data_buff[2] = len*2;

		//读取寄存器数据 填充
		for(reg_addr = reg_begin_addr;reg_addr < reg_end ;reg_addr++)
		{
			ret_data = reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
			printf("读取寄存器值:%d,值:%d\n",reg_addr,ret_data);
			ack_data_buff[read_offset++] = ((ret_data>>8)&0xFF);
			ack_data_buff[read_offset++] = (ret_data&0xFF);
		}

		//校验CRC
		crc_ret = CRC_Return(ack_data_buff,read_offset);
		ack_data_buff[read_offset++] =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
		ack_data_buff[read_offset++] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK发送至上位机
		tcp_client_tx(sockfd, ack_data_buff ,read_offset);//发送至本机上级
	}
	if(ack_data_buff[1] == 0x13)
	{
		if(len == 0)
		{
			return;
		}
		//重构头部
		ack_data_buff[0] = rec_struct->buff_addr[0];
		//回复字节数 = 寄存器数量*通道数量
		ack_data_buff[2] = (len*2*rec_struct->channel_count)&0xFF;
		read_offset = 3;
		uint16_t ret_data = 0;
		uint16_t reg_addr = 0;

		//按查询通道 填充
		uint16_t channel_end = rec_struct->channnel+rec_struct->channel_count;
		for(;rec_struct->channnel < channel_end;rec_struct->channnel++)
		{
			//查询len个寄存器 填充数据 提供首个寄存器索引号
			uint8_t reg_addr_index = 3;
			for(uint8_t i = 3;i < (len+3);i++)
			{
				reg_addr = rec_struct->buff_addr[reg_addr_index];
				reg_addr <<= 8;
				reg_addr |=  rec_struct->buff_addr[reg_addr_index+1];
				ret_data = reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
#if ENABLE_MODBUS_DEBUG
				printf("读取通道%d--寄存器:%d,值:%04X\n",rec_struct->channnel,reg_addr,ret_data);
#endif
				ack_data_buff[read_offset++] = ((ret_data>>8)&0xFF);
				ack_data_buff[read_offset++] = (ret_data&0xFF);
				//查询下一个寄存器
				reg_addr_index += 2;
			}
		}
		//校验CRC
		crc_ret = CRC_Return(ack_data_buff,read_offset);
		ack_data_buff[read_offset++] =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
		ack_data_buff[read_offset++] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK发送至上位机
		tcp_client_tx(sockfd, ack_data_buff ,read_offset);//发送至本机上级

	}
#endif
}    

/* 透传 模式下
 * 转发读写数据 0x03 0x10
 * */
static void broker_rw_message(int channel,uint8_t *ptr,uint16_t msglen)
{
	uint16_t crc_ret = 0;
	uint8_t Uart_Send_slave_BUFF[UART_SEND_BUFF_MAX];
	//复制数据
	memcpy(Uart_Send_slave_BUFF,(uint8_t*)ptr,msglen);
	//修改数据
	Uart_Send_slave_BUFF[0] = 0XFF;//更换固定头部设备ID
	crc_ret = CRC_Return(Uart_Send_slave_BUFF,msglen);
	Uart_Send_slave_BUFF[msglen] =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
	Uart_Send_slave_BUFF[msglen+1] = (uint8_t)((crc_ret>>8)&0x00FF);
	//打印数据
	debug_print((uint8_t*)ptr,msglen+2);
	printf("slave : 转换!\n");
	debug_print((uint8_t*)Uart_Send_slave_BUFF,msglen+2);
	//发送数据
	broker_send_message_to_slave(channel,Uart_Send_slave_BUFF,msglen+2);
}

/*
 * 解析master数据转发至正确端口slave
 *
 * msg_device_addr数据头部
 *
 * msg消息体，len数据长度
 * */
static void broker_send_message_to_slave(uint8_t msg_device_addr ,uint8_t* msg ,int len)
{
	if(msg_device_addr >= 0x01 && msg_device_addr <= UART_NUM_MAX)
	{
		usart_tx(polling_msg[msg_device_addr].cb->fd ,msg ,len);
	}
}


#ifdef __cplusplus //使用ｃ编译
}
#endif