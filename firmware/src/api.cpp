#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include "lwip/inet.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"

#include "files.hpp"

#include "nlohmann/json.hpp"

using nlohmann::json;

static const char *TAG = "api";

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
  char buff[128];
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
  json j = json::parse(buff, nullptr, false);
  if (j.is_discarded())
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

static esp_err_t system_time_get_handler(httpd_req_t *req)
{
  json j;

  struct timeval tv_now;
  gettimeofday(&tv_now, nullptr);
  uint64_t time_ms = tv_now.tv_sec * 1000LL + tv_now.tv_usec / 1000;

  j["time"] = time_ms;

  nvs_handle_t handle;
  nvs_open("system_settings", NVS_READONLY, &handle);

  uint16_t val = 0;
  nvs_get_u16(handle, "refresh", &val);
  j["refresh"] = val;

  nvs_close(handle);

  const std::string str = j.dump(4);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t system_time_get_uri = {
    .uri = "/api/v1/system/time",
    .method = HTTP_GET,
    .handler = system_time_get_handler,
    .user_ctx = nullptr,
};

static esp_err_t system_time_post_handler(httpd_req_t *req)
{
  char buff[128];
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
  json j = json::parse(buff, nullptr, false);
  if (j.is_discarded())
  {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse content");
    return ESP_FAIL;
  }

  if (j.contains("time"))
  {
    uint64_t time_ms = j["time"];
    struct timeval tv_now;
    tv_now.tv_sec = time_ms / 1000;
    tv_now.tv_usec = (time_ms % 1000) * 1000;
    settimeofday(&tv_now, nullptr);
  }

  if (j.contains("refresh"))
  {
    uint16_t val = j["refresh"];
    nvs_handle_t handle;
    nvs_open("system_settings", NVS_READWRITE, &handle);
    nvs_set_u16(handle, "refresh", val);
    nvs_commit(handle);
    nvs_close(handle);
  }

  json ok;
  ok["status"] = "ok";
  const std::string str = ok.dump(4);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t system_time_post_uri = {
    .uri = "/api/v1/system/time",
    .method = HTTP_POST,
    .handler = system_time_post_handler,
    .user_ctx = nullptr,
};

static esp_err_t photo_list_get_handler(httpd_req_t *req)
{
  json j;
  j["data"] = json::array();

  std::vector<std::string> bmp_images;
  readbmps("/sdcard/", bmp_images);

  for (const auto &img : bmp_images)
  {
    json ent;
    if (img[0] == '.')
    {
      ent["filename"] = img.substr(1);
      ent["hidden"] = true;
    }
    else
    {
      ent["filename"] = img;
      ent["hidden"] = false;
    }
    j["data"].push_back(ent);
  }

  const std::string str = j.dump(4);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t photo_list_get_uri = {
    .uri = "/api/v1/photos",
    .method = HTTP_GET,
    .handler = photo_list_get_handler,
    .user_ctx = nullptr,
};

static esp_err_t photo_binary_get_handler(httpd_req_t *req)
{
  std::string uri = req->uri;
  const auto filename = uri.substr(uri.find_last_of("/") + 1);

  std::ifstream ifs("/sdcard/." + filename, std::ios::in | std::ios::binary);
  if (!ifs)
  {
    ifs.open("/sdcard/" + filename, std::ios::in | std::ios::binary);
  }

  if (!ifs)
  {
    ESP_LOGE(TAG, "Image not found: %s", filename.c_str());
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not found.");
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/bmp");

  char buff[1024];
  ssize_t read_bytes;

  do
  {
    read_bytes = ifs.readsome(buff, sizeof(buff));
    if (read_bytes == -1)
    {
      ESP_LOGE(TAG, "Failed to read image: %s", filename.c_str());
    }
    else if (read_bytes > 0)
    {
      if (httpd_resp_send_chunk(req, buff, read_bytes) != ESP_OK)
      {
        ESP_LOGE(TAG, "File sending failed: %s", filename.c_str());
        httpd_resp_sendstr_chunk(req, nullptr);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
        return ESP_FAIL;
      }
    }
  } while (read_bytes > 0);

  httpd_resp_send_chunk(req, nullptr, 0);
  return ESP_OK;
}

httpd_uri_t photo_binary_get_uri = {
    .uri = "/api/v1/photos/*",
    .method = HTTP_GET,
    .handler = photo_binary_get_handler,
    .user_ctx = nullptr,
};
