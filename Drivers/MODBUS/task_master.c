/*
 *  task_master.c
 *
 *  Created on: 2020��2��17��
 *      Author: Aron566
 *
 *  ��ѯ�ӻ�--������������--���ô洢
 */

#ifdef __cplusplus //ʹ�ã����
extern "C" {
#endif
    
#include "modbus_type.h"/*����modbus��ѯ�ṹ��*/
#include "modbus_reg.h"
    
/*�Դ�վ��Ѱ�Ĵ�����ַ��Ϣ ����*/
modbus_polling_slave_t access_reg_arr[] =
{
	{
		.access_reg_addr = REAL_TIME_TEMP_INT,
		.access_num = 10 //�����ɼ�10���Ĵ���
	},
	{
		.access_reg_addr = REAL_TIME_TEMP_INT,
		.access_num = 1
	},
	{
		.access_reg_addr = REAL_TIME_TEMP_INT,
		.access_num = 6
	},
	{
		.access_reg_addr = REAL_TIME_TEMP_INT,
		.access_num = 2
	},
	{
		.access_reg_addr = 0,//END
		.access_num = 0
	}
};    

/*������Ϊmaster ��ѯ�ӻ�����*/
static void self_send_read_reg(uint8_t index ,uint8_t modbusID);

/*������Ϊmaster ����ӻ����ص�����*/
static int8_t modbus_master_decode_mannul(void* data);



/*
 * modbus ��������մ����߳�
 *
 * ������Ϊmaster
 *
 * ������͸��ģʽ,����������������,�ϱ�����
 *
 * �����ڶ�������ģʽ,���÷��ͺ�����������,
 *
 * ����:�������ɹ�����-1,�ɹ�Ϊ0
 * */
void *modbus_polling_task_loop(void* ptmr)
{
	circular_buffer *cb = (circular_buffer *) (ptmr);
	uint8_t time_out = 0;
	uint8_t modbusID = get_end_point(cb->fd);
	for(;;)
	{
		if(uart_mode == REC_NOT_THROUGH_MODE)
		{
			//ѭ����Ѱ��վ�Ĵ���
			for(int index = 0;access_reg_arr[index].access_num != 0;index++)
			{
				if(uart_mode != REC_NOT_THROUGH_MODE)break;
				self_send_read_reg(index ,modbusID);
				time_out = 0;
				//����
				do{
					if(uart_mode != REC_NOT_THROUGH_MODE)break;

					int8_t ret = modbus_master_decode_mannul(polling_msg[modbusID].cb);

					switch(ret)
					{
						case 0:
							//�ۼӳɹ�����
							device_v_info[modbusID].com_sucess_count++;
							break;
						case -1:
							time_out++;
							if(time_out > 1){
								//�ۼ�ʧ�ܴ���
								device_v_info[modbusID].com_faild_count++;
								printf("δ�д�--����!!!\n");
							}
							break;
						default:
							printf("what fuck��\n");
							break;
					}
				}while(time_out < 2 && time_out != 0);
				//����ɹ���
				if(!(device_v_info[modbusID].com_sucess_count+device_v_info[modbusID].com_faild_count)%2)
				{
					device_v_info[modbusID].device_com_rate_of_sucess = (100*(device_v_info[modbusID].com_sucess_count/(device_v_info[modbusID].com_sucess_count+device_v_info[modbusID].com_faild_count)))&0xFFFF;
					printf("ͨ��:%u ͨѶ�ɹ���:\%%u\n",modbusID,device_v_info[modbusID].device_com_rate_of_sucess);
				}
				cbClear(polling_msg[modbusID].cb);
			}
		}
		else if(uart_mode == REC_THROUGH_MODE)
		{
			modbus_master_decode_mannul(polling_msg[modbusID].cb);
		}
		//��������ȴ�����
		modbusDelay(10);
	}
	return NULL;
}

/*
 * ������Ϊmaster ,����������Ѱ�Ĵ���
 *
 * ���� index ��Ѱ�Ĵ������� fd_countd fd��Ӧ����
 * */
static void self_send_read_reg(uint8_t index ,uint8_t modbusID)
{
	uint8_t CRC_value_L ,CRC_value_H;
	uint16_t crc_ret = 0;
	uint8_t send_data_buff[8] = {0xFF ,0x03};
	printf("============��ǰ��ȡ�Ĵ�����%d --������%d============\n",access_reg_arr[index].access_reg_addr,access_reg_arr[index].access_num);
	send_data_buff[2] = (access_reg_arr[index].access_reg_addr>>8)&0xFF;//��ʼ��ַ�Ĵ������ֽ�
	send_data_buff[3] =	(access_reg_arr[index].access_reg_addr&0xFF);//�Ĵ������ֽ�
	send_data_buff[4] =	(access_reg_arr[index].access_num>>8)&0xFF;//��ȡ�������ֽ�
	send_data_buff[5] =	(access_reg_arr[index].access_num&0xFF);
	crc_ret = CRC_Return(send_data_buff ,6);
	send_data_buff[6] =	CRC_value_L =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
	send_data_buff[7] =	CRC_value_H = (uint8_t)((crc_ret>>8)&0x00FF);
	if(polling_msg[modbusID].cb->fd != 0 && polling_msg[modbusID].cb->fd != -1)
	{
#if ENABLE_MODBUS_DEBUG
		printf("��ǰ���ͣ�");
		debug_print(send_data_buff,8);
#endif
		//���ͱ�����Ҫ��Ѱ�ļĴ���
		usart_tx(polling_msg[modbusID].cb->fd ,(uint8_t *)send_data_buff ,8);
		//��װ������Ѱ״̬
		polling_msg[modbusID].read_count = ((access_reg_arr[index].access_num*2)+5);
		polling_msg[modbusID].read_channel = modbusID;
		polling_msg[modbusID].read_reg = access_reg_arr[index].access_reg_addr;
#if ENABLE_MODBUS_DEBUG
		printf("��ǰ���ȡ��%d�ֽڣ�ͨ����%d��reg��%d\n",polling_msg[modbusID].read_count,polling_msg[modbusID].read_channel,polling_msg[modbusID].read_reg);
#endif
	}
}

static int8_t modbus_master_decode_mannul(void* data)
{
	circular_buffer *cb = (circular_buffer *) (data);
	uint8_t msg_cmd ,device_channel;
	uint16_t msg_len = 0;
	uint16_t read_offset = 0;
	uint8_t tmp_buff[128];
	//��������ģʽ����������
	modbus_master_rec_t rec_data_temp;
	uint16_t time_delay = 0;
	uint8_t error_flag = 0;

	while(1)
	{
		//�Ƚϵ�ǰ�ɶ����ݳ���=��ǰд�볤��-�Ѷ����ȣ�����7���ɽ���,����modbusЭ����С����
		if (cb_bytes_can_read(cb) >= 7)
		{
			if(uart_mode == REC_NOT_THROUGH_MODE)
			{
				//�����Ƿ���������������ȴ�
				while(polling_msg[get_end_point(cb->fd)].read_count > cb_bytes_can_read(cb))
				{
#if ENABLE_MODBUS_DEBUG
					printf("������Ѱģʽ -*-�ȴ�����!\n");
#endif
					modbusDelay(100);
					error_flag++;
					if(error_flag > 3)break;
				}
			}
			read_offset = ((cb->read_offset + 1)%cb->count);
			msg_cmd = *(((uint8_t*)cb->ptr + read_offset)); //�ڶ����ֽ�Ϊ������
			//�жϹ�����
			switch(msg_cmd)
			{
			case 0x03:
				read_offset = ((cb->read_offset + 2)%cb->count);
				msg_len = (*((uint8_t*)cb->ptr + read_offset))+3;//�����ֽڣ�addr+cmd+len
				while((uint32_t)(msg_len+2) > cb_bytes_can_read(cb))
				{
					printf("��Ҫ���ȣ�%u��ǰ���ȣ�%u\n",msg_len+1,cb_bytes_can_read(cb));
					modbusDelay(100);
					error_flag++;
					if(error_flag > 3)break;
				}
				CQ_get_buff_Data(cb ,tmp_buff ,msg_len+2);
				if(return_check_crc(tmp_buff ,msg_len))
				{

					if(uart_mode == REC_THROUGH_MODE)//͸��ģʽ����CRC�����޸�ID���͸�master
					{
						return_rw_message(cb->fd,tmp_buff,msg_len);
						CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����

					}
					if(uart_mode == REC_NOT_THROUGH_MODE)//��������ģʽ
					{
						read_offset = ((cb->read_offset + 2)%cb->count);
						//�����ֽڳ���
						uint8_t data_len = *(((uint8_t*)cb->ptr + read_offset));
						read_offset = ((cb->read_offset + 3)%cb->count);
						//������ʼ��ַ
						uint8_t *reg_begin_data_H = ((uint8_t*)cb->ptr + read_offset);
						device_channel = get_end_point(cb->fd);
						rec_data_temp.channnel = device_channel;
						rec_data_temp.cmd = msg_cmd;
						rec_data_temp.data_addr = reg_begin_data_H;
						printf("��������Ĵ�����%d. . .\n",polling_msg[device_channel].read_reg);
						rec_slave_data(&rec_data_temp ,data_len);
						CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����

						return 0;
					}
//						printf("��ǰͨ����%d ���ɶ���%u\n",device_channel,cb_bytes_can_read(cb));
				}
				else
				{
					CQ_read_offset_inc(cb, 1);//�������ֽ�
					printf("master : modbus_0x03��return��CRC error!\n");
				}
				break;
			case 0x10:
				msg_len = 6;//����CRC�ֽ���
				CQ_get_buff_Data(cb, (uint8_t *)tmp_buff, msg_len+2);
				if(return_check_crc(tmp_buff ,msg_len))//У����ȷ
				{

					if(uart_mode == REC_THROUGH_MODE)//͸��ģʽ����CRC�����޸�ID���͸�master
					{
						return_rw_message(cb->fd,tmp_buff,msg_len);
						CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����

					}
					if(uart_mode == REC_NOT_THROUGH_MODE)//��������ģʽ
					{
						CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����
						return 0;
					}
				}
				else
				{
					CQ_read_offset_inc(cb, 1);//�������ֽ�
					printf("master : modbus_0x10��return��CRC error!\n");
				}
				break;
			default:
				debug_print(tmp_buff,10);
				CQ_read_offset_inc(cb, 1);//ʲô�����ǣ�>�������ֽ�
				printf("master : modbus_0x**��return��CRC error!\n");
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
    
    
#ifdef __cplusplus //ʹ�ã����
}
#endif