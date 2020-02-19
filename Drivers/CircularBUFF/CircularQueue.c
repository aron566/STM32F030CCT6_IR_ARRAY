/*
 * CircularQueue.c
 *
 *  Created on: 2020年1月11日
 *      Author: Aron566
 *
 *  继承Linux内核 kfifo原理
 *  优点：只有加减与运算，符合先进先出原则，高效的缓存和读取。
 *
 */
#ifdef __cplusplus //使用ｃ编译
extern "C" {
#endif

#include "CircularQueue.h"
#include <string.h>
#include <stdlib.h>
#if USE_LINUX_SYSTEM
#include <sys/types.h>
#endif

/*初始化参数定义：
*CircularQueue作为环形冲区的记录器是个结构体
*memAdd 作为实际数据存储区，
*len 记录实际存储区的最大长度
*/
bool CQ_init(CQ_handleTypeDef *CircularQueue ,uint8_t *memAdd, uint16_t len)
{
    CircularQueue->size = len;

    if (!IS_POWER_OF_2(CircularQueue->size))
        return FALSE;

    if(memAdd == NULL)
    {
    	return FALSE;
    }

    CircularQueue->dataBufer = memAdd;

    memset(CircularQueue->dataBufer, 0, len);
    CircularQueue->entrance = CircularQueue->exit = 0;

    return TRUE;
}

/*环形缓冲区判断是否为空：
*CircularQueue作为环形冲区的记录器，是个结构体
*若写入数据与，读取数据长度一致，那么缓冲区为空return 1
*/
bool CQ_isEmpty(CQ_handleTypeDef *CircularQueue)
{
    if (CircularQueue->entrance == CircularQueue->exit)
        return TRUE;
    else
        return FALSE;
}

/*环形缓冲区判断是否为满
*CircularQueue作为环形冲区的记录器，是个结构体
*若 【已】写入数据与，减去 【已】读取数据长度 = 剩余空间 剩余空间==总长度  判断满
*/
bool CQ_isFull(CQ_handleTypeDef *CircularQueue)
{
    if ((CircularQueue->entrance - CircularQueue->exit) == CircularQueue->size)//MAXSIZE=5,Q.rear=2,Q.front=3？
        return TRUE;//空
    else
        return FALSE;
}

/*环形缓冲区获取剩余空间长度：
*CircularQueue作为环形冲区的记录器，是个结构体
*若 【已】写入数据与，减去 【已】读取数据长度 = 剩余空间 
*/
uint32_t CQ_getLength(CQ_handleTypeDef*CircularQueue)
{
    return (CircularQueue->entrance - CircularQueue->exit);
}

/*环形缓冲区清空操作：
*CircularQueue作为环形冲区的记录器，是个结构体
* 已读和可读数据长度清零      实际存储区清空
*/
void CQ_emptyData(CQ_handleTypeDef*CircularQueue)
{
    CircularQueue->entrance = CircularQueue->exit = 0;
    memset(CircularQueue->dataBufer, 0, CircularQueue->size);
}


/*
*环形缓冲区读走数据：
*CircularQueue作为环形冲区的记录器，是个结构体
*targetBuf 为临时数据处理处
*len 为本次数据读取长度 
*使用写入长度-读取的长度 == 剩余可读  ，要读   取小值
*/
uint32_t CQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len)
{
    uint32_t size = 0;

    /*此次读取的实际大小，取 缓存事件数据大小 和 目标读取数量  两个值小的那个*/
    len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);// 假设总大小10  写入了5 - 已读4  == 1 未读   要读5个   返回1
    /*原理雷同存入*/
    size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));//10 - 0 > 1 返回1
    memcpy(targetBuf, CircularQueue->dataBufer + (CircularQueue->exit & (CircularQueue->size - 1)), size);//偏移0个 复制一个字节
    memcpy(targetBuf + size, CircularQueue->dataBufer, len - size);// 存储区偏移0个字节
    /*利用无符号数据的溢出特性*/
    CircularQueue->exit += len;//取出数据加 len 记录

    return len;
}


/*环形缓冲区加入新数据：存入数据功能已做修改：每次数据帧开头先存入本帧的数据长度，所以每次先取一个字节得到包长度，再按长度取包
*CircularQueue作为环形冲区的记录器，是个结构体
*sourceBuf 为实际存储区地址
*len 为本次数据存入长度 
*使用总长度-已写入+读取完的 == 可用空间大小
*对kfifo->size取模运算可以转化为与运算，如：kfifo->in % kfifo->size 可以转化为 kfifo->in & (kfifo->size – 1)
*/
uint32_t CQ_putData(CQ_handleTypeDef *CircularQueue, uint8_t * sourceBuf, uint32_t len)
{
    uint32_t size = 0;
    /*此次存入的实际大小，取 剩余空间 和 目标存入数量  两个值小的那个*/
    len = GET_MIN(len, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);
    
    /*&(size-1)代替取模运算，同上原理，得到此次存入队列入口到末尾的大小*/
    size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));
    memcpy(CircularQueue->dataBufer + (CircularQueue->entrance & (CircularQueue->size - 1)), sourceBuf, size);
    memcpy(CircularQueue->dataBufer, sourceBuf + size, len - size);//下次需要写入的数据长度

    /*利用无符号数据的溢出特性*/
    CircularQueue->entrance += len; //写入数据记录

    return len;
}

