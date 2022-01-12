#include "lwip/inet.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

static esp_err_t system_info_get_handler(httpd_req_t *req)
{
  json j;
  j["system"]["version"] = APP_VERSION;

#if defined(INKPLATE_10)
  j["system"]["model"] = "Inkplate 10";
  j["display"]["width"] = 1200;
  j["display"]["height"] = 825;
#elif defined(INKPLATE_6)
  j["system"]["model"] = "Inkplate 6";
  j["display"]["width"] = 800;
  j["display"]["height"] = 600;
#else
  j["system"]["model"] = "Unknown";
#endif

  uint8_t macaddr[6];
  char buff[18];
  esp_netif_ip_info_t ip_info;

  esp_read_mac(macaddr, ESP_MAC_WIFI_SOFTAP);
  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

  snprintf(buff, sizeof(buff), MACSTR, MAC2STR(macaddr));
  j["network"]["mac"] = buff;
  inet_ntoa_r(ip_info.ip.addr, buff, sizeof(buff));
  j["network"]["ipv4"] = buff;

  // FIXME: Add storage usage data
  j["storage"] = nullptr;

  const std::string str = j.dump(4);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t system_info_get_uri = {
    .uri = "/api/v1/system/info",
    .method = HTTP_GET,
    .handler = system_info_get_handler,
    .user_ctx = nullptr,
};