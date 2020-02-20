/*
 *  task_master.c
 *
 *  Created on: 2020年2月17日
 *      Author: Aron566
 *
 *  轮询从机--解析返回数据--调用存储
 */

#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif
    
#include "modbus_type.h"/*包含modbus轮询结构体*/
#include "modbus_reg.h"
#include "Utilities.h"/*CRC 及 获取端口号*/
    
/*对从站轮寻寄存器地址信息 配置*/
modbus_polling_slave_t access_reg_arr[] =
{
	{
		.access_reg_addr = REAL_TIME_TEMP,
		.access_num = 1 //连续采集1个寄存器
	},
	{
		.access_reg_addr = IR_MATRIX_GRAPH_START,
		.access_num = 32
	},
	{
		.access_reg_addr = 0,//END
		.access_num = 0
	}
};    

/*本机作为master 轮询从机数据*/
static void self_send_read_reg(uint8_t index ,uint8_t modbusID);

/*本机作为master 解码从机返回的数据*/
static int8_t modbus_master_decode_mannul(void* data);

/*本机作为master 处理来自从站的数据---存储*/
static void rec_slave_data(modbus_master_rec_t *rec_struct ,uint8_t len);

/*转发读取到的数据*/
static void return_rw_message(UART_HandleTypeDef *fd,uint8_t *ptr,uint16_t msglen);

/*
 * modbus 发送与接收处理线程
 *
 * 本机作为master
 *
 * 当处于透传模式,本机解析串口数据,上报主机
 *
 * 当处于独立运行模式,调用发送后进入解析函数,
 *
 * 返回:解析不成功返回-1,成功为0
 * */
void *modbus_polling_task_loop(void* ptmr)
{
	polling_msg_t *pPolling_data = (polling_msg_t *) (ptmr);
	uint8_t time_out = 0;
	uint8_t modbusID = get_end_point(pPolling_data->fd);
	for(;;)
	{
		if(uart_mode == REC_NOT_THROUGH_MODE)
		{
			//循环轮寻从站寄存器
			for(int index = 0;access_reg_arr[index].access_num != 0;index++)
			{
				if(uart_mode != REC_NOT_THROUGH_MODE)break;
				self_send_read_reg(index ,modbusID);
				time_out = 0;
				//解析
				do{
					if(uart_mode != REC_NOT_THROUGH_MODE)break;

					int8_t ret = modbus_master_decode_mannul(pPolling_data);

					switch(ret)
					{
						case 0:
							//累加成功次数
//							device_v_info[modbusID].com_sucess_count++;
							break;
						case -1:
							time_out++;
							if(time_out > 1){
								//累加失败次数
//								device_v_info[modbusID].com_faild_count++;
								printf("未有答复--跳过!!!\n");
							}
							break;
						default:
							printf("what fuck！\n");
							break;
					}
				}while(time_out < 2 && time_out != 0);
				//计算成功率
//				if(!(device_v_info[modbusID].com_sucess_count+device_v_info[modbusID].com_faild_count)%2)
//				{
//					device_v_info[modbusID].device_com_rate_of_sucess = (100*(device_v_info[modbusID].com_sucess_count/(device_v_info[modbusID].com_sucess_count+device_v_info[modbusID].com_faild_count)))&0xFFFF;
//					printf("通道:%u 通讯成功率:\%%u\n",modbusID,device_v_info[modbusID].device_com_rate_of_sucess);
//				}
				CQ_emptyData(pPolling_data->cb);
			}
		}
		else if(uart_mode == REC_THROUGH_MODE)
		{
			modbus_master_decode_mannul(pPolling_data);
		}
		//挂起任务等待数据
		modbusDelay(10);
	}
}

/*
 * 本机作为master ,自身主动轮寻寄存器
 *
 * 参数 index 轮寻寄存器索引 fd_countd fd对应索引
 * */
