/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-09-07 10:26     
*******************************************************************************/
#include "debugPrintf.h"
#include "debug_queue.h"
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define PRINTF_FILE_LINE_ENABLE  1   //使能调试输出文件名和行号
#define PRINTF_LEVEL             DEBUG_LEVEL_DEBUG   //打印等级
#define MAX_DEBUG_MSG_LENGTH     100 //调试信息最大长度


//从路径中获取文件名
const char *get_fileName(const char *path)
{
    const char *fileName = 0;
    const char *pChar;
    uint16_t length;
    
    length = strlen(path);
    for (int i = length - 1; i > 0; i--)
    {
        pChar = path + i;
        if ( ('\\' == *pChar)
            || ('/') == *pChar) 
        {
            fileName = pChar + 1;
            break;
        }
    }
    return fileName;
}

//通过消息队列发送调试信息
static int debug_queue_send(const char *pMsg)
{
    uint16_t length, sent_length;
    length = strlen(pMsg);
    if (length > MAX_DEBUG_MSG_LENGTH)  //超过最大长度
    {
        length = MAX_DEBUG_MSG_LENGTH;
    }
    sent_length = 0;
    while (sent_length < length)
    {
        if (DebugPrintfQueue_send((debug_msg_t *)&pMsg[sent_length], 0) < 0)
        {
            return -1;
        }
        sent_length += sizeof(debug_msg_t);
    }
    return 0;
}

/*
    description: 打印调试信息
    note:        打印长度不能超过 MAX_DEBUG_MSG_LENGTH
    parameter:
            debug_level     调试等级
            c_file          c文件名
            c_line          文件行号
            format          格式化打印
    return: -1 错误
             0 正常
*/
int debug_info ( debug_level_e debug_level, const char *c_file, const int c_line, const char *format, ... )
{
    if ( (debug_level <= DEBUG_LEVEL_ZERO)
        || (debug_level >= DEBUG_LEVEL_END)     //等级不对
            || (debug_level > PRINTF_LEVEL) )   //等级不在打印范围
    {
        return -1;
    }
    
    va_list ap;
    char msg_info[MAX_DEBUG_MSG_LENGTH] = {0};
    uint16_t msg_length = 0;
    
//    //打印调试等级
//    switch (debug_level)
//    {
//    case DEBUG_LEVEL_ERROR:
//        strcpy(&msg_info[msg_length], "error:");
//        break;
//    case DEBUG_LEVEL_WARNING:
//        strcpy(&msg_info[msg_length], "warning:");
//        break;
//    case DEBUG_LEVEL_CRITICAL:
//        strcpy(&msg_info[msg_length], "critical:");
//        break;
//    case DEBUG_LEVEL_NORMAL:
//        strcpy(&msg_info[msg_length], "normal:");
//        break;
//    case DEBUG_LEVEL_DEBUG:
//        strcpy(&msg_info[msg_length], "debug:");
//        break;
//    default:
//        return -1;
//    }
//    msg_length = strlen(msg_info);
    
#if PRINTF_FILE_LINE_ENABLE
    //打印文件名和行号
    snprintf(&msg_info[msg_length], sizeof(msg_info) - msg_length, "%s[%d]", get_fileName(c_file), c_line);
    msg_length = strlen(msg_info);
#endif

    //打印消息内容
    va_start(ap, format);
    vsnprintf(&msg_info[msg_length], sizeof(msg_info) - msg_length, format, ap);
    va_end(ap);
    strcat(msg_info, "\n");
    if ( debug_queue_send(msg_info) < 0 )
    {
        return -1;
    }
    return 0;
}

