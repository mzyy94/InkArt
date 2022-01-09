/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <dirent.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

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

static void init_localdomain(void)
{
  mdns_init();
  mdns_hostname_set("inkart");
  mdns_instance_name_set("InkArt web app");

  mdns_txt_item_t txt_items[] = {{"path", "/"}};

  ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, txt_items, sizeof(txt_items) / sizeof(txt_items[0])));

  netbiosns_init();
  netbiosns_set_name("inkart");
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_id == WIFI_EVENT_AP_STACONNECTED)
  {
    wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
  }
  else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
  {
    wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
  }
}

void init_ap()
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

  uint8_t macaddr[6];
  esp_read_mac(macaddr, ESP_MAC_WIFI_SOFTAP);

  wifi_config_t wifi_config = {};
  snprintf(reinterpret_cast<char *>(wifi_config.ap.ssid), 12, "InkArt%02x%02x", macaddr[2], macaddr[3]);
  snprintf(reinterpret_cast<char *>(wifi_config.ap.password), 12, "iNKaRT%02x%02x", macaddr[4], macaddr[5]);
  wifi_config.ap.channel = 13;
  wifi_config.ap.max_connection = 4;
  wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "SSID:%s password:%s", wifi_config.ap.ssid, wifi_config.ap.password);
}

extern "C"
{
  void app_main()
  {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    init_ap();
    init_localdomain();

    xTaskCreatePinnedToCore(main_task, "main_task", 8192, nullptr, 1, nullptr, 1);
  }
}
