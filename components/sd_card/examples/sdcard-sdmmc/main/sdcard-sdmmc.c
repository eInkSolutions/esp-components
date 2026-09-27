#include "esp_log.h"
#include "sd_card.h"

static const char *TAG = "SD CARD SDMMC";

void app_main(void) {
  ESP_LOGI(TAG, "Hello World!");

  sdcard_config_t cfg = {
      .interface = SDCARD_INTERFACE_SDMMC,
      .mount_point = "/sdcard",
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
  };

  sdcard_t *sd_card_t = sdcard_create(&cfg);

  if (!sdcard_begin(sd_card_t)) {
    ESP_LOGE(TAG, "Worked NOT");
  }

  if (sdcard_mounted(sd_card_t)) {
    ESP_LOGI(TAG, "Mounted");
  }
}
