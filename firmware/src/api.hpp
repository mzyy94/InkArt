#pragma once

#include "esp_http_server.h"

extern httpd_uri_t system_info_get_uri;
extern httpd_uri_t system_display_get_uri;
extern httpd_uri_t system_display_post_uri;
extern httpd_uri_t system_time_get_uri;
extern httpd_uri_t system_time_post_uri;
extern httpd_uri_t photo_list_get_uri;
extern httpd_uri_t photo_list_patch_uri;
extern httpd_uri_t photo_binary_get_uri;
extern httpd_uri_t photo_binary_delete_uri;
