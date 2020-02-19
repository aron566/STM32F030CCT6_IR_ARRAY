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

#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif
/*****************************实时数据********************************/
#define REAL_TIME_TEMP_INT			0x0101U//实时温度 温度值*100 单位摄氏度
#define PEOPLE_NUM_HOUR             0x0110U//当小时内人流量
#define PEOPLE_NUM_DAY              0x0111U//当天的人流量
#define PEOPLE_NUM_WEEK             0x0112U//当周人流量
#define PEOPLE_NUM_MONTH            0x0113U//当月人流量
#define IR_MATRIX_GRAPH             0xF000U//红外图像像素数据 0xf3ff 1024个
/***************************报警事件状态******************************/
#define MAX_RECORD_NUM              0xE000U//最大记录数
#define CURRENT_RECORD_NUM_H        0xE001U//最新记录编号
#define CURRENT_RECORD_NUM_L        0xE002U//最新记录编号
#define FIRST_RECORD_NUM_H          0xE003U//最早记录编号
#define FIRST_RECORD_NUM_L          0xE004U//最早记录编号
/***************************报警事件内容******************************/
#define SET_ALARM_NUM_H             0xE010U//即将读取的事件编号
#define SET_ALARM_NUM_L             0xE011U//即将读取的事件编号
#define ALARM_DEV_NUM               0xE012U//报警的设备编号 几号红外
#define ALARM_CURRENT_TEMP          0xE013U//报警时的温度
#define READ_ALARM_TEMP_SET         0xE014U//读取报警时的温度设定
#define READ_ALARM_TIME_S_H         0xE015U//读取报警持续事间
#define READ_ALARM_TIME_S_L         0xE016U//读取报警持续事间
/***************************控制命令******************************/
#define COMMAND_CONTROL             0xC000U//写1清除当天统计人数
/***************************配置参数******************************/
#define SET_ALARM_LOW_TEMP          0x2001U//设置低温报警值 实际温度*100  
#define SET_ALARM_HI_TEMP           0x2002U//设置高温报警值 实际温度*100 
#define SET_MODBUS_ID               0x2010U//设置modbusid 数据控制单元为0 不可更改
#define SET_BAUD_RATE               0x2011U//设置通讯波特率
#define SET_SYS_TIME_H              0x201EU//1970年到当前时间的秒数
#define SET_SYS_TIME_L              0x201FU//
    

    
#ifdef __cplusplus //使用ｃ编译
}
#endif

#endif