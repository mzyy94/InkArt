#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include "ff.h"
#include "lwip/inet.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sleep.h"

#include "files.hpp"
#include "draw.hpp"

#include "nlohmann/json.hpp"

using nlohmann::json;

static const char *TAG = "api";

static const uint8_t base64table[256] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x3f, 0x3e, 0x3e, 0x3f,
     0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
     0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x3f,
     0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
     0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33};

#define b64d(i) ((uint32_t)base64table[(uint8_t)i])

ssize_t b64decode(const char *data, const size_t len, char *buff, const size_t buflen)
{
  if (len == 0)
    return 0;
  if (len % 4 != 0)
    return -1;

  uint8_t pad1 = data[len - 1] == '=';
  uint8_t pad2 = pad1 && data[len - 2] != '=';
  const size_t last = (len - pad1) / 4 << 2;
  const size_t total = last / 4 * 3 + pad1 + pad2;

  if (buflen < total)
    return -1;

  char *b = buff;
  for (size_t i = 0; i < last; i += 4)
  {
    uint32_t n = b64d(data[i]) << 18 | b64d(data[i + 1]) << 12 | b64d(data[i + 2]) << 6 | b64d(data[i + 3]);
    *b++ = n >> 16;
    *b++ = n >> 8 & 0xFF;
    *b++ = n & 0xFF;
  }

  if (pad1)
  {
    uint32_t n = b64d(data[last]) << 18 | b64d(data[last + 1]) << 12;
    *b++ = n >> 16;
    if (pad2)
    {
      n |= b64d(data[last + 2]) << 6;
      *b++ = n >> 8 & 0xFF;
    }
  }
  return b - buff;
}

