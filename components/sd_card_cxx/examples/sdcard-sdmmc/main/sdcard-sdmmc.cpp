#include "esp_log.h"
#include "sd_card_cxx.hpp"

static const char *TAG = "TEST APP";

extern "C" void app_main(void) { ESP_LOGI(TAG, "Hello World"); }
