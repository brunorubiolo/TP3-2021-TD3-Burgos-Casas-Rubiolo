/*=============================================================================
 * Author: Fabian Burgos, Bruno Rubiolo, Alejo Casas
 * Date: 01/09/2021 
 * Board: ESP32
 * Entorno de programacion: idf original sin Platformio
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
#include "../include/I2C.h"
#include "../include/RTC.h"
#include "../include/Uart.h"
#include "../include/LCDI2C.h"

//=========================== Definiciones ================================

//Posicion del registro interno del RTC donde se encuentra cada uno de los siguiente valores (VER DATASHEET DS3231)
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
    rtc_wdt_protect_off(); //Desabilito el "perro guardian"
    rtc_wdt_disable();     //Desabilito el "perro guardian"
	
	I2C_init();     //Inicializo el i2c
    LCDI2C_init(); //Inicializo el LCD
    config_Uart(); //Configuro el uart para recibir dato y cambiar los valores de fecha y hora
	
	
   //Bucle infinito 
    while(1) {

        //obtengo datos desde el RTC
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

        //Imprimo los resultados en el monitor serial
		printf("%02x/%02x/%02x - %02x:%02x:%02x\n", Dia, Mes, Anio, Hora, Minutos, Segundos);

        //Imprimo los resultados en el display lcd 16x2 i2c
        char fila1[16]; //arreglo de caracteres a mostrar en fila 1
        char fila2[16]; //arreglo de caracteres a mostrar en fila 2
        sprintf( fila1, "    %02x/%02x/%02x", Dia, Mes, Anio); //Escribo arreglo de fila 1
        sprintf( fila2, "    %02x:%02x:%02x", Hora, Minutos, Segundos); //Escribo arreglo de fila 2

        lcd_gotoxy(1,1);  //Posiciono el cursor en el caracter 1-1
        lcd_print(fila1); //Imprimo fila 1 en display
        lcd_gotoxy(1,2); //Posiciono el cursor en el caracter 1-2
        lcd_print(fila2); //Imprimo fila 1 en display
        vTaskDelay(1000 / portTICK_PERIOD_MS); //Espero 1000 milisegundo
		
    }

}