static void self_send_read_reg(uint8_t index ,uint8_t modbusID)
{
	uint16_t crc_ret = 0;
	uint8_t send_data_buff[8] = {0xFF ,0x03};
	printf("============当前读取寄存器：%d --数量：%d============\n",access_reg_arr[index].access_reg_addr,access_reg_arr[index].access_num);
	send_data_buff[2] = (access_reg_arr[index].access_reg_addr>>8)&0xFF;//起始地址寄存器高字节
	send_data_buff[3] =	(access_reg_arr[index].access_reg_addr&0xFF);//寄存器低字节
	send_data_buff[4] =	(access_reg_arr[index].access_num>>8)&0xFF;//读取数量高字节
	send_data_buff[5] =	(access_reg_arr[index].access_num&0xFF);
	crc_ret = CRC_Return(send_data_buff ,6);
	send_data_buff[6] =	(uint8_t)(crc_ret &0x00FF);//有无符号重要！
	send_data_buff[7] =	(uint8_t)((crc_ret>>8)&0x00FF);
	if(polling_msg[modbusID].fd != NULL)
	{
#if ENABLE_MODBUS_DEBUG
		printf("当前发送：");
		debug_print(send_data_buff,8);
#endif
		//发送本次需要轮寻的寄存器
		ModbusUsart_tx(polling_msg[modbusID].fd ,(uint8_t *)send_data_buff ,8);
		//封装本次轮寻状态
		polling_msg[modbusID].read_count = ((access_reg_arr[index].access_num*2)+5);
		polling_msg[modbusID].read_channel = modbusID;
		polling_msg[modbusID].read_reg = access_reg_arr[index].access_reg_addr;
#if ENABLE_MODBUS_DEBUG
		printf("当前需读取到%d字节，通道：%d，reg：%d\n",polling_msg[modbusID].read_count,polling_msg[modbusID].read_channel,polling_msg[modbusID].read_reg);
#endif
	}
}

static int8_t modbus_master_decode_mannul(void* data)
{
    polling_msg_t *pPolling_data = (polling_msg_t *) (data);
    CQ_handleTypeDef *cb = pPolling_data->cb;
	uint8_t msg_cmd ,device_channel;
	uint16_t msg_len = 0;
	uint16_t read_offset = 0;
	uint8_t tmp_buff[128];

	modbus_master_rec_t rec_data_temp;
	uint16_t time_delay = 0;
	uint8_t error_flag = 0;

	while(1)
	{
		//比较当前可读数据长度=当前写入长度-已读长度）大于7即可进入,符合modbus协议最小长度
		if (CQ_getLength(cb) >= 7)
		{
			if(uart_mode == REC_NOT_THROUGH_MODE)
			{
				//检测包是否完整，不完整则等待
				while(polling_msg[get_end_point(pPolling_data->fd)].read_count > CQ_getLength(cb))
				{
#if ENABLE_MODBUS_DEBUG
					printf("独立轮寻模式 -*-等待数据!\n");
#endif
					modbusDelay(100);
					error_flag++;
					if(error_flag > 3)break;
				}
			}
			read_offset = ((cb->exit + 1)%cb->size);
			msg_cmd = *(((uint8_t*)cb->dataBufer + read_offset)); //第二个字节为功能码
			//判断功能码
			switch(msg_cmd)
			{
			case 0x03:
				read_offset = ((cb->exit + 2)%cb->size);
				msg_len = (*((uint8_t*)cb->dataBufer + read_offset))+3;//计算字节：addr+cmd+len
				while((uint32_t)(msg_len+2) > CQ_getLength(cb))
				{
					printf("需要长度：%u当前长度：%u\n",msg_len+1,CQ_getLength(cb));
					modbusDelay(100);
					error_flag++;
					if(error_flag > 3)break;
				}
				CQ_get_buff_Data(cb ,tmp_buff ,msg_len+2);
				if(return_check_crc(tmp_buff ,msg_len))
				{

					if(uart_mode == REC_THROUGH_MODE)//透传模式－－CRC计算修改ID后发送给master
					{
						return_rw_message(pPolling_data->fd,tmp_buff,msg_len);
						CQ_read_offset_inc(cb, msg_len+2);//偏移一帧报文

					}
					if(uart_mode == REC_NOT_THROUGH_MODE)//本机解析模式
					{
						read_offset = ((cb->exit + 2)%cb->size);
						//数据字节长度
						uint8_t data_len = *(((uint8_t*)cb->dataBufer + read_offset));
						read_offset = ((cb->exit + 3)%cb->size);
						//数据起始地址
						uint8_t *reg_begin_data_H = ((uint8_t*)cb->dataBufer + read_offset);
						device_channel = get_end_point(pPolling_data->fd);
						rec_data_temp.channnel = device_channel;
						rec_data_temp.cmd = msg_cmd;
						rec_data_temp.data_addr = reg_begin_data_H;
						printf("进入解析寄存器：%d. . .\n",polling_msg[device_channel].read_reg);
						rec_slave_data(&rec_data_temp ,data_len);
						CQ_read_offset_inc(cb, msg_len+2);//偏移一帧报文

						return 0;
					}
//						printf("当前通道：%d ，可读：%u\n",device_channel,CQ_getLength(cb));
				}
				else
				{
					CQ_read_offset_inc(cb, 1);//丢弃１字节
					printf("master : modbus_0x03　return　CRC error!\n");
				}
				break;
			case 0x10:
				msg_len = 6;//不含CRC字节数
				CQ_get_buff_Data(cb, (uint8_t *)tmp_buff, msg_len+2);
				if(return_check_crc(tmp_buff ,msg_len))//校验正确
				{

					if(uart_mode == REC_THROUGH_MODE)//透传模式－－CRC计算修改ID后发送给master
					{
						return_rw_message(pPolling_data->fd,tmp_buff,msg_len);
						CQ_read_offset_inc(cb, msg_len+2);//偏移一帧报文

					}
					if(uart_mode == REC_NOT_THROUGH_MODE)//本机解析模式
					{
						CQ_read_offset_inc(cb, msg_len+2);//偏移一帧报文
						return 0;
					}
				}
				else
				{
					CQ_read_offset_inc(cb, 1);//丢弃１字节
					printf("master : modbus_0x10　return　CRC error!\n");
				}
				break;
			default:
				debug_print(tmp_buff,10);
				CQ_read_offset_inc(cb, 1);//什么都不是－>丢弃１字节
				printf("master : modbus_0x**　return　CRC error!\n");
			break;
			}
		}
		modbusDelay(100);
		time_delay++;
		if(time_delay > 2)//200ms out
		{
			return -1;//exit while
		}
	}
}
    
