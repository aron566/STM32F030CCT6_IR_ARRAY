

/* Includes ------------------------------------------------------------------*/
#include "peripheral.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include "cmsis_os.h"
#include "iwdg.h"
//#include "tim.h"
#include "usart.h"


/*CPU_ID----------------------------------------------------------------------*/
void sys_get_CPU_ID(uint32_t*pUID)
{   
    pUID[0] = HAL_GetUIDw0();
    pUID[1] = HAL_GetUIDw1();
    pUID[2] = HAL_GetUIDw2();
}

/* IWDG ----------------------------------------------------------------------*/
extern IWDG_HandleTypeDef hiwdg;
void IWDG_clr(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}

/* LED -----------------------------------------------------------------------*/
void sys_LED_toggle()
{
//    HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}
void high_light_LED_on()
{
//    HAL_GPIO_WritePin(HIGH_LIGHT_LED_GPIO_Port, HIGH_LIGHT_LED_Pin, GPIO_PIN_SET);
}
void high_light_LED_off()
{
//    HAL_GPIO_WritePin(HIGH_LIGHT_LED_GPIO_Port, HIGH_LIGHT_LED_Pin, GPIO_PIN_RESET);
}

/* BEEP ----------------------------------------------------------------------*/
void beep1_on()
{
//    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
}
void beep1_off()
{
//    HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);//关闭PWM输出
}
void beep2_on()
{
//    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
}
void beep2_off()
{
//    HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_2);//关闭PWM输出
}




//LCD背光调节。percent:0~1000
void LCD_backlight_control(uint16_t percent)
{
    if(percent >= MAX_BACKLIGHT_PERCENT)
    {
        percent = MAX_BACKLIGHT_PERCENT;
    }
//    __HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_3,htim2.Init.Period * percent/MAX_BACKLIGHT_PERCENT); //PWM输出占空比100% == 1000  TFT  
}

/* UART ----------------------------------------------------------------------*/

static UART_HandleTypeDef *get_huart_by_tty( UART_TTY_e tty )
{
    UART_HandleTypeDef *pHuart = 0;
    
    switch (tty)
    {
    case UART_TTY0:
        pHuart = &huart1;
        break;
    case UART_TTY1:
        pHuart = &huart2;
        break;
    case UART_TTY2:
        pHuart = &huart3;
        break;
    case UART_TTY3:
        pHuart = &huart4;
        break;
    case UART_TTY4:
        pHuart = &huart5;
        break;
    case UART_TTY5:
        pHuart = &huart6;
        break;
    default:
        break;
    }
    return pHuart;
}
void UART_set_baudrate(UART_TTY_e tty, uint32_t baudrate)
{
    UART_HandleTypeDef *pHuart = get_huart_by_tty( tty );
    if ( 0 == pHuart )
    {
        return;
    }
    if ( baudrate != pHuart->Init.BaudRate )
    {
        HAL_UART_Abort(pHuart); //先停止再设置。如果直接设置，串口又正好在发送中断，会出现中断死循环
        pHuart->Init.BaudRate = baudrate;
        HAL_UART_Init(pHuart);
    }
}

static HAL_UART_StateTypeDef get_uart_gState( UART_TTY_e tty )
{
    UART_HandleTypeDef *pHuart = get_huart_by_tty( tty );
    if ( 0 == pHuart )
    {
        return HAL_UART_STATE_RESET;
    }
    return pHuart->gState;
}

int8_t tty_is_ready(  UART_TTY_e tty  )
{
    if ( HAL_UART_STATE_READY == get_uart_gState( tty ) )
    {
        return 1;
    }
    return 0;
}



