/*==================[ Inclusiones ]============================================*/
#include "../include/RTC.h"


/*==================[Prototipos de funciones]======================*/
static esp_err_t i2c_master_init(void);
//static esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size);
//static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size);


/*==================[Implementaciones]=================================*/

static void RTC_init(){
    print_mux = xSemaphoreCreateMutex();
    ESP_ERROR_CHECK(i2c_master_init());
}



void RTC_read(uint8_t instruccion, uint8_t *variable){  //La primera variable indica la intruccion a leer. La segunda variable es la direccion de la variable donde se va a guardar el dato de retorno
    
    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, &instruccion, 1, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
         
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
            if (1 > 1) {
                i2c_master_read(cmd, variable, 1 - 1, ACK_VAL);
            }
            i2c_master_read_byte(cmd, variable + 1 - 1, NACK_VAL);
            i2c_master_stop(cmd);
            i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
            i2c_cmd_link_delete(cmd);
            
   }else{
         printf("Errro en RTC : %s!\n", esp_err_to_name(ret));
    }      
}

uint8_t dec2bcd(uint8_t num)
{
    uint8_t ones = 0;
    uint8_t tens = 0;
    uint8_t temp = 0;
    ones = num%10;
    temp = num/10;
    tens = temp<<4;
    return (tens + ones);
}

void RTC_setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t
                   dayOfMonth, uint8_t month, uint8_t year){

    uint8_t second_bcd=dec2bcd(second)&0b01111111; 
    uint8_t minute_bcd=dec2bcd(minute)&0b01111111;  
    uint8_t hour_bcd=dec2bcd(hour)&0b00111111;
    uint8_t dayOfWeek_bcd=dec2bcd(dayOfWeek)&0b00000111;
    uint8_t dayOfMonth_bcd=dec2bcd(dayOfMonth)&0b00111111;
    uint8_t month_bcd=dec2bcd(month)&0b00011111;
    uint8_t year_bcd=dec2bcd(year);
    
              
    int ret;
    uint8_t instruccion=0x00;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, &instruccion, 1, ACK_CHECK_EN);
    i2c_master_write(cmd, &second_bcd, 1, ACK_CHECK_EN);
    i2c_master_write(cmd, &minute_bcd, 1, ACK_CHECK_EN);
    i2c_master_write(cmd, &hour_bcd, 1, ACK_CHECK_EN);
    i2c_master_write(cmd, &dayOfWeek_bcd, 1, ACK_CHECK_EN);
    i2c_master_write(cmd, &dayOfMonth_bcd, 1, ACK_CHECK_EN);
    i2c_master_write(cmd, &month_bcd, 1, ACK_CHECK_EN);
    i2c_master_write(cmd, &year_bcd, 1, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        printf("Fecha cambiada\n");
    }else{
         printf("Errro en RTC : %s!\n", esp_err_to_name(ret));
     }                     

}




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
        //.clk_flags = 0,  /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    //Establece la configuración con los datos previamente cargados
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
    //si no hay error, instala el driver sin buffers porque es maestro y devuelve el valor de error correspondiente
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/*
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