#include "esp_log.h"

#include "inkplate.hpp"
#include "qrcode.h"

#undef PROGMEM
#define PROGMEM
#include "fonts/FreeMonoBold12pt7b.h"

static const char *TAG = "draw";

extern Inkplate display;

void draw_qrcode(QRCode *qrcode, int16_t offset_x, int16_t offset_y, int16_t size)
{
  uint16_t white = display.getDisplayMode() == DisplayMode::INKPLATE_3BIT ? 7 : 0;
  uint16_t black = display.getDisplayMode() == DisplayMode::INKPLATE_3BIT ? 0 : 1;

  for (size_t y = 0; y < qrcode->size; y++)
  {
    for (size_t x = 0; x < qrcode->size; x++)
    {
      auto color = qrcode_getModule(qrcode, x, y) ? black : white;
      display.fillRect(offset_x + x * size, offset_y + y * size, size, size, color);
    }
  }
}

void draw_setup_info(const char *ssid, const char *password, const char *ip_addr)
{
  QRCode qr;
  const uint8_t version = 3;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  char buff[50];

  display.selectDisplayMode(DisplayMode::INKPLATE_1BIT);

  snprintf(buff, sizeof(buff), "WIFI:T:WPA;S:%s;P:%s;;", ssid, password);
  qrcode_initText(&qr, qrcodeData, version, 0, buff);

  const int16_t size = 10;
  int16_t offset_x = (display.width() - qr.size * size) / 2;
  int16_t offset_y = (display.height() - qr.size * size) / 2;
  draw_qrcode(&qr, offset_x, offset_y, size);

  display.setTextSize(1);
  display.setFont(&FreeMonoBold12pt7b);

  display.setCursor(offset_x, offset_y + qr.size * size + 24);
  display.printf("Setup mode. Touch any touchpad to exit.\n");
  display.setCursor(offset_x, display.getCursorY());
  display.printf("SSID: %s Password: %s\n", ssid, password);
  display.setCursor(offset_x, display.getCursorY());
  display.printf("WebUI: http://inkart.local\n");
  display.setCursor(offset_x, display.getCursorY());
  display.printf("     : http://%s\n", ip_addr);

  display.display();
  ESP_LOGI(TAG, "Display setup information complete");
}

void draw_padding_preview(int16_t top, int16_t left, int16_t right, int16_t bottom, uint8_t rotation, bool invert)
{
  display.selectDisplayMode(DisplayMode::INKPLATE_1BIT);
  display.clearDisplay();

  display.setRotation(rotation);
  uint16_t color = 1;
  if (invert)
  {
    display.fillRect(0, 0, display.width(), display.height(), 1);
    color = 0;
  }
  display.drawRect(left, top, display.width() - (left + right), display.height() - (top + bottom), color);

  display.drawLine(left, top, display.width() - right, display.height() - bottom, color);
  display.drawLine(left, display.height() - bottom, display.width() - right, top, color);

  display.fillRect(left, top, 15, 4, color);
  display.fillRect(left, top, 4, 15, color);
  display.fillRect(left, display.height() - bottom - 4, 15, 4, color);
  display.fillRect(left, display.height() - bottom - 15, 4, 15, color);
  display.fillRect(display.width() - right - 4, top, 4, 15, color);
  display.fillRect(display.width() - right - 15, top, 15, 4, color);
  display.fillRect(display.width() - right - 4, display.height() - bottom - 15, 4, 15, color);
  display.fillRect(display.width() - right - 15, display.height() - bottom - 4, 15, 4, color);

  display.display();
}
