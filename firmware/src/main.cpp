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
RTC_DATA_ATTR int last_index = -1;

void readbmps(const std::string &dirname, std::vector<std::string> &output)
{
  DIR *dir = opendir(dirname.c_str());
  struct dirent *ent;
  while ((ent = readdir(dir)) != nullptr)
  {
    if (ent->d_type == DT_REG)
    {
      std::string name = ent->d_name;
      if (name.substr(name.find_last_of(".") + 1) == "bmp")
        output.push_back(dirname + name);
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
    std::vector<std::string> bmp_images;

    readbmps("/sdcard/", bmp_images);

    if (bmp_images.size() > 0)
    {
      last_index++;
      ESP_LOGI(TAG, "Search bmp image at index %d in SD", last_index);
      auto iter = bmp_images.begin();
      for (size_t i = 0; i < last_index; i++)
      {
        iter++;
        if (iter == bmp_images.end())
        {
          iter = bmp_images.begin();
          last_index = 0;
          break;
        }
      }

      ESP_LOGI(TAG, "Display bmp image: %s", iter->c_str());
      display.drawImage(iter->c_str(), 0, 0);
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

  ESP::delay(1000);
  ESP_LOGI(TAG, "Entering deep sleep");
  esp_sleep_enable_timer_wakeup(30 * 60 * 1000000);
  esp_deep_sleep_start();
}

extern "C"
{
  void app_main()
  {
    xTaskCreatePinnedToCore(main_task, "main_task", 8192, nullptr, 1, nullptr, 1);
  }
}
