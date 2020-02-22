/*
 *  misc_task.c
 *
 *  Created on: 2020年2月20日
 *      Author: Aron566
 *
 *
 */
#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif
  
#include "misc_task.h"
#include "modbus_reg.h"
static uint16_t People_Num_Cnt(void);

static uint16_t Calc_Tmp(Calc_Mode_t mode ,uint16_t *data ,uint16_t len);
static uint8_t Check_16bitData_Sum(uint8_t *data ,uint16_t len);
static uint16_t Check_Graph_Data(uint8_t *data ,int len);
static void Read_IR_Temperature_Graph(void);
static void Read_IR_Temperature(void);

static void update_people_num_month(uint16_t num);
static void update_people_num_week(uint16_t num);
static void update_people_num_day(uint16_t num);
static void update_people_num_hour(uint16_t num);
void start_misc_task_loop(void)
{
  //初始化

  //RTC时间初始化
  RTC_Set_Date(7,2,20,20);	/*设置日期*/
  RTC_Set_Time(19,0,40);	/*设置时间*/
  for(;;)
  {
    //喂狗
    IWDG_clr();
    
    //RTC
    Get_Date_Time();
    
    //继电器控制检测
    ststc
    //报警--声光
    
    //人流量更新检测
    People_Num_Cnt();
    
    //控制人流量-温度数据显示
    
    
    osDelay(100);
  }
  
}

/*数据显示控制*/
static void display_control(void)
{


}


/*
 * 每1ms检测按键按下事件处理 ,需将方法1ms调用一次
 *                                                            - - - - - - - - - - -- - - -
 * 参数：端口号 引脚号 需检测的状态 检测标准时间（消抖时间ms）|_全局或静态数据保存区指针_|time
 *
 * 返回：1状态正确 0状态错误
 */
uint8_t Key_State_Check(GPIO_TypeDef* GPIOx ,uint16_t GPIO_Pin ,GPIO_PinState state ,uint16_t check_time ,uint16_t *time)
{
    if(state == HAL_GPIO_ReadPin(GPIOx, GPIO_Pin))//读取按键状态
    {
        //消除抖动
        *time++;
        if(*time >= check_time)
        {
            *time = 0;
            return 1;
        }
    }
    else
    {
        *time = 0;
        return 0;
    }
    return 0;
}

/*
 * 计算人流量，更新数据 
 *
 * 返回：当天人数
 */
static uint16_t People_Num_Cnt(void)
{
    static uint32_t people_leave_time = 0;
    static uint8_t people_enter_flag = 0;
    //计算最大值
    uint16_t tmp = Calc_Tmp(MAX_VALUE_MODE ,device_v_info.real_time_tmp ,REAL_TIME_TMP_BUFF_MAX);
    //上升沿触发计数
    if(tmp < NORMAL_TEMPERATURE && people_enter_flag == 1)
    {
       uint32_t time_tmp = get_time_sec(); 

       if((time_tmp - people_leave_time) < MAX_CHECK_INTERVAL)
       {
          return device_v_info.people_cnt_day_val; 
       }
       else
       {
          people_enter_flag = 0;
       } 
    }
    if(tmp > NORMAL_TEMPERATURE && tmp < MAX_TEMPERATURE)
    {
        if(people_enter_flag == 0)
        {
            //计入
            update_people_num_hour(1);
            people_enter_flag = 1;
        }
        /*更新离开时间*/
        people_leave_time = get_time_sec();
        return device_v_info.people_cnt_day_val;
    }
    return device_v_info.people_cnt_day_val;
}

/*人流量 小时更新*/
static void update_people_num_hour(uint16_t num)
{
    //小时人数更新
    device_v_info.people_cnt_hour_val += num;
    //rtc小时已更新
    if(current_date_time.minute == 0 && current_date_time.Seconds <= 1)
    {
        //当天人数累加
        update_people_num_day(device_v_info.people_cnt_hour_val);
        //清空当前小时人数
        device_v_info.people_cnt_hour_val = 0;
    }
}

/*人流量 天更新*/
static void update_people_num_day(uint16_t num)
{
    //天人数更新
    device_v_info.people_cnt_day_val += num;
    //rtc天已更新
    if(current_date_time.hour == 0 && current_date_time.minute == 0)
    {
        //当周人数累加
        update_people_num_week(device_v_info.people_cnt_day_val);
        //清空当天人数
        device_v_info.people_cnt_day_val = 0;
    }    
}

/*人流量 周更新*/
static void update_people_num_week(uint16_t num)
{
    //周人数更新
    device_v_info.people_cnt_week_val += num;
    //rtc周已更新
    if(current_date_time.WeekDay == 0 && current_date_time.hour == 0)
    {
        //当月人数累加
        update_people_num_month(device_v_info.people_cnt_week_val);
        //清空当周人数
        device_v_info.people_cnt_week_val = 0;
    } 
}
/*人流量 月更新*/
static void update_people_num_month(uint16_t num)
{
    //月人数更新
    device_v_info.people_cnt_month_val += num;  
}

/* 清除当天 人流量数据*/
void clear_people_num_day(uint16_t command)
{
    if(command == 1)
    {
        device_v_info.people_cnt_day_val = 0;
    }
}
/*
 * 计算连续数据中，中位值 最大值 平均值 最小值
 *
 * 参数：返回数据类型 数据地址 数据长度
 * 
 * 返回：依据设置mode 中位值 最大值 平均值 最小值
 */
