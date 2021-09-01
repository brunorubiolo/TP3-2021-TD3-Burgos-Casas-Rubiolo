/*==================[ Inclusiones ]============================================*/
#include "../include/LCDI2C.h"
#include "../include/RTC.h"

/*==================[Prototipos de funciones]======================*/
static esp_err_t i2c_master_init(void);
//static esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size);
//static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size);


//=========================== Definiciones ================================
#define ESP_SLAVE_ADDR_LCD 0x27                /*!< ESP32 slave address, you can set any 7bit value */
#define NOP() asm volatile ("nop")

/*==================[Variables]======================*/
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; //Inicializa el spinlock desbloqueado

/*==================[Implementaciones]=================================*/


unsigned long IRAM_ATTR micros()
{
    return (unsigned long) (esp_timer_get_time());
}
void IRAM_ATTR delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}


static void I2C_init(){
    print_mux2 = xSemaphoreCreateMutex();
    ESP_ERROR_CHECK(i2c_master_init());
}


void Send_i2c(uint8_t dato){

    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR_LCD << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, &dato, sizeof(dato), ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
    }else{
         printf("Errro LCD_I2C : %s!\n", esp_err_to_name(ret));
     }                     

}

void lcdCommand(unsigned char cmnd){
    //P0=RS P1=RW  P2=EN   P3=luz  P4=D4  P5=D5  P6=D6  P7=D7 

    Send_i2c(((cmnd & 0xF0) & 0b11111100) | 0b00001100);//RS = 0 para colocar dato 
                                          //RW = 0 para colocar en modo escritura
                                          //EN = 1 pulso en alto y en bajo para validar
                                          //P3=1 luz encendida
    vTaskDelay(1 / portTICK_PERIOD_MS);                //espero para que se valide                               
    Send_i2c(((cmnd & 0xF0) & 0b11111000 ) | 0b00001000);           //EN = 0 pulso en alto y en bajo para validar dato
    vTaskDelay(1 / portTICK_PERIOD_MS);              //espero para que se valide

    Send_i2c(((cmnd <<4) & 0b11111100) | 0b00001100);//RS = 0 para colocar dato 
                                          //RW = 0 para colocar en modo escritura
                                          //EN = 1 pulso en alto y en bajo para validar
                                          //P3=1 luz encendida
    vTaskDelay(1 / portTICK_PERIOD_MS);                //espero para que se valide                               
    Send_i2c(((cmnd << 4) & 0b11111000 ) | 0b00001000);           //EN = 0 pulso en alto y en bajo para validar dato
    vTaskDelay(1 / portTICK_PERIOD_MS);              //espero para que se valide

}

void lcdData(unsigned char data)
{
    Send_i2c(((data & 0xF0) & 0b11111101) | 0b00001101);//RS = 1 para colocar dato 
                                          //RW = 0 para colocar en modo escritura
                                          //EN = 1 pulso en alto y en bajo para validar
    vTaskDelay(1 / portTICK_PERIOD_MS);                //espero para que se valide                               
    Send_i2c(((data & 0xF0) & 0b11111001) | 0b00001001);//EN = 0 pulso en alto y en bajo para validar dato
    vTaskDelay(1 / portTICK_PERIOD_MS);              //espero para que se valide

    Send_i2c(((data << 4) & 0b11111101) | 0b00001101);//RS = 1 para colocar dato 
                                          //RW = 0 para colocar en modo escritura
                                          //EN = 1 pulso en alto y en bajo para validar
    vTaskDelay(1 / portTICK_PERIOD_MS);                //espero para que se valide                               
    Send_i2c(((data << 4) & 0b11111001) | 0b00001001);//EN = 0 pulso en alto y en bajo para validar dato
    vTaskDelay(1 / portTICK_PERIOD_MS);              //espero para que se valide
}


void LCDI2C_init(){
    
    Send_i2c(0xFF); //Limpio la salida de P0 a P7 produciendo el apagado de enable
	vTaskDelay(30 / portTICK_PERIOD_MS);  // Espera para iniciar
    lcdCommand(0x02); //HOME 
    lcdCommand(0x28); //Inicia. LCD 2 lineas, 5x7 matriz, modo 4 bits
	lcdCommand(0x0C); //Enciende display
    //lcdCommand(0x0E); //Cursor on
    lcdCommand(0x01); //Limpia LCD
	vTaskDelay(2 / portTICK_PERIOD_MS); //Espera
	lcdCommand(0x06); //Mueve el cursor a la derecha
   
}

void lcd_gotoxy(unsigned char x, unsigned char y)
{
	unsigned char firstCharAdr[] ={ 0x80,0xC0,0x94,0xD4} ;//Comandos para poscicionarse Tabla 12-5
	lcdCommand (firstCharAdr[y-1] + x - 1);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

void lcd_print( char * str )
{
	unsigned char i = 0;
	while (str[i] !=0)
	{
		lcdData(str[i]) ;
		i++ ;
	}
}



/*
//i2c master initialization
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;       //Puerto I2C a utilizar - 0 o 1
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,                //Modo maestro
        .sda_io_num = I2C_MASTER_SDA_IO,        //Pin SDA
        .sda_pullup_en = GPIO_PULLUP_ENABLE,    //Habilita pull up en SDA
        .scl_io_num = I2C_MASTER_SCL_IO,        //Pin SCL
        .scl_pullup_en = GPIO_PULLUP_ENABLE,    //Habilita pull up en SCL
        .master.clk_speed = I2C_MASTER_FREQ_HZ, //Frecuencia de clock
        //.clk_flags = 0,  //!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. 
    };
    //Establece la configuración con los datos previamente cargados
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
    //si no hay error, instala el driver sin buffers porque es maestro y devuelve el valor de error correspondiente
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}


//The data will be stored in slave buffer. We can read them out from slave buffer.
static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

//We need to fill the buffer of esp slave device, then master can read them out.
static esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size)
{
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

*/