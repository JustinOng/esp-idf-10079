#include <stdio.h>
#include <string.h>

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

void task1() {
  while (1) {
    uint8_t i = 0x01;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
  }
}

void task2() {
  while (1) {
    uint32_t crashme;
    esp_flash_read(NULL, (void *)&crashme, 0, sizeof(crashme));
  }
}

void app_main(void) {
#ifdef CONFIG_FREERTOS_PLACE_FUNCTIONS_INTO_FLASH
  ESP_LOGW("main", "CONFIG_FREERTOS_PLACE_FUNCTIONS_INTO_FLASH enabled");
#endif

  i2c_config_t conf;
  memset(&conf, 0, sizeof(conf));
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = 4;
  conf.scl_io_num = 5;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = 100000;

  i2c_param_config(I2C_NUM_0, &conf);
  i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, ESP_INTR_FLAG_SHARED | ESP_INTR_FLAG_IRAM);

  xTaskCreatePinnedToCore(task1, "task1", 8192, NULL, 10, NULL, 0);
  xTaskCreatePinnedToCore(task2, "task2", 8192, NULL, 10, NULL, 1);
}
