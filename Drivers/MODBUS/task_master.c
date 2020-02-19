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
#include "Utilities.h"/*CRC �� ��ȡ�˿ں�*/
    
/*�Դ�վ��Ѱ�Ĵ�����ַ��Ϣ ����*/
modbus_polling_slave_t access_reg_arr[] =
{
	{
		.access_reg_addr = REAL_TIME_TEMP,
		.access_num = 1 //�����ɼ�1���Ĵ���
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

/*������Ϊmaster ��ѯ�ӻ�����*/
static void self_send_read_reg(uint8_t index ,uint8_t modbusID);

/*������Ϊmaster ����ӻ����ص�����*/
static int8_t modbus_master_decode_mannul(void* data);

/*������Ϊmaster �������Դ�վ������---�洢*/
static void rec_slave_data(modbus_master_rec_t *rec_struct ,uint8_t len);

/*ת����ȡ��������*/
static void return_rw_message(UART_HandleTypeDef *fd,uint8_t *ptr,uint16_t msglen);

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
	polling_msg_t *pPolling_data = (polling_msg_t *) (ptmr);
	uint8_t time_out = 0;
	uint8_t modbusID = get_end_point(pPolling_data->fd);
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

					int8_t ret = modbus_master_decode_mannul(pPolling_data);

					switch(ret)
					{
						case 0:
							//�ۼӳɹ�����
//							device_v_info[modbusID].com_sucess_count++;
							break;
						case -1:
							time_out++;
							if(time_out > 1){
								//�ۼ�ʧ�ܴ���
//								device_v_info[modbusID].com_faild_count++;
								printf("δ�д�--����!!!\n");
							}
							break;
						default:
							printf("what fuck��\n");
							break;
					}
				}while(time_out < 2 && time_out != 0);
				//����ɹ���
//				if(!(device_v_info[modbusID].com_sucess_count+device_v_info[modbusID].com_faild_count)%2)
//				{
//					device_v_info[modbusID].device_com_rate_of_sucess = (100*(device_v_info[modbusID].com_sucess_count/(device_v_info[modbusID].com_sucess_count+device_v_info[modbusID].com_faild_count)))&0xFFFF;
//					printf("ͨ��:%u ͨѶ�ɹ���:\%%u\n",modbusID,device_v_info[modbusID].device_com_rate_of_sucess);
//				}
				CQ_emptyData(pPolling_data->cb);
			}
		}
		else if(uart_mode == REC_THROUGH_MODE)
		{
			modbus_master_decode_mannul(pPolling_data);
		}
		//��������ȴ�����
		modbusDelay(10);
	}
}

/*
 * ������Ϊmaster ,����������Ѱ�Ĵ���
 *
 * ���� index ��Ѱ�Ĵ������� fd_countd fd��Ӧ����
 * */
