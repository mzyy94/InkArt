#include <string>
#include <vector>
#include <algorithm>
#include "nvs_flash.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include "lwip/inet.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "api.hpp"

static const char *TAG = "webapp";

void init_nvs()
{
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void init_localdomain(void)
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

void init_ap(char *ssid, char *password, char *ip_addr)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

  uint8_t macaddr[6];
  esp_read_mac(macaddr, ESP_MAC_WIFI_SOFTAP);
  snprintf(ssid, 12, "InkArt%02x%02x", macaddr[2], macaddr[3]);
  snprintf(password, 12, "iNKaRT%02x%02x", macaddr[4], macaddr[5]);

  wifi_config_t wifi_config = {};
  strncpy(reinterpret_cast<char *>(wifi_config.ap.ssid), ssid, 12);
  strncpy(reinterpret_cast<char *>(wifi_config.ap.password), password, 12);
  wifi_config.ap.channel = 13;
  wifi_config.ap.max_connection = 4;
  wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

  inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);

  ESP_LOGI(TAG, "IP: %s SSID:%s password:%s", ip_addr, wifi_config.ap.ssid, wifi_config.ap.password);
}

void start_web_server()
{
  httpd_handle_t server = nullptr;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_open_sockets = 7;
  config.lru_purge_enable = true;
  config.uri_match_fn = httpd_uri_match_wildcard;

  ESP_LOGI(TAG, "Starting HTTP Server");

  if (httpd_start(&server, &config) != ESP_OK)
  {
    ESP_LOGE(TAG, "Start server failed");
    return;
  };

  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &system_info_get_uri));
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &system_display_get_uri));

  return;
}
