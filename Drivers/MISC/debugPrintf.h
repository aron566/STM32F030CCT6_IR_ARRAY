/*******************************************************************************

Copyright 2018~2019 GNNGA Inc.All Rights Reserved

*      **ID     **Name    **Date               **Notes**************************
1      G012     仇亚鹏    2019-09-07 10:26     
*******************************************************************************/
#pragma once

#include <stdint.h>

//调试打印等级
typedef enum
{
    DEBUG_LEVEL_ZERO           = 0,
    DEBUG_LEVEL_ERROR,                  //错误
    DEBUG_LEVEL_WARNING,                //警告
    DEBUG_LEVEL_CRITICAL,               //关键状态
    DEBUG_LEVEL_NORMAL,                 //正常状态
    DEBUG_LEVEL_DEBUG,                  //调试状态
    DEBUG_LEVEL_END
}debug_level_e;

typedef struct
{
    uint16_t    JPortNO;        //调试口编号，0表示调试功能关闭
    uint32_t    baudrate;       //调试口波特率;
    debug_level_e    eDebug_level;   //调试级别，高于该级别的信息不打印
}debug_config_t;
    

const char *get_fileName(const char *path);
int debug_info ( debug_level_e debug_level, const char *c_name, const int c_line, const char *format, ... );
//调试打印接口
#define LOG(debug_level, ...) debug_info(debug_level, __FILE__, __LINE__, __VA_ARGS__)

