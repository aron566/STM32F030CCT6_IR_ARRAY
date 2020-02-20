/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-09-07 10:26     
*******************************************************************************/
/* Function Description
    设计说明：
    只有J1+接口，可以作为打印口
    对应的J1+接口被配置成打印口时，LED灯按照1快闪，2慢闪进行指示
    可以配置打印开关和打印级别

    功能说明：
    1. 默认波特率38400
    2. 可以接收外部输入命令，对系统内容进行查看
    3. 按照报警级别，对报警数据进行输出
    4. 打印级别包括, (1):错误、(2)警告、 (3)关键状态、(4)正常状态、(5)调试
    5. 设置报警缓冲区，超过缓冲区的数据进行丢弃
    6. 可通讯命令进行关闭打印，可通过显示板打开打印

    生产的数据
    1. 命令行相关的配置

    接收的消息:
    1. 打印级别及内容
    2. 打印口的配置信息

*/
#include "debugPrintfTask.h"
#include "cmsis_os.h"
#include "debug_queue.h"
#include "debugPrintf.h"
#include <string.h>
#include "GN_uart.h"
#include "peripheral.h"

#define UART_DEBUG  UART_J1

static void test_printf()
{
    debug_msg_t msg;
    uint16_t msg_length;
    
    if ( DebugPrintfQueue_recv(&msg, 0) < 0 )
    {
        return;
    }
    msg_length = strlen(msg.buffer);
    UART_write(UART_DEBUG, (uint8_t *)msg.buffer, msg_length);
}
        
        
void debugPrintf_task()
{
////    UART_set_baudrate(UART_DEBUG, 115200);

    for(;;)
    {
        test_printf();
        osDelay(10);
    }
}
