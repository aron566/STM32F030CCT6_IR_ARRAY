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
#include "modbus_port.h" /*外部方法*/
    
/*****************************实时数据********************************/
#define REAL_TIME_TEMP			    0x0101U//实时温度 温度值*100 单位摄氏度
#define IR_MATRIX_GRAPH_START       0xF000U//红外图像像素数据 0xf3ff 1024个
#define IR_MATRIX_GRAPH_END         0xF3FFU//红外图像像素数据
                          /*本机提供数据*/
#define PEOPLE_NUM_HOUR             0x0110U//当小时内人流量
#define PEOPLE_NUM_DAY              0x0111U//当天的人流量
#define PEOPLE_NUM_WEEK             0x0112U//当周人流量
#define PEOPLE_NUM_MONTH            0x0113U//当月人流量
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
#define SET_BAUD_RATE_H             0x2011U//设置通讯波特率
#define SET_BAUD_RATE_L             0x2012U//设置通讯波特率低字节   
#define SET_SYS_TIME_H              0x201EU//1970年到当前时间的秒数
#define SET_SYS_TIME_L              0x201FU//
    
typedef struct 
{
    //本机提供参数-->
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
    uint16_t alarm_time_cnt_H_val;//报警持续时间
    uint16_t alarm_time_cnt_L_val;
    /*相关设置*/
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
    //红外设备 提供--->
    uint16_t real_time_tmp[UART_NUM_MAX];
    uint16_t ir_graph[256];//共用
}device_reg_info_t;


/*查找索引*/
int SlaveCheck_Modbus_Addr(uint16_t Reg_start_addr);
int MasterCheck_Modbus_Addr(uint16_t Reg_start_addr);
/*寄存器处理映射*/
extern modbus_process_t MasteReg_process_map[];
extern modbus_process_t SlaveReg_process_map[];

/*寄存器 对应 数值存储区域*/
extern device_reg_info_t device_v_info;

/*存储本机数据 数值存储区域 采用一维数组*/
//extern uint8_t device_slave_info[];

/*存储多个从站数据 采用二维数组 以modbusID作为行号*/
//extern uint8_t device_master_info[][256];


#ifdef __cplusplus //使用ｃ编译
}
#endif

#endif