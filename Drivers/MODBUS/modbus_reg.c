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

#define SLAVE_MAX_REG_COUNT 	35U//�������Ĵ�����������    
#define MASTER_MAX_REG_COUNT 	 3U//�������Ĵ�����������  

device_reg_info_t device_v_info;

static uint16_t Real_Time_Temp(void *data);
static uint16_t Ir_Matrix_Graph(void *data);/*����ͼ��*/

static uint16_t People_Num_Hour(void *data);
static uint16_t People_Num_Day(void *data);
static uint16_t People_Num_Week(void *data);
static uint16_t People_Num_Month(void *data);
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
static uint16_t Set_Baud_Rate_H(void *data);
static uint16_t Set_Baud_Rate_L(void *data);
static uint16_t Set_Sys_Time_H(void *data);
static uint16_t Set_Sys_Time_L(void *data);
static uint16_t Set_Alarm_Low_Temp_Time_H(void *data);
static uint16_t Set_Alarm_Low_Temp_Time_L(void *data);
static uint16_t Set_Alarm_Hi_Temp_Time_H(void *data);
static uint16_t Set_Alarm_Hi_Temp_Time_L(void *data);
static uint16_t Set_Alarm_Low_Temp_Delay(void *data);
static uint16_t Set_Alarm_Hi_Temp_Delay(void *data);
static uint16_t Set_Switch_Endpoint(void *data);
static uint16_t Set_Relay_Toggle_Type(void *data);
static uint16_t Set_Alarm_Affirm_Type(void *data);
/*δ�ҵ�Ĭ�ϴ���*/
static uint16_t Unused_Ack(void *data);



/*�Ĵ�����Ӧ����ӳ�� --ÿ���������б� ����� MAX_REG_COUNT ֵ */
modbus_process_t SlaveReg_process_map[SLAVE_MAX_REG_COUNT] =
{
    {IR_MATRIX_GRAPH_START ,Ir_Matrix_Graph},
	{PEOPLE_NUM_HOUR ,People_Num_Hour},
	{PEOPLE_NUM_DAY ,People_Num_Day},
	{PEOPLE_NUM_WEEK ,People_Num_Week},
	{PEOPLE_NUM_MONTH ,People_Num_Month},
	{MAX_RECORD_NUM ,Max_Record_Num},
	{CURRENT_RECORD_NUM_H ,Current_Record_Num_H},
	{CURRENT_RECORD_NUM_L ,Current_Record_Num_L},
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
	{SET_BAUD_RATE_H ,Set_Baud_Rate_H},
    {SET_BAUD_RATE_L ,Set_Baud_Rate_L},
	{SET_SYS_TIME_H ,Set_Sys_Time_H},
	{SET_SYS_TIME_L ,Set_Sys_Time_L},
    {SET_ALARM_LOW_TEMP_TIME_H ,Set_Alarm_Low_Temp_Time_H},
    {SET_ALARM_LOW_TEMP_TIME_L ,Set_Alarm_Low_Temp_Time_L},
    {SET_ALARM_HI_TEMP_TIME_H ,Set_Alarm_Hi_Temp_Time_H},
    {SET_ALARM_HI_TEMP_TIME_L ,Set_Alarm_Hi_Temp_Time_L},
    {SET_ALARM_LOW_TEMP_DELAY ,Set_Alarm_Low_Temp_Delay},
    {SET_ALARM_HI_TEMP_DELAY ,Set_Alarm_Hi_Temp_Delay},
    {SET_SWITCH_ENDPOINT ,Set_Switch_Endpoint},
    {SET_RELAY_TOGGLE_TYPE ,Set_Relay_Toggle_Type},
    {SET_ALARM_AFFIRM_TYPE ,Set_Alarm_Affirm_Type},
	{0xFFFF,Unused_Ack}
};   

modbus_process_t MasteReg_process_map[MASTER_MAX_REG_COUNT] =
{
	{REAL_TIME_TEMP ,Real_Time_Temp},
    {IR_MATRIX_GRAPH_START ,Ir_Matrix_Graph},
    {0xFFFF,Unused_Ack}
};



