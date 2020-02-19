/*
 *  reg_master.c
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
#include "modbus_reg.h"

#define MAX_REG_COUNT 	26U//�������Ĵ�����������    

/*��������*/
static int Check_Modbus_Addr(uint16_t Reg_start_addr);

static uint16_t Real_Time_Temp_Int(void *data);
static uint16_t People_Num_Hour(void *data);
static uint16_t People_Num_Day(void *data);
static uint16_t People_Num_Week(void *data);
static uint16_t People_Num_Month(void *data);
static uint16_t Ir_Matrix_Graph(void *data);/*����ͼ��*/
static uint16_t Max_Record_Num(void *data);
static uint16_t Current_Record_Num_H(void *data);
static uint16_t Current_Record_Num_L(void *data);
static uint16_t First_Record_Num_H(void *data);
static uint16_t First_Record_Num_L(void *data);
static uint16_t Set_Alarm_Num_H(void *data);
static uint16_t Set_Alarm_Num_L(void *data);
static uint16_t Alarm_Dev_Num(void *data);
static uint16_t Alarm_Current_Temp(void *data);
static uint16_t Read_Alarm_Temp_Set(void *data);
static uint16_t Read_Alarm_Time_S_H(void *data);
static uint16_t Read_Alarm_Time_S_L(void *data);
static uint16_t Command_Control(void *data);
static uint16_t Set_Alarm_Low_Temp(void *data);
static uint16_t Set_Alarm_Hi_Temp(void *data);
static uint16_t Set_Modbus_Id(void *data);
static uint16_t Set_Baud_Rate(void *data);
static uint16_t Set_Sys_Time_H(void *data);
static uint16_t Set_Sys_Time_L(void *data);

/*δ�ҵ�Ĭ�ϴ���*/
static uint16_t Unused_Ack(void *data);



/*�Ĵ�����Ӧ����ӳ�� --ÿ���������б� ����� MAX_REG_COUNT ֵ */
modbus_process_t reg_process_map[MAX_REG_COUNT] =
{
	{REAL_TIME_TEMP_INT ,Real_Time_Temp_Int},//1
	{PEOPLE_NUM_HOUR ,People_Num_Hour},//2
	{PEOPLE_NUM_DAY ,People_Num_Day},//3
	{PEOPLE_NUM_WEEK ,People_Num_Week},//4
	{PEOPLE_NUM_MONTH ,People_Num_Month},//5
	{IR_MATRIX_GRAPH ,Ir_Matrix_Graph},//6
	{MAX_RECORD_NUM ,Max_Record_Num},//7
	{CURRENT_RECORD_NUM_H ,Current_Record_Num_H},//8
	{CURRENT_RECORD_NUM_L ,Current_Record_Num_L},//9
	{FIRST_RECORD_NUM_H ,First_Record_Num_H},
	{FIRST_RECORD_NUM_L ,First_Record_Num_L},
	{SET_ALARM_NUM_H ,Set_Alarm_Num_H},
	{SET_ALARM_NUM_L ,Set_Alarm_Num_L},
	{ALARM_DEV_NUM ,Alarm_Dev_Num},
	{ALARM_CURRENT_TEMP ,Alarm_Current_Temp},
	{READ_ALARM_TEMP_SET ,Read_Alarm_Temp_Set},
	{READ_ALARM_TIME_S_H ,Read_Alarm_Time_S_H},
	{READ_ALARM_TIME_S_L ,Read_Alarm_Time_S_L},
	{COMMAND_CONTROL ,Command_Control},
	{SET_ALARM_LOW_TEMP ,Set_Alarm_Low_Temp},
	{SET_ALARM_HI_TEMP ,Set_Alarm_Hi_Temp},
	{SET_MODBUS_ID ,Set_Modbus_Id},
	{SET_BAUD_RATE ,Set_Baud_Rate},
	{SET_SYS_TIME_H ,Set_Sys_Time_H},
	{SET_SYS_TIME_L ,Set_Sys_Time_L},
	{0xFFFF,Unused_Ack}
};   


/*
 * ������Ϊmaster������վ���� --�������ݿ�
 *
 * ����:rec_struct ��������cmd datapointer channel_num
 * ����:len cmd 0x03ʱ�ֽڳ���  cmd 0x10ʱack��Ϣ
 * */
void rec_slave_data(modbus_master_rec_t *rec_struct ,uint8_t len)
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
			reg_process_map[Check_Modbus_Addr(reg_addr)].func(rec_struct);
			//��һ���Ĵ���
			reg_addr++;
		}
	}
	//������վ���ص�д����ACK
	if(rec_struct->cmd == 0x10)
	{

	}
}


/* ���Ҷ�Ӧ�Ĵ����������� */
static int Check_Modbus_Addr(uint16_t Reg_start_addr)
{
	int ret = 0;
	uint8_t index = 0;
	for(;index < MAX_REG_COUNT;index++)
	{
		if(reg_process_map[index].register_num == Reg_start_addr)
		{
			ret = index;
			return ret;
		}
	}
	return MAX_REG_COUNT-1;/*δ�ҵ�,ʹ��Ĭ�ϴ���*/
}

/* δ���ҵ���Ӧ�Ĵ��������� */
static uint16_t Unused_Ack(void *data)
{
  return 0;
}

/*ʵʱ�¶�*/
static uint16_t Real_Time_Temp_Int(void *data)
{
    return 0;
}

/*��ǰСʱ��������*/
static uint16_t People_Num_Hour(void *data)
{
    return 0;
}

/*������������*/
static uint16_t People_Num_Day(void *data)
{
    return 0;
}

/*������������*/
static uint16_t People_Num_Week(void *data)
{
    return 0;
}

/*������������*/
static uint16_t People_Num_Month(void *data)
{
    return 0;
}

/*����ͼ�� 1024��u16����*/
static uint16_t Ir_Matrix_Graph(void *data)
{
    return 0;
}

/*����¼��*/
static uint16_t Max_Record_Num(void *data)
{
    return 0;
}

/*��ǰ��¼���*/
static uint16_t Current_Record_Num_H(void *data)
{
    return 0;
}
static uint16_t Current_Record_Num_L(void *data)
{
    return 0;
}

/*���ȼ�¼���*/
static uint16_t First_Record_Num_H(void *data)
{
    return 0;
}
static uint16_t First_Record_Num_L(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Num_H(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Num_L(void *data)
{
    return 0;
}
static uint16_t Alarm_Dev_Num(void *data)
{
    return 0;
}
static uint16_t Alarm_Current_Temp(void *data)
{
    return 0;
}
static uint16_t Read_Alarm_Temp_Set(void *data)
{
    return 0;
}
static uint16_t Read_Alarm_Time_S_H(void *data)
{
    return 0;
}
static uint16_t Read_Alarm_Time_S_L(void *data)
{
    return 0;
}
static uint16_t Command_Control(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Low_Temp(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Hi_Temp(void *data)
{
    return 0;
}
static uint16_t Set_Modbus_Id(void *data)
{
    return 0;
}
static uint16_t Set_Baud_Rate(void *data)
{
    return 0;
}

static uint16_t Set_Sys_Time_H(void *data)
{
    return 0;
}
static uint16_t Set_Sys_Time_L(void *data)
{
    return 0;
}


#ifdef __cplusplus //ʹ�ã����
}
#endif