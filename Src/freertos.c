/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId RTCHandle;
uint32_t RTCTaskBuffer[ 128 ];
osStaticThreadDef_t RTCTaskControlBlock;
osThreadId IR_Task_1Handle;
uint32_t IR_Task_1Buffer[ 128 ];
osStaticThreadDef_t IR_Task_1ControlBlock;
osThreadId IR_Task_2Handle;
uint32_t IR_Task_2Buffer[ 128 ];
osStaticThreadDef_t IR_Task_2ControlBlock;
osThreadId IR_Task_3TaskHandle;
uint32_t IR_Task_3TaskBuffer[ 128 ];
osStaticThreadDef_t IR_Task_3TaskControlBlock;
osThreadId IR_Task_4TaskHandle;
uint32_t IR_Task_4TaskBuffer[ 128 ];
osStaticThreadDef_t IR_Task_4Task1TaskControlBlock;
osThreadId PcConectTaskHandle;
uint32_t PcConectTaskBuffer[ 128 ];
osStaticThreadDef_t PcConectTaskControlBlock;
osThreadId DisplayTaskHandle;
uint32_t DisplayTaskBuffer[ 128 ];
osStaticThreadDef_t DisplayTaskControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartRTCTask(void const * argument);
void StartIR_Task_1Task(void const * argument);
void StartIR_Task_2Task(void const * argument);
void StartIR_Task_3Task(void const * argument);
void StartIR_Task_4Task(void const * argument);
void StartPcConectTask(void const * argument);
void StartDisplayTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of RTC */
  osThreadStaticDef(RTC, StartRTCTask, osPriorityIdle, 0, 128, RTCTaskBuffer, &RTCTaskControlBlock);
  RTCHandle = osThreadCreate(osThread(RTC), NULL);

  /* definition and creation of IR_Task_1 */
  osThreadStaticDef(IR_Task_1, StartIR_Task_1Task, osPriorityNormal, 0, 128, IR_Task_1Buffer, &IR_Task_1ControlBlock);
  IR_Task_1Handle = osThreadCreate(osThread(IR_Task_1), NULL);

  /* definition and creation of IR_Task_2 */
  osThreadStaticDef(IR_Task_2, StartIR_Task_2Task, osPriorityNormal, 0, 128, IR_Task_2Buffer, &IR_Task_2ControlBlock);
  IR_Task_2Handle = osThreadCreate(osThread(IR_Task_2), NULL);

  /* definition and creation of IR_Task_3Task */
  osThreadStaticDef(IR_Task_3Task, StartIR_Task_3Task, osPriorityNormal, 0, 128, IR_Task_3TaskBuffer, &IR_Task_3TaskControlBlock);
  IR_Task_3TaskHandle = osThreadCreate(osThread(IR_Task_3Task), NULL);

  /* definition and creation of IR_Task_4Task */
  osThreadStaticDef(IR_Task_4Task, StartIR_Task_4Task, osPriorityNormal, 0, 128, IR_Task_4TaskBuffer, &IR_Task_4Task1TaskControlBlock);
  IR_Task_4TaskHandle = osThreadCreate(osThread(IR_Task_4Task), NULL);

  /* definition and creation of PcConectTask */
  osThreadStaticDef(PcConectTask, StartPcConectTask, osPriorityNormal, 0, 128, PcConectTaskBuffer, &PcConectTaskControlBlock);
  PcConectTaskHandle = osThreadCreate(osThread(PcConectTask), NULL);

  /* definition and creation of DisplayTask */
  osThreadStaticDef(DisplayTask, StartDisplayTask, osPriorityLow, 0, 128, DisplayTaskBuffer, &DisplayTaskControlBlock);
  DisplayTaskHandle = osThreadCreate(osThread(DisplayTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartRTCTask */
/**
  * @brief  Function implementing the RTC thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartRTCTask */
void StartRTCTask(void const * argument)
{
  /* USER CODE BEGIN StartRTCTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartRTCTask */
}

/* USER CODE BEGIN Header_StartIR_Task_1Task */
/**
* @brief Function implementing the IR_Task_1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartIR_Task_1Task */
void StartIR_Task_1Task(void const * argument)
{
  /* USER CODE BEGIN StartIR_Task_1Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartIR_Task_1Task */
}

/* USER CODE BEGIN Header_StartIR_Task_2Task */
/**
* @brief Function implementing the IR_Task_2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartIR_Task_2Task */
void StartIR_Task_2Task(void const * argument)
{
  /* USER CODE BEGIN StartIR_Task_2Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartIR_Task_2Task */
}

/* USER CODE BEGIN Header_StartIR_Task_3Task */
/**
* @brief Function implementing the IR_Task_3Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartIR_Task_3Task */
void StartIR_Task_3Task(void const * argument)
{
  /* USER CODE BEGIN StartIR_Task_3Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartIR_Task_3Task */
}

/* USER CODE BEGIN Header_StartIR_Task_4Task */
/**
* @brief Function implementing the IR_Task_4Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartIR_Task_4Task */
void StartIR_Task_4Task(void const * argument)
{
  /* USER CODE BEGIN StartIR_Task_4Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartIR_Task_4Task */
}

/* USER CODE BEGIN Header_StartPcConectTask */
/**
* @brief Function implementing the PcConectTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPcConectTask */
void StartPcConectTask(void const * argument)
{
  /* USER CODE BEGIN StartPcConectTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartPcConectTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void const * argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDisplayTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
