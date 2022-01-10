#include "lwip/inet.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "cJSON.h"

static esp_err_t system_info_get_handler(httpd_req_t *req)
{
  cJSON *system = cJSON_CreateObject();
  cJSON *display = cJSON_CreateObject();

  cJSON_AddStringToObject(system, "version", APP_VERSION);
#if defined(INKPLATE_10)
  cJSON_AddStringToObject(system, "model", "Inkplate 10");
  cJSON_AddNumberToObject(display, "width", 1200);
  cJSON_AddNumberToObject(display, "height", 825);
#elif defined(INKPLATE_6)
  cJSON_AddStringToObject(system, "model", "Inkplate 6");
  cJSON_AddNumberToObject(display, "width", 800);
  cJSON_AddNumberToObject(display, "height", 600);
#else
  cJSON_AddStringToObject(system, "model", "Unknown");
  cJSON_Delete(display);
  display = nullptr;
#endif

  uint8_t macaddr[6];
  char buff[18];
  esp_netif_ip_info_t ip_info;

  esp_read_mac(macaddr, ESP_MAC_WIFI_SOFTAP);
  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

  cJSON *network = cJSON_CreateObject();
  snprintf(buff, sizeof(buff), MACSTR, MAC2STR(macaddr));
  cJSON_AddStringToObject(network, "mac", buff);
  inet_ntoa_r(ip_info.ip.addr, buff, sizeof(buff));
  cJSON_AddStringToObject(network, "ipv4", buff);

  cJSON *root = cJSON_CreateObject();
  cJSON_AddItemToObject(root, "system", system);
  cJSON_AddItemToObject(root, "display", display);
  // FIXME: Add storage usage data
  cJSON_AddItemToObject(root, "storage", nullptr);
  cJSON_AddItemToObject(root, "network", network);
  const auto *sys_info = cJSON_Print(root);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, sys_info);

  free((void *)sys_info);
  cJSON_Delete(root);

  return ESP_OK;
}

httpd_uri_t system_info_get_uri = {
    .uri = "/api/v1/system/info",
    .method = HTTP_GET,
    .handler = system_info_get_handler,
    .user_ctx = nullptr,
};