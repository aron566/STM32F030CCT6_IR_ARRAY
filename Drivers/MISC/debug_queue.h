/******************************************************************************/
#pragma once
#include <stdint.h>

//调试消息
typedef struct
{
    char  buffer[50];
}debug_msg_t;

int DebugPrintfQueue_recv(debug_msg_t *pMsg, uint32_t time_ms);
int DebugPrintfQueue_send(debug_msg_t *pMsg, uint32_t time_ms);

