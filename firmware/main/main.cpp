/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

void main_task(void *)
{
  std::cout << "Hello world!" << std::endl;

  /* Print chip information */
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  std::cout << "This is ESP32 chip with " << std::dec << +chip_info.cores
            << " CPU cores, WiFi" << ((chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "")
            << ((chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "") << ", ";

  std::cout << "silicon revision " << std::dec << +chip_info.revision << ", ";

  std::cout << spi_flash_get_chip_size() / (1024 * 1024) << "MB "
            << ((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external") << " flash" << std::endl;

  for (int i = 10; i >= 0; i--)
  {
    std::cout << "Restarting in " << i << " seconds..." << std::endl;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  std::cout << "Restarting now." << std::endl;
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

extern "C"
{
  void app_main()
  {
    xTaskCreatePinnedToCore(main_task, "main_task", 8192, nullptr, 1, nullptr, 1);
  }
}
