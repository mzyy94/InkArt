#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "nvs_flash.h"

#include "webapp.hpp"
#include "draw.hpp"
#include "files.hpp"
#include "inkplate.hpp"

static const char *TAG = "main";
static const int16_t nvs_version = 1;

Inkplate display(DisplayMode::INKPLATE_3BIT);
RTC_DATA_ATTR int last_index = -1;

void init_nvs()
{
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  nvs_handle_t handle;
  if (nvs_open("system_settings", NVS_READONLY, &handle) == ESP_ERR_NVS_NOT_FOUND)
  {
    ESP_ERROR_CHECK(nvs_open("system_settings", NVS_READWRITE, &handle));
    nvs_set_i16(handle, "version", nvs_version);
    nvs_commit(handle);
  }
  nvs_close(handle);
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

    uint8_t touched, count;
    for (;;)
    {
      count = 0;
      for (uint8_t i = 0; i < 3; i++)
      {
        touched = display.readTouchpad(i);
        if (touched)
        {
          ESP_LOGD(TAG, "Detect touched touchpad [%d]", i);
          count++;
        }
      }
      if (count >= 3)
      {
        ESP_LOGI(TAG, "Entering deep sleep");
        esp_sleep_enable_timer_wakeup(1000000);
        esp_deep_sleep_start();
      }
      ESP::delay(300);
    }
  }

  int16_t x, y;
  uint8_t invert, rotation;
  uint16_t interval;

  nvs_handle_t handle;
  nvs_open("system_settings", NVS_READONLY, &handle);
  nvs_get_u8(handle, "inverted", &invert);
  nvs_get_u8(handle, "orientation", &rotation);
  nvs_get_i16(handle, "padding-top", &y);
  nvs_get_i16(handle, "padding-left", &x);
  nvs_get_u16(handle, "refresh", &interval);
  nvs_close(handle);

  std::vector<std::string> bmp_images;
  std::vector<std::string> available;

  readbmps("/sdcard/", bmp_images);
  std::copy_if(bmp_images.begin(), bmp_images.end(), std::back_inserter(available), [](std::string bmp)
               { return bmp[0] != '.'; });

  if (available.size() > 0)
  {
    last_index++;
    ESP_LOGI(TAG, "Search bmp image at index %d in SD", last_index);
    auto iter = available.begin();
    for (size_t i = 0; i < last_index; i++)
    {
      iter++;
      if (iter == available.end())
      {
        iter = available.begin();
        last_index = 0;
        break;
      }
    }

    ESP_LOGI(TAG, "Display bmp image: %s", iter->c_str());
    std::string filepath = "/sdcard/" + *iter;
    display.setRotation(rotation);
    display.drawImage(filepath.c_str(), x, y, false, invert);
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

  ESP::delay(1000);
  ESP_LOGI(TAG, "Entering deep sleep. Wake up after %d min.", interval > 0 ? interval : 30);
  esp_sleep_enable_timer_wakeup((uint64_t)(interval > 0 ? interval : 30) * 60 * 1000000);
  esp_deep_sleep_start();
}

extern "C"
{
  void app_main()
  {
    xTaskCreatePinnedToCore(main_task, "main_task", 8192, nullptr, 1, nullptr, 1);
  }
}
