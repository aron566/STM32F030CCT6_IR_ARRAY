/*
 *  FILE: reg_master.h
 *
 *  Created on: 2020/2/18
 *
 *      Author: aron66
 *
 *  DESCRIPTION:--
 */
#ifndef MODBUS_REG_H_
#define MODBUS_REG_H_

#ifdef __cplusplus //ʹ�ã����
extern "C" {
#endif
#include "modbus_port.h" /*�ⲿ����*/
    
/*****************************ʵʱ����********************************/
#define REAL_TIME_TEMP			    0x0101U//ʵʱ�¶� �¶�ֵ*100 ��λ���϶�
#define IR_MATRIX_GRAPH_START       0xF000U//����ͼ���������� 0xf3ff 1024��
#define IR_MATRIX_GRAPH_END         0xF3FFU//����ͼ����������
                          /*�����ṩ����*/
#define PEOPLE_NUM_HOUR             0x0110U//��Сʱ��������
#define PEOPLE_NUM_DAY              0x0111U//�����������
#define PEOPLE_NUM_WEEK             0x0112U//����������
#define PEOPLE_NUM_MONTH            0x0113U//����������
/***************************�����¼�״̬******************************/
#define MAX_RECORD_NUM              0xE000U//����¼��
#define CURRENT_RECORD_NUM_H        0xE001U//���¼�¼���
#define CURRENT_RECORD_NUM_L        0xE002U//���¼�¼���
#define FIRST_RECORD_NUM_H          0xE003U//�����¼���
#define FIRST_RECORD_NUM_L          0xE004U//�����¼���
/***************************�����¼�����******************************/
#define SET_ALARM_NUM_H             0xE010U//������ȡ���¼����
#define SET_ALARM_NUM_L             0xE011U//������ȡ���¼����
#define ALARM_DEV_NUM               0xE012U//�������豸��� ���ź���
#define ALARM_CURRENT_TEMP          0xE013U//����ʱ���¶�
#define READ_ALARM_TEMP_SET         0xE014U//��ȡ����ʱ���¶��趨
#define READ_ALARM_TIME_S_H         0xE015U//��ȡ���������¼�
#define READ_ALARM_TIME_S_L         0xE016U//��ȡ���������¼�
/***************************��������******************************/
#define COMMAND_CONTROL             0xC000U//д1�������ͳ������
/***************************���ò���******************************/
#define SET_ALARM_LOW_TEMP          0x2001U//���õ��±���ֵ ʵ���¶�*100  
#define SET_ALARM_HI_TEMP           0x2002U//���ø��±���ֵ ʵ���¶�*100 
#define SET_MODBUS_ID               0x2010U//����modbusid ���ݿ��Ƶ�ԪΪ0 ���ɸ���
#define SET_BAUD_RATE_H             0x2011U//����ͨѶ������
#define SET_BAUD_RATE_L             0x2012U//����ͨѶ�����ʵ��ֽ�   
#define SET_SYS_TIME_H              0x201EU//1970�굽��ǰʱ�������
#define SET_SYS_TIME_L              0x201FU//
    
typedef struct 
{
    //�����ṩ����-->
    uint16_t people_cnt_hour_val;
    uint16_t people_cnt_day_val;
    uint16_t people_cnt_week_val;
    uint16_t people_cnt_month_val;
    uint16_t max_record_cnt_val;
    uint16_t current_record_num_H_val;
    uint16_t current_record_num_L_val;
    uint16_t first_record_num_H_val;
    uint16_t first_record_num_L_val;
    uint16_t alarm_dev_num_val;   
    uint16_t alarm_current_tmp_val;
    uint16_t alarm_current_tmp_set_val[UART_NUM_MAX];
    uint16_t alarm_time_cnt_H_val;//��������ʱ��
    uint16_t alarm_time_cnt_L_val;
    /*�������*/
    uint16_t set_event_num_H_val;
    uint16_t set_event_num_L_val;   
    uint16_t command_val;
    uint16_t alarm_low_tmp_val;
    uint16_t alarm_hi_tmp_val;
    uint16_t modbus_id_val;
    uint16_t baud_rate_H_val;
    uint16_t baud_rate_L_val;
    uint16_t sys_time_s_H_val;
    uint16_t sys_time_s_L_val;
    //�����豸 �ṩ--->
    uint16_t real_time_tmp[UART_NUM_MAX];
    uint16_t ir_graph[256];//����
}device_reg_info_t;


/*��������*/
int SlaveCheck_Modbus_Addr(uint16_t Reg_start_addr);
int MasterCheck_Modbus_Addr(uint16_t Reg_start_addr);
/*�Ĵ�������ӳ��*/
extern modbus_process_t MasteReg_process_map[];
extern modbus_process_t SlaveReg_process_map[];

/*�Ĵ��� ��Ӧ ��ֵ�洢����*/
extern device_reg_info_t device_v_info;

/*�洢�������� ��ֵ�洢���� ����һά����*/
//extern uint8_t device_slave_info[];

/*�洢�����վ���� ���ö�ά���� ��modbusID��Ϊ�к�*/
//extern uint8_t device_master_info[][256];


#ifdef __cplusplus //ʹ�ã����
}
#endif

#endif