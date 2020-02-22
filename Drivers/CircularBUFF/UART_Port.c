/*
 *  FILE: UART_Port.c
 *
 *  Created on: 2020/2/22
 *
 *         Author: aron66
 *
 *  DESCRIPTION:--
 */
#ifdef __cplusplus //use C compiler
extern "C" {
#endif
#include "UART_Port.h"/*�ⲿ�ӿ�*/
    
/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6; 
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern DMA_HandleTypeDef hdma_usart4_rx;
extern DMA_HandleTypeDef hdma_usart4_tx;
extern DMA_HandleTypeDef hdma_usart5_rx;
extern DMA_HandleTypeDef hdma_usart5_tx;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;

static uint8_t get_uart_index(USART_TypeDef *Instance);
static Uart_Dev_info_t *Create_Uart_Dev(Uart_num_t uart_num ,UART_HandleTypeDef *huart ,DMA_HandleTypeDef *hdma_rx ,uint16_t rx_temp_size ,uint32_t rxsize ,int work_mode ,osSemaphoreId *pRX_Sem);

/*Ԥ���崮���豸��Ϣ*/
Uart_Dev_info_t *Uart_pDevice[UART_MAX_NUM+1];

void Uart_Port_Init(void)
{
    Create_Uart_Dev(UART1 ,&huart1 ,&hdma_usart1_rx ,128 ,256 ,0 ,NULL);
    Create_Uart_Dev(UART2 ,&huart2 ,&hdma_usart2_rx ,128 ,256 ,0 ,NULL);
    Create_Uart_Dev(UART3 ,&huart3 ,&hdma_usart3_rx ,128 ,256 ,0 ,NULL);
    Create_Uart_Dev(UART4 ,&huart4 ,&hdma_usart4_rx ,128 ,256 ,0 ,NULL);
}
/*
 * ���������豸
 *
 * ���������ں� �����豸ָ�� dma������ַ ����ʱ�����С ���ն��д�С ����ģʽ ��ֵ�ź���
 */
static Uart_Dev_info_t *Create_Uart_Dev(Uart_num_t uart_num ,UART_HandleTypeDef *huart ,DMA_HandleTypeDef *hdma_rx ,uint16_t rx_temp_size ,uint32_t rxsize ,int work_mode ,osSemaphoreId *pRX_Sem)
{
    Uart_Dev_info_t *pUart_Dev = (Uart_Dev_info_t *)malloc(sizeof(Uart_Dev_info_t));
    pUart_Dev->phuart = huart;
    pUart_Dev->phdma_rx = hdma_rx;
    pUart_Dev->cb = cb_create(rxsize);
    pUart_Dev->MAX_RX_Temp = rx_temp_size;
    pUart_Dev->RX_Buff_Temp = (uint8_t *)malloc(sizeof(uint8_t)*rx_temp_size);
    pUart_Dev->Is_Half_Duplex = work_mode;
    pUart_Dev->pRX_Sem = pRX_Sem;
    Uart_pDevice[uart_num] = pUart_Dev;
    //�򿪿����ж�
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    //ʹ��DMA����
    HAL_UART_Receive_DMA(huart, pUart_Dev->cb->dataBufer, rxsize);
    return pUart_Dev;
}


/**
  * @brief  Rx Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t index = get_uart_index(huart->Instance);
    if(index != 0)
    {
        if((__HAL_UART_GET_FLAG(Uart_pDevice[index]->phuart ,UART_FLAG_IDLE) != RESET))
        {
          __HAL_UART_CLEAR_IDLEFLAG(Uart_pDevice[index]->phuart);                       //��������жϱ�־�������һֱ���Ͻ����жϣ�
          HAL_UART_DMAStop(Uart_pDevice[index]->phuart);
          uint32_t data_length  = Uart_pDevice[index]->MAX_RX_Temp - __HAL_DMA_GET_COUNTER(Uart_pDevice[index]->phdma_rx);
          CQ_putData(Uart_pDevice[index]->cb, Uart_pDevice[index]->RX_Buff_Temp, (uint32_t) data_length);//�����ݼ�¼��������
          memset(Uart_pDevice[index]->RX_Buff_Temp,0,data_length);
          data_length = 0;
          __HAL_UART_ENABLE_IT(Uart_pDevice[index]->phuart ,UART_IT_IDLE);              //�򿪿����ж�---
          HAL_UART_Receive_DMA(Uart_pDevice[index]->phuart ,Uart_pDevice[index]->RX_Buff_Temp, Uart_pDevice[index]->MAX_RX_Temp);  //������ʼDMA���� ÿ��64�ֽ�����
        }
    }
}   

/*��õ�ǰ������Ϣ����*/
static uint8_t get_uart_index(USART_TypeDef *Instance)
{
    uint8_t index = 0;
    for(;index < UART_MAX_NUM+1;index++)
    {
        if(Uart_pDevice[index]->phuart->Instance == Instance)
        {
            return index;
        }  
    }
    return 0;
}

/*************************************************
  * ��������: �ض���c�⺯��printf��DEBUG_USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
int fputc(int ch,FILE *f)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 10);//ԭ��ʹ������ʽ����
  return ch;
}
/**
  * ��������: �ض���c�⺯��getchar,scanf��DEBUG_USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
int fgetc(FILE * f)
{
  uint8_t ch = 0;
  while(HAL_UART_Receive(&huart2,&ch, 1, 0xffff)!=HAL_OK);
  return ch;
}
#ifdef __cplusplus //end extern c
}
#endif