/*
 * 本机作为master解析从站数据 --存入数据库
 * 参数:rec_struct 包含所需cmd datapointer channel_num
 * 参数:len cmd 0x03时字节长度  cmd 0x10时ack消息
 * */
static void rec_slave_data(modbus_master_rec_t *rec_struct ,uint8_t len)
{
	uint16_t data_temp = 0;
	//数据地址
	uint8_t *data_addr = rec_struct->data_addr;
	//获取来自通道信息
	uint8_t channel_num = rec_struct->channnel;
	//数据对应起始寄存器地址
	uint16_t reg_addr = polling_msg[channel_num].read_reg;
	//解析从站返回的读操作数据
	if(rec_struct->cmd == 0x03)
	{
#if ENABLE_MODBUS_DEBUG
		printf("接收通道：%u 数据长度:%d\n",channel_num,len);
#endif
		for(uint16_t read_offset = 0; read_offset < len;read_offset += 2)
		{
			//准备数据
			data_temp = (*(data_addr+read_offset))&0xFF;
			data_temp <<= 8;
			data_temp |= (*(data_addr+read_offset+1))&0xFF;
			rec_struct->rec_data = data_temp;
			//存入数据
#if ENABLE_MODBUS_DEBUG
			printf("接收寄存器%u--数据:%04X---%u\n",reg_addr,data_temp,data_temp);
#endif
			//调用处理对应寄存器
			MasteReg_process_map[MasterCheck_Modbus_Addr(reg_addr)].func(rec_struct);
			//下一个寄存器
			reg_addr++;
		}
	}
	//解析从站返回的写操作ACK
	if(rec_struct->cmd == 0x10)
	{

	}
}

/* 透传 模式下
 * 给上位机返回读写数据 0x03 0x10 结果
 * */
static void return_rw_message(UART_HandleTypeDef *fd,uint8_t *ptr,uint16_t msglen)
{
	uint16_t crc_ret = 0;
#if USE_TCP_SEQUENS_NUM
	uint8_t send_master_buff[UART_SEND_BUFF_MAX];
#endif
	//修改数据
	ptr[0] = get_end_point(fd);//更换固定头部设备ID
	crc_ret = CRC_Return(ptr,msglen);
	ptr[msglen] =(uint8_t)(crc_ret &0x00FF);//有无符号重要！
	ptr[msglen+1] = (uint8_t)((crc_ret>>8)&0x00FF);
	//打印数据
	debug_print((uint8_t*)ptr,msglen+2);
	printf("master : 转换!\n");
#if USE_TCP_SEQUENS_NUM
	send_master_buff[0] = (msg_sequens_num>>8)&0xFF;
	send_master_buff[1] = msg_sequens_num&0xFF;
	memcpy(send_master_buff+2,ptr,msglen+2);
	debug_print(send_master_buff,msglen+2+2);
	//发送数据
	ModbusUsart_tx_Master(send_master_buff, msglen+2+2);//发送至本机上级
#else
	debug_print((uint8_t*)ptr,msglen+2);
	//发送数据
	ModbusUsart_tx_Master(ptr, msglen+2);//发送至本机上级
#endif
}


#ifdef __cplusplus //使用ｃ编译
}
#endif