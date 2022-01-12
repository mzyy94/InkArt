#include <string>
#include <vector>
#include <algorithm>
#include "lwip/inet.h"
#include "nvs_flash.h"
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

std::vector<std::string> orientations = {
    "landscape",
    "portrait-left",
    "upside-down",
    "portrait-right",
};

static esp_err_t system_display_get_handler(httpd_req_t *req)
{
  json j;

  nvs_handle_t handle;
  nvs_open("system_settings", NVS_READONLY, &handle);

  uint8_t val = 0;
  nvs_get_u8(handle, "inverted", &val);
  j["inverted"] = bool(val);
  nvs_get_u8(handle, "orientation", &val);
  j["orientation"] = orientations[val];

  int16_t val1 = 0;
  nvs_get_i16(handle, "padding-top", &val1);
  j["padding"]["top"] = val1;
  nvs_get_i16(handle, "padding-left", &val1);
  j["padding"]["left"] = val1;
  nvs_get_i16(handle, "padding-right", &val1);
  j["padding"]["right"] = val1;
  nvs_get_i16(handle, "padding-bottom", &val1);
  j["padding"]["bottom"] = val1;

  nvs_close(handle);

  const std::string str = j.dump(4);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t system_display_get_uri = {
    .uri = "/api/v1/system/display",
    .method = HTTP_GET,
    .handler = system_display_get_handler,
    .user_ctx = nullptr,
};

static esp_err_t system_display_post_handler(httpd_req_t *req)
{
  char buff[256];
  if (req->content_len >= sizeof(buff))
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Content too long");
    return ESP_FAIL;
  }

  auto len = httpd_req_recv(req, buff, sizeof(buff));
  if (len <= 0)
  {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read content");
    return ESP_FAIL;
  }

  buff[len] = '\0';
  json j;
  try
  {
    j = json::parse(buff);
  }
  catch (json::parse_error &e)
  {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse content");
    return ESP_FAIL;
  }

  nvs_handle_t handle;
  nvs_open("system_settings", NVS_READWRITE, &handle);

  uint8_t val;
  if (j.contains("inverted"))
  {
    val = j["inverted"];
    nvs_set_u8(handle, "inverted", val);
  }
  if (j.contains("orientation"))
  {
    std::string orientation = j["orientation"];
    auto iter = std::find(orientations.begin(), orientations.end(), orientation);
    val = std::distance(orientations.begin(), iter);
    nvs_set_u8(handle, "orientation", val);
  }
  if (j.contains("padding"))
  {
    int16_t val1;
    val1 = j["padding"]["top"];
    nvs_set_i16(handle, "padding-top", val1);
    val1 = j["padding"]["left"];
    nvs_set_i16(handle, "padding-left", val1);
    val1 = j["padding"]["right"];
    nvs_set_i16(handle, "padding-right", val1);
    val1 = j["padding"]["bottom"];
    nvs_set_i16(handle, "padding-bottom", val1);
  }
  nvs_commit(handle);
  nvs_close(handle);

  json ok;
  ok["status"] = "ok";
  const std::string str = ok.dump(4);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t system_display_post_uri = {
    .uri = "/api/v1/system/display",
    .method = HTTP_POST,
    .handler = system_display_post_handler,
    .user_ctx = nullptr,
};