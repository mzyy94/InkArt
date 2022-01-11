#pragma once

#include "inkplate.hpp"
#include "qrcode.h"

void draw_qrcode(QRCode *qrcode, int16_t offset_x, int16_t offset_y, int16_t size);
void draw_setup_info(const char *ssid, const char *password, const char *ip_addr);
