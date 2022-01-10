#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <dirent.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "webapp.hpp"

static const char *TAG = "main";

#include "inkplate.hpp"

Inkplate display(DisplayMode::INKPLATE_3BIT);

void readfiles(const std::string &dirname, std::vector<std::string> &output)
{
  DIR *dir = opendir(dirname.c_str());
  struct dirent *ent;
  while ((ent = readdir(dir)) != nullptr)
  {
    if (ent->d_type == DT_REG)
    {
      output.push_back(ent->d_name);
    }
  }
  closedir(dir);
}

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

  display.begin(true);
  display.clearDisplay();

  std::vector<std::string> file_names;
  std::vector<std::string> bmp_images;

  readfiles("/sdcard/", file_names);

  std::copy_if(file_names.begin(), file_names.end(), std::back_inserter(bmp_images), [](std::string s)
               { return s.substr(s.find_last_of(".") + 1) == "bmp"; });

  if (bmp_images.size() > 0)
  {
    display.drawImage(bmp_images[0].c_str(), 0, 0);
  }
  else
  {
    const auto width = display.width() / 8;
    for (size_t i = 0; i < 8; i++)
    {
      display.fillRect(width * i, 0, width, display.height(), i);
    }
  }

  display.display();

  for (;;)
  {
    vTaskDelay(1000000 / portTICK_PERIOD_MS);
  }
  std::cout << "Restarting now." << std::endl;
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

extern "C"
{
  void app_main()
  {
    init_nvs();

    init_ap();
    init_localdomain();

    start_web_server();

    xTaskCreatePinnedToCore(main_task, "main_task", 8192, nullptr, 1, nullptr, 1);
  }
}
