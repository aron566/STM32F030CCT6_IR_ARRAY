#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <stdint.h>
#include "GN_uart.h"

/* IWDG ----------------------------------------------------------------------*/
void IWDG_clr(void);

/* cpu id --------------------------------------------------------------------*/
void sys_get_CPU_ID(uint32_t*pUID);

/* LED -----------------------------------------------------------------------*/
void sys_LED_toggle();
void high_light_LED_on();
void high_light_LED_off();
void beep1_on();
void beep1_off();
void beep2_on();
void beep2_off();

/* LCD -----------------------------------------------------------------------*/

#define LCD_BACKLIGHT_0 0
#define LCD_BACKLIGHT_1 50
#define LCD_BACKLIGHT_2 200
#define LCD_BACKLIGHT_3 600
#define LCD_BACKLIGHT_4 1000

#define MAX_BACKLIGHT_PERCENT 1000U



void LCD_backlight_control(uint16_t percent);

/* time ----------------------------------------------------------------------*/
uint32_t get_time_ms();
uint32_t get_time_sec();

void UART_set_baudrate(UART_TTY_e tty, uint32_t baudrate);
int8_t tty_is_ready(  UART_TTY_e tty  );

#endif