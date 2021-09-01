/*=============================================================================
 * Author: Fabian Burgos
 * Date: 07/04/2021 
 * Board: ESP32
 *===========================================================================*/

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "soc/soc.h" //disable brownout detector
#include "soc/rtc_cntl_reg.h" //disable brownout detector (deteccion de apagon)
#include "soc/rtc_wdt.h"
#include "../include/RTC.h"
#include "../include/Uart.h"
#include "../include/LCDI2C.h"
//=========================== Definiciones ================================

#define SEGUNDOS 0X00
#define MINUTOS 0X01
#define HORA 0X02
#define DIA_SEMANA 0X03  //Establece el dia de la semana del 1 al 7
#define DIA_MES 0X04     //Establece el numero del dia en el mes
#define MES 0X05
#define ANIO 0X06

//=========================== Función principal ================================
void app_main() {
	//Configuración
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    rtc_wdt_protect_off();
    rtc_wdt_disable();
	
	RTC_init();
    LCDI2C_init();
    config_Uart();
	
	
   //Bucle infinito 
    while(1) {
		//printf("Escribo en serial\n");
	    uint8_t Segundos;
        uint8_t Minutos;
        uint8_t Hora;
        uint8_t Dia;
        uint8_t Mes;
        uint8_t Anio;
        RTC_read(SEGUNDOS, &Segundos);
        RTC_read(MINUTOS, &Minutos);
        RTC_read(HORA, &Hora);
        RTC_read(DIA_MES, &Dia);
        RTC_read(MES, &Mes);
        RTC_read(ANIO, &Anio);
		printf("%02x/%02x/%02x - %02x:%02x:%02x\n", Dia, Mes, Anio, Hora, Minutos, Segundos);
        char fila1[16];
        char fila2[16];
        sprintf( fila1, "    %02x/%02x/%02x", Dia, Mes, Anio);
        sprintf( fila2, "    %02x:%02x:%02x", Hora, Minutos, Segundos);
        lcd_gotoxy(1,1);
        lcd_print(fila1);
        lcd_gotoxy(1,2);
        lcd_print(fila2);
        vTaskDelay(1000 / portTICK_PERIOD_MS); //Espero 1000 milisegundo
		
    }

}