/*修改后的-->环形缓冲区加入新数据：存入数据功能已做修改：每次数据帧开头先存入本帧的数据长度，所以每次先取一个字节得到包长度，再按长度取包
*CircularQueue作为环形冲区的记录器，是个结构体
*sourceBuf 为实际存储区地址
*len 为本次数据存入长度
*使用总长度-已写入+读取完的 == 可用空间大小
*对kfifo->size取模运算可以转化为与运算，如：kfifo->in % kfifo->size 可以转化为 kfifo->in & (kfifo->size – 1)
*/
uint32_t DQ_putData(CQ_handleTypeDef *CircularQueue, uint8_t * sourceBuf, uint32_t len)
{
    uint32_t size = 0;
    uint32_t lenth = 1;
    uint32_t pack_len = len;
    /*此次存入的实际大小，取 剩余空间 和 目标存入数量  两个值小的那个*/
    len = GET_MIN(len+lenth, CircularQueue->size - CircularQueue->entrance + CircularQueue->exit);//长度上头部加上数据长度记录

    /*&(size-1)代替取模运算，同上原理，得到此次存入队列入口到末尾的大小*/
    size = GET_MIN(len, CircularQueue->size - (CircularQueue->entrance & (CircularQueue->size - 1)));//5
    memcpy(CircularQueue->dataBufer + (CircularQueue->entrance & (CircularQueue->size - 1)), &pack_len, lenth);//存入数据包长度，占1字节
    memcpy(CircularQueue->dataBufer + (CircularQueue->entrance & (CircularQueue->size - 1))+lenth, sourceBuf, size-lenth);//地址再次偏移cmd_len，代表尾部缩短一个
    memcpy(CircularQueue->dataBufer, sourceBuf + size - lenth, len - size);//下次需要写入的数据长度（长度不需-1，因为已写入，起始地址需要-1，因为实际包没那么长）

    /*利用无符号数据的溢出特性*/
    CircularQueue->entrance += len; //写入数据记录

    return len;
}

/*
*修改后的-->环形缓冲区读走数据：DQ会调用CQ取走一字节数据用来判断本次数据包长度
*CircularQueue作为环形冲区的记录器，是个结构体
*targetBuf 为临时数据处理处
*len 为本次数据读取长度
*使用写入长度-读取的长度 == 剩余可读  ，要读   取小值
*/
uint32_t DQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf)
{
    uint32_t size = 0;
    uint32_t len = 0;
    //存储帧头 长度信息
    uint8_t package_len[1];
    //获取长度信息
    CQ_getData(CircularQueue, (uint8_t *)package_len, 1);
    len = package_len[0];
    /*此次读取的实际大小，取 缓存事件数据大小 和 目标读取数量  两个值小的那个*/
    len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);// 假设总大小10  写入了5 - 已读4  == 1 未读   要读5个   返回1
    /*原理雷同存入*/
    size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));//10 - 0 > 1 返回1
    memcpy(targetBuf, CircularQueue->dataBufer + (CircularQueue->exit & (CircularQueue->size - 1)), size);//偏移0个 复制一个字节
    memcpy(targetBuf + size, CircularQueue->dataBufer, len - size);// 存储区偏移0个字节
    /*利用无符号数据的溢出特性*/
    CircularQueue->exit += len;//取出数据加 len 记录

    return len;
}



/*
*环形缓冲区读走数据：(手动缓冲区长度记录---适用于modbus解析)
*CircularQueue作为环形冲区的记录器，是个结构体
*targetBuf 为临时数据处理处
*len 为本次数据读取长度
*使用写入长度-读取的长度 == 剩余可读  ，要读   取小值
*/
uint32_t CQ_get_buff_Data(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len)
{
    uint32_t size = 0;

    /*此次读取的实际大小，取 缓存事件数据大小 和 目标读取数量  两个值小的那个*/
    len = GET_MIN(len, CircularQueue->entrance - CircularQueue->exit);// 假设总大小10  写入了5 - 已读4  == 1 未读   要读5个   返回1
    /*原理雷同存入*/
    size = GET_MIN(len, CircularQueue->size - (CircularQueue->exit & (CircularQueue->size - 1)));//10 - 0 > 1 返回1
    memcpy(targetBuf, CircularQueue->dataBufer + (CircularQueue->exit & (CircularQueue->size - 1)), size);//偏移0个 复制一个字节
    memcpy(targetBuf + size, CircularQueue->dataBufer, len - size);// 存储区偏移0个字节
    /*利用无符号数据的溢出特性*/
//    CircularQueue->exit += len;//手动缩减

    return len;
}

/*手动缩减缓冲区*/
void CQ_read_offset_inc(CQ_handleTypeDef *CircularQueue, uint32_t len)
{
	CircularQueue->exit += len ;
}

/*分配一个缓冲区并进行初始化--替代--CQ_init
*
* 参数buffsize 申请的字节空间大小
*/
CQ_handleTypeDef *cb_create(uint32_t buffsize)
{
	CQ_handleTypeDef *cb = (CQ_handleTypeDef *)malloc(sizeof(CQ_handleTypeDef));//分配一个新的内存空间。cb_struct_pointer
	if (NULL == cb)
	{
		return NULL;
	}
	buffsize = (buffsize <= 2048 ? buffsize : 2048);
	cb->size = buffsize;
	cb->exit = 0;
	cb->entrance = 0;
	//the buff never release!
	cb->dataBufer = (uint8_t *)malloc(sizeof(uint8_t)*cb->size);
	return cb;
}
#ifdef __cplusplus //使用ｃ编译
}
#endif