static void self_send_read_reg(uint8_t index ,uint8_t modbusID)
{
	uint16_t crc_ret = 0;
	uint8_t send_data_buff[8] = {0xFF ,0x03};
	printf("============��ǰ��ȡ�Ĵ�����%d --������%d============\n",access_reg_arr[index].access_reg_addr,access_reg_arr[index].access_num);
	send_data_buff[2] = (access_reg_arr[index].access_reg_addr>>8)&0xFF;//��ʼ��ַ�Ĵ������ֽ�
	send_data_buff[3] =	(access_reg_arr[index].access_reg_addr&0xFF);//�Ĵ������ֽ�
	send_data_buff[4] =	(access_reg_arr[index].access_num>>8)&0xFF;//��ȡ�������ֽ�
	send_data_buff[5] =	(access_reg_arr[index].access_num&0xFF);
	crc_ret = CRC_Return(send_data_buff ,6);
	send_data_buff[6] =	(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
	send_data_buff[7] =	(uint8_t)((crc_ret>>8)&0x00FF);
	if(polling_msg[modbusID].fd != NULL)
	{
#if ENABLE_MODBUS_DEBUG
		printf("��ǰ���ͣ�");
		debug_print(send_data_buff,8);
#endif
		//���ͱ�����Ҫ��Ѱ�ļĴ���
		ModbusUsart_tx(polling_msg[modbusID].fd ,(uint8_t *)send_data_buff ,8);
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
		//�Ƚϵ�ǰ�ɶ����ݳ���=��ǰд�볤��-�Ѷ����ȣ�����7���ɽ���,����modbusЭ����С����
		if (CQ_getLength(cb) >= 7)
		{
			if(uart_mode == REC_NOT_THROUGH_MODE)
			{
				//�����Ƿ���������������ȴ�
				while(polling_msg[get_end_point(pPolling_data->fd)].read_count > CQ_getLength(cb))
				{
#if ENABLE_MODBUS_DEBUG
					printf("������Ѱģʽ -*-�ȴ�����!\n");
#endif
					modbusDelay(100);
					error_flag++;
					if(error_flag > 3)break;
				}
			}
			read_offset = ((cb->exit + 1)%cb->size);
			msg_cmd = *(((uint8_t*)cb->dataBufer + read_offset)); //�ڶ����ֽ�Ϊ������
			//�жϹ�����
			switch(msg_cmd)
			{
			case 0x03:
				read_offset = ((cb->exit + 2)%cb->size);
				msg_len = (*((uint8_t*)cb->dataBufer + read_offset))+3;//�����ֽڣ�addr+cmd+len
				while((uint32_t)(msg_len+2) > CQ_getLength(cb))
				{
					printf("��Ҫ���ȣ�%u��ǰ���ȣ�%u\n",msg_len+1,CQ_getLength(cb));
					modbusDelay(100);
					error_flag++;
					if(error_flag > 3)break;
				}
				CQ_get_buff_Data(cb ,tmp_buff ,msg_len+2);
				if(return_check_crc(tmp_buff ,msg_len))
				{

					if(uart_mode == REC_THROUGH_MODE)//͸��ģʽ����CRC�����޸�ID���͸�master
					{
						return_rw_message(pPolling_data->fd,tmp_buff,msg_len);
						CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����

					}
					if(uart_mode == REC_NOT_THROUGH_MODE)//��������ģʽ
					{
						read_offset = ((cb->exit + 2)%cb->size);
						//�����ֽڳ���
						uint8_t data_len = *(((uint8_t*)cb->dataBufer + read_offset));
						read_offset = ((cb->exit + 3)%cb->size);
						//������ʼ��ַ
						uint8_t *reg_begin_data_H = ((uint8_t*)cb->dataBufer + read_offset);
						device_channel = get_end_point(pPolling_data->fd);
						rec_data_temp.channnel = device_channel;
						rec_data_temp.cmd = msg_cmd;
						rec_data_temp.data_addr = reg_begin_data_H;
						printf("��������Ĵ�����%d. . .\n",polling_msg[device_channel].read_reg);
						rec_slave_data(&rec_data_temp ,data_len);
						CQ_read_offset_inc(cb, msg_len+2);//ƫ��һ֡����

						return 0;
					}
//						printf("��ǰͨ����%d ���ɶ���%u\n",device_channel,CQ_getLength(cb));
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
						return_rw_message(pPolling_data->fd,tmp_buff,msg_len);
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
    
/*
 * ������Ϊmaster������վ���� --�������ݿ�
 * ����:rec_struct ��������cmd datapointer channel_num
 * ����:len cmd 0x03ʱ�ֽڳ���  cmd 0x10ʱack��Ϣ
 * */
static void rec_slave_data(modbus_master_rec_t *rec_struct ,uint8_t len)
{
	uint16_t data_temp = 0;
	//���ݵ�ַ
	uint8_t *data_addr = rec_struct->data_addr;
	//��ȡ����ͨ����Ϣ
	uint8_t channel_num = rec_struct->channnel;
	//���ݶ�Ӧ��ʼ�Ĵ�����ַ
	uint16_t reg_addr = polling_msg[channel_num].read_reg;
	//������վ���صĶ���������
	if(rec_struct->cmd == 0x03)
	{
#if ENABLE_MODBUS_DEBUG
		printf("����ͨ����%u ���ݳ���:%d\n",channel_num,len);
#endif
		for(uint16_t read_offset = 0; read_offset < len;read_offset += 2)
		{
			//׼������
			data_temp = (*(data_addr+read_offset))&0xFF;
			data_temp <<= 8;
			data_temp |= (*(data_addr+read_offset+1))&0xFF;
			rec_struct->rec_data = data_temp;
			//��������
#if ENABLE_MODBUS_DEBUG
			printf("���ռĴ���%u--����:%04X---%u\n",reg_addr,data_temp,data_temp);
#endif
			//���ô����Ӧ�Ĵ���
			MasteReg_process_map[MasterCheck_Modbus_Addr(reg_addr)].func(rec_struct);
			//��һ���Ĵ���
			reg_addr++;
		}
	}
	//������վ���ص�д����ACK
	if(rec_struct->cmd == 0x10)
	{

	}
}

/* ͸�� ģʽ��
 * ����λ�����ض�д���� 0x03 0x10 ���
 * */
static void return_rw_message(UART_HandleTypeDef *fd,uint8_t *ptr,uint16_t msglen)
{
	uint16_t crc_ret = 0;
#if USE_TCP_SEQUENS_NUM
	uint8_t send_master_buff[UART_SEND_BUFF_MAX];
#endif
	//�޸�����
	ptr[0] = get_end_point(fd);//�����̶�ͷ���豸ID
	crc_ret = CRC_Return(ptr,msglen);
	ptr[msglen] =(uint8_t)(crc_ret &0x00FF);//���޷�����Ҫ��
	ptr[msglen+1] = (uint8_t)((crc_ret>>8)&0x00FF);
	//��ӡ����
	debug_print((uint8_t*)ptr,msglen+2);
	printf("master : ת��!\n");
#if USE_TCP_SEQUENS_NUM
	send_master_buff[0] = (msg_sequens_num>>8)&0xFF;
	send_master_buff[1] = msg_sequens_num&0xFF;
	memcpy(send_master_buff+2,ptr,msglen+2);
	debug_print(send_master_buff,msglen+2+2);
	//��������
	ModbusUsart_tx_Master(send_master_buff, msglen+2+2);//�����������ϼ�
#else
	debug_print((uint8_t*)ptr,msglen+2);
	//��������
	ModbusUsart_tx_Master(ptr, msglen+2);//�����������ϼ�
#endif
}


#ifdef __cplusplus //ʹ�ã����
}
#endif