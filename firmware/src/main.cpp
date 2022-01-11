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
#include "esp_log.h"
#include "esp_sleep.h"

#include "webapp.hpp"
#include "draw.hpp"
#include "inkplate.hpp"

static const char *TAG = "main";

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
  init_nvs();

  display.begin(true);
  display.clearDisplay();

  const auto wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER)
  {
    char ssid[16], password[16], ip_addr[16];

    init_ap(ssid, password, ip_addr);
    init_localdomain();

    start_web_server();
    draw_setup_info(ssid, password, ip_addr);

    uint8_t touched;
    for (;;)
    {
      for (uint8_t i = 0; i < 3; i++)
      {
        touched = display.readTouchpad(i);
        if (touched)
        {
          ESP_LOGI(TAG, "Detect touched touchpad [%d]", i);
          ESP_LOGI(TAG, "Entering deep sleep");
          esp_sleep_enable_timer_wakeup(1000000);
          esp_deep_sleep_start();
        }
      }
      ESP::delay(300);
    }
  }
  else
  {
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
  }
  for (;;)
  {
    vTaskDelay(1000000 / portTICK_PERIOD_MS);
  }
}

extern "C"
{
  void app_main()
  {
    xTaskCreatePinnedToCore(main_task, "main_task", 8192, nullptr, 1, nullptr, 1);
  }
}
