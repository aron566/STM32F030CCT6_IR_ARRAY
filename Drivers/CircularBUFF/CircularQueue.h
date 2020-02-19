/*
 * CircularQueue.h
 *
 *  Created on: 2020年1月11日
 *      Author: Aron566
 */

#ifndef CIRCULARQUEUE_H_
#define CIRCULARQUEUE_H_
#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>

#define GET_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define IS_POWER_OF_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

#define TRUE true
#define FALSE false

typedef struct
{
	uint8_t *dataBufer;
	uint32_t size;
	uint32_t entrance;
	uint32_t exit;
}CQ_handleTypeDef;


bool CQ_init(CQ_handleTypeDef *CircularQueue ,uint8_t *memAdd,uint16_t len);
bool CQ_isEmpty(CQ_handleTypeDef *CircularQueue);
bool CQ_isFull(CQ_handleTypeDef *CircularQueue);
void CQ_emptyData(CQ_handleTypeDef*CircularQueue);
uint32_t CQ_getLength(CQ_handleTypeDef *CircularQueue);
uint32_t CQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len);
uint32_t CQ_putData(CQ_handleTypeDef *CircularQueue, uint8_t *sourceBuf, uint32_t len);

/*修改后的加入数据操作--数据长度作为帧头先存入缓冲区*/
uint32_t DQ_putData(CQ_handleTypeDef *CircularQueue, uint8_t *sourceBuf, uint32_t len);
/*修改后的取数据操作--会直接读取帧长度信息，依据当前一包长度加入缓冲区*/
uint32_t DQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf);

/*修改后的获取数据操作--数据读取后不会减小缓冲区长度，需手动减小,目的为了分步取出完整数据*/
uint32_t CQ_get_buff_Data(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len);
/*加入数据不做改变*/
/*手动缩减缓冲区长度--用作：错误帧偏移-正确帧读取后剔除*/
void CQ_read_offset_inc(CQ_handleTypeDef *CircularQueue, uint32_t len);

/*分配一个缓冲区并进行初始化--替代--CQ_init*/
CQ_handleTypeDef *cb_create(uint32_t buffsize);
#ifdef __cplusplus //使用ｃ编译
}
#endif
#endif /* CIRCULARQUEUE_H_ */
