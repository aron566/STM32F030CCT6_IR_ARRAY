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
/*****************************ʵʱ����********************************/
#define REAL_TIME_TEMP_INT			0x0101U//ʵʱ�¶� �¶�ֵ*100 ��λ���϶�
#define PEOPLE_NUM_HOUR             0x0110U//��Сʱ��������
#define PEOPLE_NUM_DAY              0x0111U//�����������
#define PEOPLE_NUM_WEEK             0x0112U//����������
#define PEOPLE_NUM_MONTH            0x0113U//����������
#define IR_MATRIX_GRAPH             0xF000U//����ͼ���������� 0xf3ff 1024��
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
#define SET_BAUD_RATE               0x2011U//����ͨѶ������
#define SET_SYS_TIME_H              0x201EU//1970�굽��ǰʱ�������
#define SET_SYS_TIME_L              0x201FU//
    

    
#ifdef __cplusplus //ʹ�ã����
}
#endif

#endif