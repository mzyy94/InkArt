#pragma once

#include "inkplate.hpp"
#include "qrcode.h"

void draw_qrcode(QRCode *qrcode, int16_t offset_x, int16_t offset_y, int16_t size);
void draw_setup_info(const char *ssid, const char *password, const char *ip_addr);
void draw_padding_preview(int16_t top, int16_t left, int16_t right, int16_t bottom, uint8_t rotation, bool invert);