/* ������Ϊslave ���Ҷ�Ӧ�Ĵ����������� */
int SlaveCheck_Modbus_Addr(uint16_t Reg_start_addr)
{
	int ret = 0;
	uint8_t index = 0;
	for(;index < SLAVE_MAX_REG_COUNT;index++)
	{
		if(SlaveReg_process_map[index].register_num == Reg_start_addr)
		{
			ret = index;
			return ret;
		}
	}
	return SLAVE_MAX_REG_COUNT-1;/*δ�ҵ�,ʹ��Ĭ�ϴ���*/
}

/*������Ϊmaster ���Ҷ�Ӧ�Ĵ�����������*/
int MasterCheck_Modbus_Addr(uint16_t Reg_start_addr)
{
    int ret = 0;
	uint8_t index = 0;
	for(;index < MASTER_MAX_REG_COUNT;index++)
	{
		if(MasteReg_process_map[index].register_num == Reg_start_addr)
		{
			ret = index;
			return ret;
		}
	}
	return MASTER_MAX_REG_COUNT-1;/*δ�ҵ�,ʹ��Ĭ�ϴ���*/
}


/* δ���ҵ���Ӧ�Ĵ��������� */
static uint16_t Unused_Ack(void *data)
{
  return 0;
}

/*ʵʱ�¶�*/
static uint16_t Real_Time_Temp(void *data)
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

/*����ͨѶ������ ���ֽ�*/
static uint16_t Set_Baud_Rate_H(void *data)
{
	modbus_master_rec_t *temp_data = (modbus_master_rec_t *)data;
	if(temp_data->channnel > UART_NUM_MAX)
	{
		printf("����Խ�磡\n");
		return 0;
	}
    //��ȡ
	if(temp_data->cmd == 0x03)
	{
		return device_v_info.baud_rate_H_val;
	}
    //��ȡ
	if(temp_data->cmd == 0x13)
	{
		return device_v_info.baud_rate_H_val;
	}
    //����
    if(temp_data->cmd == 0x10)
    {
        device_v_info.baud_rate_H_val = (temp_data->rec_data&0xFFFF);
    }
    return 0;
}

/*����ͨѶ������ ���ֽ�*/
static uint16_t Set_Baud_Rate_L(void *data)
{
	modbus_master_rec_t *temp_data = (modbus_master_rec_t *)data;
	if(temp_data->channnel > UART_NUM_MAX)
	{
		printf("����Խ�磡\n");
		return 0;
	}
	if(temp_data->cmd == 0x03)
	{
		return device_v_info.baud_rate_L_val;
	}
	if(temp_data->cmd == 0x13)
	{
		return device_v_info.baud_rate_L_val;
	}
    if(temp_data->cmd == 0x10)
    {
        UART_HandleTypeDef *pHuart = polling_msg[temp_data->channnel].fd;
        uint32_t baud_rate = device_v_info.baud_rate_H_val;
        baud_rate <<= 16;
        device_v_info.baud_rate_L_val = (temp_data->rec_data&0xFFFF);
        //������
        baud_rate |= device_v_info.baud_rate_L_val;
        if ( baud_rate != pHuart->Init.BaudRate )
        {
            HAL_UART_Abort(pHuart); //��ֹͣ�����á����ֱ�����ã������������ڷ����жϣ�������ж���ѭ��
            pHuart->Init.BaudRate = baud_rate;
            HAL_UART_Init(pHuart);
        }
    }
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

static uint16_t Set_Alarm_Low_Temp_Time_H(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Low_Temp_Time_L(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Hi_Temp_Time_H(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Hi_Temp_Time_L(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Low_Temp_Delay(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Hi_Temp_Delay(void *data)
{
    return 0;
}
static uint16_t Set_Switch_Endpoint(void *data)
{
    return 0;
}
static uint16_t Set_Relay_Toggle_Type(void *data)
{
    return 0;
}
static uint16_t Set_Alarm_Affirm_Type(void *data)
{
    return 0;
}

#ifdef __cplusplus //ʹ�ã����
}
#endif