esp_err_t parse_json(httpd_req_t *req, json &j)
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
  j = json::parse(buff, nullptr, false);
  if (j.is_discarded())
  {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse content");
    return ESP_FAIL;
  }

  return ESP_OK;
}

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

  FATFS *fs;
  DWORD free_clst;
  if (f_getfree("0:", &free_clst, &fs) == FR_OK)
  {
    uint64_t total_clst = fs->n_fatent - 2;
    j["storage"]["used"] = ((uint64_t)fs->csize * (total_clst - free_clst)) * fs->ssize;
    j["storage"]["total"] = ((uint64_t)fs->csize * total_clst) * fs->ssize;
  }
  else
  {
    j["storage"] = nullptr;
  }

  std::vector<std::string> bmps;
  readbmps("/sdcard/", bmps);
  j["photos"] = bmps.size();

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
  nvs_get_u8(handle, "invert", &val);
  j["invert"] = bool(val);
  nvs_get_u8(handle, "dithering", &val);
  j["dithering"] = bool(val);
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
  json j;
  auto ret = parse_json(req, j);
  if (ret != ESP_OK)
  {
    return ret;
  }

  nvs_handle_t handle;
  nvs_open("system_settings", NVS_READWRITE, &handle);

  uint8_t val;
  if (j.contains("invert"))
  {
    val = j["invert"];
    nvs_set_u8(handle, "invert", val);
  }
  if (j.contains("dithering"))
  {
    val = j["dithering"];
    nvs_set_u8(handle, "dithering", val);
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

static esp_err_t system_display_preview_post_handler(httpd_req_t *req)
{
  json j;
  auto ret = parse_json(req, j);
  if (ret != ESP_OK)
  {
    return ret;
  }

  bool invert = false;
  uint8_t rotation = 0;
  int16_t top = 0, left = 0, right = 0, bottom = 0;

  if (j.contains("invert"))
  {
    invert = j["invert"];
  }
  if (j.contains("orientation"))
  {
    std::string orientation = j["orientation"];
    auto iter = std::find(orientations.begin(), orientations.end(), orientation);
    rotation = std::distance(orientations.begin(), iter);
  }
  if (j.contains("padding"))
  {
    top = j["padding"]["top"];
    left = j["padding"]["left"];
    right = j["padding"]["right"];
    bottom = j["padding"]["bottom"];
  }

  draw_padding_preview(top, left, right, bottom, rotation, invert);

  json ok;
  ok["status"] = "ok";
  const std::string str = ok.dump(4);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t system_display_preview_post_uri = {
    .uri = "/api/v1/system/display/preview",
    .method = HTTP_POST,
    .handler = system_display_preview_post_handler,
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

  uint8_t val2 = 0;
  nvs_get_u8(handle, "shuffle", &val2);
  j["shuffle"] = val2;

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
  json j;
  auto ret = parse_json(req, j);
  if (ret != ESP_OK)
  {
    return ret;
  }

  if (j.contains("time"))
  {
    uint64_t time_ms = j["time"];
    struct timeval tv_now;
    tv_now.tv_sec = time_ms / 1000;
    tv_now.tv_usec = (time_ms % 1000) * 1000;
    settimeofday(&tv_now, nullptr);
  }

  if (j.contains("refresh") || j.contains("shuffle"))
  {
    nvs_handle_t handle;
    nvs_open("system_settings", NVS_READWRITE, &handle);
    if (j.contains("refresh"))
    {
      uint16_t val = j["refresh"];
      nvs_set_u16(handle, "refresh", val);
    }
    if (j.contains("shuffle"))
    {
      uint8_t val = j["shuffle"];
      nvs_set_u8(handle, "shuffle", val);
    }
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

static esp_err_t photo_list_patch_handler(httpd_req_t *req)
{
  json j;
  auto ret = parse_json(req, j);
  if (ret != ESP_OK)
  {
    return ret;
  }

  if (j.contains("data") && j["data"].is_array())
  {
    for (const auto &ent : j["data"])
    {
      bool hidden = ent["hidden"];
      std::string filename = ent["filename"];
      std::string old_path, new_path;
      if (hidden)
      {
        old_path = "/sdcard/" + filename;
        new_path = "/sdcard/." + filename;
      }
      else
      {
        old_path = "/sdcard/." + filename;
        new_path = "/sdcard/" + filename;
      }

      if (rename(old_path.c_str(), new_path.c_str()) != 0)
      {
        ret |= ESP_FAIL;
      }
    }
  }

  json res;
  res["status"] = ret == ESP_OK ? "ok" : "fail";
  std::string str = res.dump(4);
  httpd_resp_set_type(req, "application/json");
  if (ret == ESP_OK)
  {
    httpd_resp_sendstr(req, str.c_str());
  }
  else
  {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to change hidden state");
  }

  return ret;
}

httpd_uri_t photo_list_patch_uri = {
    .uri = "/api/v1/photos",
    .method = HTTP_PATCH,
    .handler = photo_list_patch_handler,
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
  httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=31536000, immutable");

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

static esp_err_t photo_binary_delete_handler(httpd_req_t *req)
{
  std::string uri = req->uri;
  const auto filename = uri.substr(uri.find_last_of("/") + 1);

  esp_err_t ret = ESP_OK;
  auto filepath = "/sdcard/" + filename;
  if (remove(filepath.c_str()) != 0)
  {
    filepath = "/sdcard/." + filename;
    if (remove(filepath.c_str()) != 0)
    {
      ret |= ESP_FAIL;
    }
  }

  json res;
  res["status"] = ret == ESP_OK ? "ok" : "fail";
  std::string str = res.dump(4);
  httpd_resp_set_type(req, "application/json");
  if (ret == ESP_OK)
  {
    httpd_resp_sendstr(req, str.c_str());
  }
  else
  {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to change hidden state");
  }

  return ret;
}

httpd_uri_t photo_binary_delete_uri = {
    .uri = "/api/v1/photos/*",
    .method = HTTP_DELETE,
    .handler = photo_binary_delete_handler,
    .user_ctx = nullptr,
};

static esp_err_t photo_binary_post_handler(httpd_req_t *req)
{
  if (req->content_len % 4 != 0)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid content length");
    return ESP_FAIL;
  }

  const size_t total_len = req->content_len;
  size_t cur_len = 0;

  char buff[128];
  char buff2[96];
  struct timeval tv_now;
  gettimeofday(&tv_now, nullptr);
  snprintf(buff, sizeof(buff), "/sdcard/%ld.bmp", tv_now.tv_sec);

  std::ofstream ofs(buff, std::ios::out | std::ios::binary);
  if (!ofs)
  {
    ESP_LOGE(TAG, "Failed to create new file");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create new file");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Open new file: %s", buff);

  while (cur_len < total_len)
  {
    auto len = httpd_req_recv(req, buff, sizeof(buff));
    if (len <= 0)
    {
      ESP_LOGE(TAG, "Failed to receive content");
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive content");
      return ESP_FAIL;
    }
    auto decoded = b64decode(buff, len, buff2, sizeof(buff2));
    if (decoded == -1)
    {
      ESP_LOGE(TAG, "Failed to decode base64 binary");
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to decode base64 binary");
      return ESP_FAIL;
    }
    else if (decoded > 0)
    {
      ofs.write(buff2, decoded);
    }
    cur_len += len;
  }
  ESP_LOGI(TAG, "Create new file completed");

  json res;
  snprintf(buff, sizeof(buff), "%ld.bmp", tv_now.tv_sec);
  res["filename"] = buff;
  res["status"] = "ok";
  std::string str = res.dump(4);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t photo_binary_post_uri = {
    .uri = "/api/v1/photos",
    .method = HTTP_POST,
    .handler = photo_binary_post_handler,
    .user_ctx = nullptr,
};

#include "inkplate.hpp"
extern Inkplate display;

static esp_err_t photo_preview_binary_post_handler(httpd_req_t *req)
{
  if (req->content_len % 4 != 0)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid content length");
    return ESP_FAIL;
  }

  int16_t x, y;
  uint8_t invert, rotation, dithering;

  nvs_handle_t handle;
  nvs_open("system_settings", NVS_READONLY, &handle);
  nvs_get_u8(handle, "invert", &invert);
  nvs_get_u8(handle, "dithering", &dithering);
  nvs_get_u8(handle, "orientation", &rotation);
  nvs_get_i16(handle, "padding-top", &y);
  nvs_get_i16(handle, "padding-left", &x);
  nvs_close(handle);

  const size_t total_len = req->content_len;
  size_t cur_len = 0;

  char buff[128];
  char *buf = (char *)malloc(total_len * 3 / 4);

  while (cur_len < total_len)
  {
    auto len = httpd_req_recv(req, buff, sizeof(buff));
    if (len <= 0)
    {
      ESP_LOGE(TAG, "Failed to receive content");
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive content");
      return ESP_FAIL;
    }
    auto buff2 = buf + (cur_len * 3 / 4);
    auto decoded = b64decode(buff, len, buff2, len * 3 / 4);
    if (decoded == -1)
    {
      ESP_LOGE(TAG, "Failed to decode base64 binary");
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to decode base64 binary");
      return ESP_FAIL;
    }
    cur_len += len;
  }

  display.selectDisplayMode(DisplayMode::INKPLATE_3BIT);
  display.setRotation(rotation);
  display.drawBitmapFromBuffer((uint8_t *)buf, x, y, dithering, invert);
  display.display();

  free(buf);
  ESP_LOGI(TAG, "Preview file completed");

  json res;
  res["status"] = "ok";
  std::string str = res.dump(4);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  return ESP_OK;
}

httpd_uri_t photo_preview_binary_post_uri = {
    .uri = "/api/v1/photos/preview",
    .method = HTTP_POST,
    .handler = photo_preview_binary_post_handler,
    .user_ctx = nullptr,
};

static esp_err_t system_reboot_post_handler(httpd_req_t *req)
{
  json res;
  res["status"] = "ok";
  std::string str = res.dump(4);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, str.c_str());

  ESP::delay(1000);
  esp_sleep_enable_timer_wakeup(1000000);
  esp_deep_sleep_start();

  return ESP_OK;
}

httpd_uri_t system_reboot_post_uri = {
    .uri = "/api/v1/system/reboot",
    .method = HTTP_POST,
    .handler = system_reboot_post_handler,
    .user_ctx = nullptr,
};
