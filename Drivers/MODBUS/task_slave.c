/*
 *  task_master.c
 *
 *  Created on: 2020��2��17��
 *      Author: Aron566
 *
 *
 */

#ifdef __cplusplus //ʹ�ã����
extern "C" {
#endif
    
#include "modbus_type.h"
#include "CircularQueue.h"/*����modbusͨѶ֡�洢*/    

static void broker_send_message_to_slave(uint8_t msg_device_addr ,uint8_t* msg ,int len);


/* modbus�����LOOP
 *
 * ����master����������������Ϊslave��
 *
 * uart_mode:����������ģʽ,����master����,��������ģʽ,�������ݣ���͸��ģʽ��ת��master�����¼�slave
 *
 * ������Uart��1��_Send_slave_BUFF
 * */
void* modbus_slave_decode_start(void* data) {
	circular_buffer *cb = (circular_buffer *) (data);
	uint8_t msg_device_addr ,msg_cmd ,data_len;
	uint16_t msg_len = 0;
	uint32_t read_offset = 0;
	uint8_t tmp_buff[128];
	//��������ģʽ
	uint16_t reg_start_addr = 0;
	uint16_t reg_len = 0;
	modbus_master_rec_t rec_data_temp;
	while (1) {
#if USE_TCP_SEQUENS_NUM
		if(cb_bytes_can_read(cb) >= 9)
		{
			//�������к�
			msg_sequens_num = *(((uint8_t*)cb->ptr + (cb->read_offset%cb->count)));
			read_offset = ((cb->read_offset + 1)%cb->count);
			msg_sequens_num <<= 8;
			msg_sequens_num |= *(((uint8_t*)cb->ptr + read_offset));
			//������ַ
			read_offset = ((cb->read_offset + 2)%cb->count);
			msg_device_addr = *(((uint8_t*)cb->ptr + read_offset)); //��3���ֽ�Ϊ�豸��ַ
			//��������
			read_offset = ((cb->read_offset + 3)%cb->count);
			msg_cmd = *(((uint8_t*)cb->ptr + read_offset)); //��4���ֽ�Ϊ������
			rec_data_temp.cmd = msg_cmd;

			switch(msg_cmd)
			{
				case 0x03:
					msg_len = 6;//����CRC�ֽ���
					//�ж����ݳ��� + ��Ϣ
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2+2))
					{
						CQ_get_buff_Data(cb, tmp_buff, msg_len+2+2);
						if(return_check_crc(tmp_buff+2 ,msg_len))
						{
							/*��ȡ������ʼ��ַ*/
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
								//͸��ģʽ���������͸�slave
								debug_print(tmp_buff,msg_len+2+2);
								broker_rw_message(msg_device_addr,tmp_buff+2,msg_len);
							}
							if(uart_mode == REC_NOT_THROUGH_MODE)
							{
								//��������ģʽ
								_read_self_par:
								reg_start_addr = tmp_buff[2+2];
								reg_start_addr <<= 8;
								reg_start_addr |= tmp_buff[3+2];
								reg_len = tmp_buff[4+2];
								reg_len <<= 8;
								reg_len |= tmp_buff[5+2];
								rec_data_temp.channnel = msg_device_addr;
								rec_data_temp.cmd = msg_cmd;
								printf("��ȡ�Ĵ���:%u-%u��\n",reg_start_addr,reg_len);
								printf("���ն�ȡ����:");
								debug_print(tmp_buff,msg_len+2+2);
								rec_master_data(&rec_data_temp ,reg_start_addr ,reg_len);
							}
							CQ_read_offset_inc(cb, msg_len+2+2);//ƫ��һ֡����
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//�������ֽ�
							printf("slave : modbus_0x03��return��CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
					}
					break;
				case 0x13:
					//�������ȡ�ļĴ�����Ŀ--������
					read_offset = ((cb->read_offset + 4)%cb->count);
					reg_len = (*((uint8_t*)cb->ptr + read_offset));
					//����CRCУ�����ݳ���
					msg_len = 3+(reg_len*2)+3;
					//�ж����ݳ���
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2+2))
					{
						CQ_get_buff_Data(cb ,tmp_buff ,msg_len+2+2);
						//CRCУ��
						if(return_check_crc(tmp_buff+2 ,msg_len))
						{
							rec_data_temp.channnel = tmp_buff[msg_len-3+2];
							rec_data_temp.channnel <<= 8;
							rec_data_temp.channnel |= tmp_buff[msg_len-2+2];
							rec_data_temp.channel_count = tmp_buff[msg_len-1+2];
							rec_data_temp.buff_addr = tmp_buff;
							rec_master_data(&rec_data_temp ,0 ,reg_len);
							CQ_read_offset_inc(cb, msg_len+2+2);//ƫ��һ֡����
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//�������ֽ�
							printf("slave : modbus_0x13��return��CRC error!\n");
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
					//�����ֽڣ�����Ԫ�������ֽ�����ǰ�棷���ֽ���
					msg_len = data_len+7;

					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2+2))
					{
						CQ_get_buff_Data(cb, tmp_buff, msg_len+2+2);
						if(return_check_crc(tmp_buff+2 ,msg_len))//У����ȷ
						{
							/*��ȡ������ʼ��ַ*/
							reg_start_addr = tmp_buff[2+2];
							reg_start_addr <<= 8;
							reg_start_addr |= tmp_buff[3+2];
							if(reg_start_addr >= SET_UART_MODE && (reg_start_addr+data_len/2) < MODBUS_PROTOL)
							{
								goto _set_self_par;
							}
							if(uart_mode == REC_THROUGH_MODE)//͸��ģʽ�����޸�ID CRC������͸�master
							{
								broker_rw_message(msg_device_addr,tmp_buff+2,msg_len);
							}
							if(uart_mode == REC_NOT_THROUGH_MODE)//��������ģʽ
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
								printf("��������:0x%02X-0x%02X\n",tmp_buff[7+2],tmp_buff[8+2]);
								printf("���տ�������:");
								debug_print(tmp_buff,msg_len+2+2);
								rec_master_data(&rec_data_temp ,reg_start_addr ,reg_len);
								CQ_read_offset_inc(cb, msg_len+2+2);//ƫ��һ֡����
//								pthread_mutex_unlock(&GNNC_REC_Data_mutex_lock);
							}
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//�������ֽ�
							printf("slave : modbus_0x10��return��CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
					}
					break;
				default:
					debug_print((uint8_t*)((uint8_t*)cb->ptr + cb->read_offset),8+2);
					CQ_read_offset_inc(cb, 1);//ʲô�����ǣ�>�������ֽ�
					printf("slave : modbus_0x**��return��CRC error!\n");
					break;
			}
		}
		modbusDelay(100);
#else
		//�Ƚϵ�ǰ�ɶ����ݳ���=��ǰд�볤��-�Ѷ����ȣ�����7���ɽ���,����modbusЭ����С����
		if(cb_bytes_can_read(cb) >= 7)
		{
			msg_device_addr = *(((uint8_t*)cb->ptr + (cb->read_offset%cb->count))); //��һ���ֽ�Ϊ�豸��ַ
			read_offset = ((cb->read_offset + 1)%cb->count);
			msg_cmd = *(((uint8_t*)cb->ptr + read_offset)); //�ڶ����ֽ�Ϊ������
			rec_data_temp.cmd = msg_cmd;
			switch(msg_cmd)
			{
				case 0x03:
					msg_len = 6;//����CRC�ֽ���
					//�ж����ݳ���
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2))
					{
						CQ_get_buff_Data(cb, tmp_buff, msg_len+2);
						if(return_check_crc(tmp_buff ,msg_len))
						{
							/*��ȡ������ʼ��ַ*/
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
								//͸��ģʽ���������͸�slave
								broker_rw_message(msg_device_addr,tmp_buff,msg_len);
							}
							if(uart_mode == REC_NOT_THROUGH_MODE)
							{
								//��������ģʽ
								_read_self_par:
								reg_start_addr = tmp_buff[2];
								reg_start_addr <<= 8;
								reg_start_addr |= tmp_buff[3];
								reg_len = tmp_buff[4];
								reg_len <<= 8;
								reg_len |= tmp_buff[5];
								rec_data_temp.channnel = msg_device_addr;
								rec_data_temp.cmd = msg_cmd;
								printf("��ȡ�Ĵ���:%u-%u��\n",reg_start_addr,reg_len);
								printf("���ն�ȡ����:");
								debug_print(tmp_buff,msg_len+2);
								rec_master_data(&rec_data_temp ,reg_start_addr ,reg_len);
							}
							CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//�������ֽ�
							printf("slave : modbus_0x03��return��CRC error!\n");
						}
					}
					else
					{
						modbusDelay(1);
						error_flag++;
					}
					break;
				case 0x13:
					//�������ȡ�ļĴ�����Ŀ--������
					read_offset = ((cb->read_offset + 2)%cb->count);
					reg_len = (*((uint8_t*)cb->ptr + read_offset));
					//CRCУ��
					msg_len = 3+(reg_len*2)+3;
					//�ж����ݳ���
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2))
					{
						CQ_get_buff_Data(cb, (uint8_t *)tmp_buff, msg_len+2);
						if(return_check_crc(tmp_buff ,msg_len))//У����ȷ
						{
							rec_data_temp.channnel = tmp_buff[msg_len-3];
							rec_data_temp.channnel <<= 8;
							rec_data_temp.channnel |= tmp_buff[msg_len-2];
							rec_data_temp.channel_count = tmp_buff[msg_len-1];
							rec_data_temp.buff_addr = tmp_buff;
							rec_master_data(&rec_data_temp ,0 ,reg_len);
							CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//�������ֽ�
							printf("slave : modbus_0x13��return��CRC error!\n");
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
					//�����ֽڣ�����Ԫ�������ֽ�����ǰ�棷���ֽ���
					msg_len = data_len+7;
					if(cb_bytes_can_read(cb) >= (uint32_t)(msg_len+2))
					{
						CQ_get_buff_Data(cb, (uint8_t *)tmp_buff, msg_len+2);
						if(return_check_crc(tmp_buff ,msg_len))//У����ȷ
						{
							/*��ȡ������ʼ��ַ*/
							reg_start_addr = tmp_buff[2];
							reg_start_addr <<= 8;
							reg_start_addr |= tmp_buff[3];
							if(reg_start_addr >= SET_UART_MODE && (reg_start_addr+data_len/2) < MODBUS_PROTOL)
							{
								goto _set_self_par;
							}
							if(uart_mode == REC_THROUGH_MODE)//͸��ģʽ�����޸�ID CRC������͸�master
							{
								broker_rw_message(msg_device_addr,tmp_buff,msg_len);
							}
							if(uart_mode == REC_NOT_THROUGH_MODE)//��������ģʽ
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
								printf("��������:0x%02X-0x%02X\n",tmp_buff[7],tmp_buff[8]);
								printf("���տ�������:");
								debug_print(tmp_buff,msg_len+2);
								rec_master_data(&rec_data_temp ,reg_start_addr ,reg_len);
								CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����
							}
						}
						else
						{
							CQ_read_offset_inc(cb, 1);//�������ֽ�
							printf("slave : modbus_0x10��return��CRC error!\n");
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
					CQ_read_offset_inc(cb, 1);//ʲô�����ǣ�>�������ֽ�
					printf("slave : modbus_0x**��return��CRC error!\n");
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
 * ������Ϊslave����������վ���ݰ� --��Ӧ ���ƺͶ�ȡ����
 * ������rec_struct ��Ҫ�� д�����ݵ���ʼ��ַ���루��Ϊд����� | �������� | ͨ���ţ�����Ҫ��ȡָ��ͨ��ʱ��
 * ����:reg_begin_addr ��ʼ�Ĵ�����ַ
 * ����:len �Ĵ�����������
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
	//׼�������ͷ����Ϣ
	ack_data_buff[0] = (msg_sequens_num>>8)&0xFF;
	ack_data_buff[1] = msg_sequens_num&0xFF;
	ack_data_buff[0+2] = rec_struct->channnel&0xFF;
	ack_data_buff[1+2] = rec_struct->cmd;
	//д������
	if(ack_data_buff[1+2] == 0x10)
	{
		if(data_addr == NULL || len == 0)
		{
			printf("��������!\n");
			return;
		}
#if ENABLE_MODBUS_DEBUG
		printf("ͨ��:%02X-����:%02X-����:%02X-����:%02X\n",rec_struct->channnel,rec_struct->cmd,*rec_struct->data_addr,*(rec_struct->data_addr+1));
		printf("===����Ĵ���ֵ:%d,����:%d===\n",reg_addr,len);
#endif
		for(;reg_addr < reg_end ;reg_addr++)
		{
			rec_struct->rec_data = *(data_addr+read_offset)&0xFF;
			rec_struct->rec_data <<= 8;
			rec_struct->rec_data |= (*(data_addr+read_offset+1))&0xFF;
#if ENABLE_MODBUS_DEBUG
			printf("==д��Ĵ���ֵ:%04X,�Ĵ���:%d==\n",rec_struct->rec_data,reg_addr);
#endif
			reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
			read_offset += 2;
		}
		//�Ĵ�����ַ
		ack_data_buff[2+2] = ((reg_begin_addr>>8)&0xFF);
		ack_data_buff[3+2] = (reg_begin_addr&0xFF);
		//�Ĵ�������
		uint16_t reg_num = len;
		ack_data_buff[4+2] = ((reg_num>>8)&0xFF);
		ack_data_buff[5+2] = reg_num&0xFF;
		//У��CRC
		crc_ret = CRC_Return(ack_data_buff+2,6);
		ack_data_buff[6+2] =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
		ack_data_buff[7+2] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK��������λ��
		tcp_client_tx(sockfd, ack_data_buff, 8+2);//�����������ϼ�
	}
	//��ȡ����
	if(ack_data_buff[1+2] == 0x03)
	{
		if(len == 0)
		{
			printf("��������,����Ϊ0!\n");
			return;
		}
		uint16_t ret_data = 0;
		//�Ĵ������ݴӵ�4+2���ֽڿ�ʼ��� ����Ϊ3+2
		read_offset = 3+2;
		//�ֽ���
		ack_data_buff[2+2] = len*2;

		//��ȡ�Ĵ������� ���
		for(reg_addr = reg_begin_addr;reg_addr < reg_end ;reg_addr++)
		{
			ret_data = reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
//			printf("��ȡ�Ĵ���ֵ:%d,ֵ:%d\n",reg_addr,ret_data);
			ack_data_buff[read_offset++] = ((ret_data>>8)&0xFF);
			ack_data_buff[read_offset++] = (ret_data&0xFF);
		}

		//У��CRC
		crc_ret = CRC_Return(ack_data_buff+2,read_offset-2);
		ack_data_buff[read_offset++] =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
		ack_data_buff[read_offset++] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK��������λ��
		tcp_client_tx(sockfd, ack_data_buff ,read_offset);//�����������ϼ�
	}
	if(ack_data_buff[1+2] == 0x13)
	{
		if(len == 0)
		{
			printf("��������,����Ϊ0!\n");
			return;
		}
		//�ع�ͷ��
		ack_data_buff[0+2] = rec_struct->buff_addr[2];
		//�ظ��ֽ��� = �Ĵ�������*ͨ������
		ack_data_buff[2+2] = (len*2*rec_struct->channel_count)&0xFF;
		read_offset = 3+2;
		uint16_t ret_data = 0;
		uint16_t reg_addr = 0;

		//����ѯͨ�� ���
		uint16_t channel_end = rec_struct->channnel+rec_struct->channel_count;
		for(;rec_struct->channnel < channel_end;rec_struct->channnel++)
		{
			//��ѯlen���Ĵ��� ������� �ṩ�׸��Ĵ���������
			uint8_t reg_addr_index = 3+2;
			for(uint8_t i = 3;i < (len+3);i++)
			{
				reg_addr = rec_struct->buff_addr[reg_addr_index];
				reg_addr <<= 8;
				reg_addr |=  rec_struct->buff_addr[reg_addr_index+1];
				ret_data = reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
#if ENABLE_MODBUS_DEBUG
				printf("��ȡͨ��%d--�Ĵ���:%d,ֵ:%04X\n",rec_struct->channnel,reg_addr,ret_data);
#endif
				ack_data_buff[read_offset++] = ((ret_data>>8)&0xFF);
				ack_data_buff[read_offset++] = (ret_data&0xFF);
				//��ѯ��һ���Ĵ���
				reg_addr_index += 2;
			}
		}
		//У��CRC
		crc_ret = CRC_Return(ack_data_buff+2,read_offset-2);
		ack_data_buff[read_offset++] =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
		ack_data_buff[read_offset++] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK��������λ��
		tcp_client_tx(sockfd, ack_data_buff ,read_offset);//�����������ϼ�

	}
#else
	//׼��ͨ��ͷ��Ӧ����Ϣ
	ack_data_buff[0] = rec_struct->channnel&0xFF;
	ack_data_buff[1] = rec_struct->cmd;
	//д������
	if(ack_data_buff[1] == 0x10)
	{
		if(data_addr == NULL || len == 0)
		{
			return;
		}
#if ENABLE_MODBUS_DEBUG
		printf("ͨ��:%02X-����:%02X-����:%02X-����:%02X\n",rec_struct->channnel,rec_struct->cmd,*rec_struct->data_addr,*(rec_struct->data_addr+1));
		printf("===����Ĵ���ֵ:%d,����:%d===\n",reg_addr,len);
#endif
		for(;reg_addr < reg_end ;reg_addr++)
		{
			rec_struct->rec_data = *(data_addr+read_offset)&0xFF;
			rec_struct->rec_data <<= 8;
			rec_struct->rec_data |= (*(data_addr+read_offset+1))&0xFF;
			printf("==д��Ĵ���ֵ:%04X,�Ĵ���:%d==\n",rec_struct->rec_data,reg_addr);
			reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
			read_offset += 2;
		}
		//�Ĵ�����ַ
		ack_data_buff[2] = ((reg_begin_addr>>8)&0xFF);
		ack_data_buff[3] = (reg_begin_addr&0xFF);
		//�Ĵ�������
		uint16_t reg_num = len;
		ack_data_buff[4] = ((reg_num>>8)&0xFF);
		ack_data_buff[5] = reg_num&0xFF;
		//У��CRC
		crc_ret = CRC_Return(ack_data_buff,6);
		ack_data_buff[6] =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
		ack_data_buff[7] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK��������λ��
		tcp_client_tx(sockfd, ack_data_buff, 8);//�����������ϼ�
	}
	//��ȡ����
	if(ack_data_buff[1] == 0x03)
	{
		if(len == 0)
		{
			return;
		}
		uint16_t ret_data = 0;
		//�Ĵ������ݴӵ�4���ֽڿ�ʼ��� ����Ϊ3
		read_offset = 3;
		//�ֽ���
		ack_data_buff[2] = len*2;

		//��ȡ�Ĵ������� ���
		for(reg_addr = reg_begin_addr;reg_addr < reg_end ;reg_addr++)
		{
			ret_data = reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
			printf("��ȡ�Ĵ���ֵ:%d,ֵ:%d\n",reg_addr,ret_data);
			ack_data_buff[read_offset++] = ((ret_data>>8)&0xFF);
			ack_data_buff[read_offset++] = (ret_data&0xFF);
		}

		//У��CRC
		crc_ret = CRC_Return(ack_data_buff,read_offset);
		ack_data_buff[read_offset++] =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
		ack_data_buff[read_offset++] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK��������λ��
		tcp_client_tx(sockfd, ack_data_buff ,read_offset);//�����������ϼ�
	}
	if(ack_data_buff[1] == 0x13)
	{
		if(len == 0)
		{
			return;
		}
		//�ع�ͷ��
		ack_data_buff[0] = rec_struct->buff_addr[0];
		//�ظ��ֽ��� = �Ĵ�������*ͨ������
		ack_data_buff[2] = (len*2*rec_struct->channel_count)&0xFF;
		read_offset = 3;
		uint16_t ret_data = 0;
		uint16_t reg_addr = 0;

		//����ѯͨ�� ���
		uint16_t channel_end = rec_struct->channnel+rec_struct->channel_count;
		for(;rec_struct->channnel < channel_end;rec_struct->channnel++)
		{
			//��ѯlen���Ĵ��� ������� �ṩ�׸��Ĵ���������
			uint8_t reg_addr_index = 3;
			for(uint8_t i = 3;i < (len+3);i++)
			{
				reg_addr = rec_struct->buff_addr[reg_addr_index];
				reg_addr <<= 8;
				reg_addr |=  rec_struct->buff_addr[reg_addr_index+1];
				ret_data = reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
#if ENABLE_MODBUS_DEBUG
				printf("��ȡͨ��%d--�Ĵ���:%d,ֵ:%04X\n",rec_struct->channnel,reg_addr,ret_data);
#endif
				ack_data_buff[read_offset++] = ((ret_data>>8)&0xFF);
				ack_data_buff[read_offset++] = (ret_data&0xFF);
				//��ѯ��һ���Ĵ���
				reg_addr_index += 2;
			}
		}
		//У��CRC
		crc_ret = CRC_Return(ack_data_buff,read_offset);
		ack_data_buff[read_offset++] =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
		ack_data_buff[read_offset++] = (uint8_t)((crc_ret>>8)&0x00FF);
		//ACK��������λ��
		tcp_client_tx(sockfd, ack_data_buff ,read_offset);//�����������ϼ�

	}
#endif
}    

/* ͸�� ģʽ��
 * ת����д���� 0x03 0x10
 * */
static void broker_rw_message(int channel,uint8_t *ptr,uint16_t msglen)
{
	uint16_t crc_ret = 0;
	uint8_t Uart_Send_slave_BUFF[UART_SEND_BUFF_MAX];
	//��������
	memcpy(Uart_Send_slave_BUFF,(uint8_t*)ptr,msglen);
	//�޸�����
	Uart_Send_slave_BUFF[0] = 0XFF;//�����̶�ͷ���豸ID
	crc_ret = CRC_Return(Uart_Send_slave_BUFF,msglen);
	Uart_Send_slave_BUFF[msglen] =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
	Uart_Send_slave_BUFF[msglen+1] = (uint8_t)((crc_ret>>8)&0x00FF);
	//��ӡ����
	debug_print((uint8_t*)ptr,msglen+2);
	printf("slave : ת��!\n");
	debug_print((uint8_t*)Uart_Send_slave_BUFF,msglen+2);
	//��������
	broker_send_message_to_slave(channel,Uart_Send_slave_BUFF,msglen+2);
}

/*
 * ����master����ת������ȷ�˿�slave
 *
 * msg_device_addr����ͷ��
 *
 * msg��Ϣ�壬len���ݳ���
 * */
static void broker_send_message_to_slave(uint8_t msg_device_addr ,uint8_t* msg ,int len)
{
	if(msg_device_addr >= 0x01 && msg_device_addr <= UART_NUM_MAX)
	{
		usart_tx(polling_msg[msg_device_addr].cb->fd ,msg ,len);
	}
}


#ifdef __cplusplus //ʹ�ã����
}
#endif