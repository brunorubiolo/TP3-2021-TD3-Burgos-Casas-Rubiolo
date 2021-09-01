#ifndef LCDI2C_H_
#define LCDI2C_H_
/*==================[ Inclusiones ]============================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_log.h"



/*==================[Prototipos de funciones]======================*/
//static void I2C_init(void);
void RTC_read(uint8_t instruccion, uint8_t *variable);
void RTC_setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t
                   dayOfMonth, uint8_t month, uint8_t year);
void LCDI2C_init(void);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_print( char * str );
//Byte bcdToDec(Byte val);

//=========================== Variables ================================
SemaphoreHandle_t print_mux2 = NULL;

#include "../main/LCDI2C.c"
#endif