static uint16_t Calc_Tmp(Calc_Mode_t mode ,uint16_t *data ,uint16_t len)
{
    uint16_t ret = 0;
    uint8_t index = 0;
    uint16_t i,j,temp;
    //插入
    for(i = 1;i < len ;i++) //从数组元素1开始
    {
        temp = data[i];     //将当前数组元素的值给临时变量temp
        for( j = i ;j > 0 && data[j-1] > temp ;j--) //将当前的数组元素号给J 满足j>0且当前元素的前一个元素 大于当前值。则
        {
           data[j] = data[j-1];//将大值给
        }
        data[j] = temp;
    }
    switch(mode)
    {
    case MEDIAN_VALUE_MODE:
        //查询中位值
        if(mode == MEDIAN_VALUE_MODE)
        {
            if(len % 2)
            {
                index = (len-1) /2;
                ret = data[index];
            }
            else
            {
                index = len /2;
                ret = (data[index]+data[index-1])/2;
            }
        }
        return ret;
    case MAX_VALUE_MODE:
        //查询最大值
        return data[index-1]; 
    case AVERAGE_VALUE_MODE:
        //查询平均值
        ret = data[0];
        for(uint16_t index = 1;index < len;index++)
        {
            ret = (ret+data[index])/2;
        }
        return ret;
    case MIN_VALUE_MODE:
        //查询最小值
        return data[0];
    default:
      return 0;
    }
}

/*轮询采集红外信息任务*/
void Read_IR_Temperature_loop(void)
{
    //采集温度
    Read_IR_Temperature();
    //采集图像
    Read_IR_Temperature_Graph();
}

/*读取 温度 命令：0xA5 0x55 0x01 0xFB
  0xA5 0x55 0x4E 0x0E 0x13 0x06 0x6F  校验 6f 温度：(0x4e+256*0x0e)/100 0x13列 0x06行
*/
static void Read_IR_Temperature(void)
{
    static uint8_t read_tmp_buff[4] = {0xA5,0x55,0x01,0xFB};
    uint8_t rec_buff[7] = {0};
    uint16_t index = 0;
    UART_write(UART_TTY1, read_tmp_buff, 4);
    uart_read(UART_TTY1, (uint8_t *)rec_buff, 7, 10);
    //检验数据
    if(Check_Data_Sum(rec_buff ,6))
    {
        //取出温度
        device_v_info.real_time_tmp[index] = rec_buff[2]+256*rec_buff[3];
        index = (index+1)%REAL_TIME_TMP_BUFF_MAX;
        //取出行 列信息
        device_v_info.ir_graph_tmp_point_x = rec_buff[5];
        device_v_info.ir_graph_tmp_point_y = rec_buff[4];
    }    
}

/*读取 红外温度图像*/
static void Read_IR_Temperature_Graph(void)
{
    static uint8_t read_tmp_buff[4] = {0xA5,0x35,0x01,0xDB};
    uint16_t tmp_data_len = 0;
    uint8_t rec_buff[1024*2+100] = {0};
    //读
    UART_write(UART_TTY1, read_tmp_buff, 4);
    //取
    int data_len = uart_read(UART_TTY1, rec_buff, 2100, 10);
    //校验
    tmp_data_len = Check_Graph_Data(rec_buff ,data_len);
    if(tmp_data_len)
    {
        //复制寄存器数据
        for(uint16_t i = 0;i < tmp_data_len;i++)
        {
            //更新图像数据
            device_v_info.ir_graph[i] = rec_buff[4+i];
        }
    } 
}
/* 8bit和校验 -判断连续len个数据和 ，与len+1位置的数据是否一致
 *
 * 参数：连续数据地址的首地址 校验的数据长度
 *
 * 返回：校验结果：1 正确  0错误 
 */

uint8_t Check_Data_Sum(uint8_t *data ,uint16_t len)
{
    uint8_t ret = 0;  
    for(uint16_t i = 0;i < len;i++)
    {
        ret += data[i];
    }
    if(ret == data[len])
    {
        return 1;
    }
    return 0;
}

/*
 * 检验红外数据帧
 *
 * 参数： 数据起始地址 数据长度
 *
 * 返回：数据长度  0校验失败
 */
static uint16_t Check_Graph_Data(uint8_t *data ,int len)
{
    uint8_t frame_header = 0x5A;
    uint16_t data_len = 0;
    //判断必要长度
    if(len > 4)
    {
        //判断帧头
        if(frame_header == data[0] && frame_header == data[1])
        {
            //取出数据包长度 不含帧头+检验值+数量 共计6个字节
            data_len = data[2]+data[3]*256;//data_len=1538  total=1544 check=1542
            //判断包完整
            if(data_len+6 <= len)
            {
                //16bit 和校验 去掉检验 2字节长度
                if(Check_16bitData_Sum(data ,data_len+4))
                {
                    return data_len;
                }
            }
        }
    }
    return 0;
}

/* 16bit和校验 -判断连续len个数据和 ，CL在前 CH在后 判断与 len len+1位置的数据是否一致
 *
 * 参数：连续数据地址的首地址 校验的数据长度
 *
 * 返回：校验结果：1 正确  0错误 
 */

static uint8_t Check_16bitData_Sum(uint8_t *data ,uint16_t len)
{
    uint16_t result = 0;
    uint8_t check_val_L = 0,check_val_H = 0;
    
    for(uint16_t i = 0;i < len;i++)
    {
        result += data[i];
    }
    check_val_L = result&0xFF;
    check_val_H = (result>>8)&0xFF;
    if(check_val_L == data[len] && check_val_H == data[len+1])
    {
        return 1;
    }
    return 0;
}
#ifdef __cplusplus //使用ｃ编译
}
#endif