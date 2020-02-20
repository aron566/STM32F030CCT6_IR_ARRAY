/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-09-07 10:26     
*******************************************************************************/
#include "debug_queue.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern QueueHandle_t DebugPrintfQueueHandle;    //调试打印消息队列

/*
    description: 调试（打印）队列接收消息
    parameter:
            pMsg    接收的消息
            time_ms 等待时间。0不等待; portMAX_DELAY一直等待
    return: -1 无消息
             0 有消息
*/
int DebugPrintfQueue_recv(debug_msg_t *pMsg, uint32_t time_ms)
{
    TickType_t ticks;
    ticks = time_ms / portTICK_PERIOD_MS;
    if (pdTRUE != xQueueReceive(DebugPrintfQueueHandle, pMsg, ticks))
    {
        return -1;
    }
    return 0;
}

/*
    description: 调试（打印）队列发送消息
    parameter:
            pMsg    发送的消息
            time_ms 等待时间。0不等待; portMAX_DELAY一直等待
    return: -1 发送成功
             0 发送失败
*/
int DebugPrintfQueue_send(debug_msg_t *pMsg, uint32_t time_ms)
{
    TickType_t ticks;
    ticks = time_ms / portTICK_PERIOD_MS;
    if (pdTRUE != xQueueSend(DebugPrintfQueueHandle, pMsg, ticks))
    {
        return -1;
    }
    return 0;
}

