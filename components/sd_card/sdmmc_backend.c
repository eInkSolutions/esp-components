#include "sdmmc_backend.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "include/sd_card.h"
#include "sdkconfig.h"

#if CONFIG_SDCARD_INTERFACE_SDMMC_1BIT

// SDMMC 1-bit

#elif CONFIG_SDCARD_INTERFACE_SDMMC_4BIT

// SDMMC 4-bit

#elif CONFIG_SDCARD_INTERFACE_SDSPI

// SDSPI

#endif

static const char *TAG = "SDMMC_Backend";

struct sdmmc_backend_t {
  sdcard_config_t config;
  sdmmc_card_t *card;
  bool mounted;
};

sdmmc_backend_t *sdmmc_backend_create(const sdcard_config_t *config) {
  sdmmc_backend_t *backend = calloc(1, sizeof(sdmmc_backend_t));

  if (backend == NULL) {
    return NULL;
  }

  if (config != NULL) {
    backend->config = *config;
  }

  return backend;
}

void sdmmc_backend_destroy(sdmmc_backend_t *backend) {
  if (backend == NULL) {
    return;
  }

  sdmmc_backend_end(backend);

  free(backend);
}

bool sdmmc_backend_begin(sdmmc_backend_t *backend) {
  if (backend == NULL || backend->mounted) {
    return false;
  }

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

  /*
   * Freenove ESP32 WROVER board:
   *
   * CLK = GPIO14
   * CMD = GPIO15
   * D0  = GPIO2
   */
#if CONFIG_SDCARD_INTERFACE_SDMMC_1BIT
  slot_config.clk = (gpio_num_t)CONFIG_SDCARD_SDMMC_CLK_GPIO;
  slot_config.cmd = (gpio_num_t)CONFIG_SDCARD_SDMMC_CMD_GPIO;
  slot_config.d0 = (gpio_num_t)CONFIG_SDCARD_SDMMC_D0_GPIO;
  // Use 1-bit SDMMC mode.
  slot_config.width = 1;
#endif

#if CONFIG_SDCARD_INTERFACE_SDMMC_4BIT
  // Use 1-bit SDMMC mode.
  slot_config.width = 4;
#endif

  esp_vfs_fat_mount_config_t mount_config = {
      .format_if_mount_failed = backend->config.format_if_mount_failed,

      .max_files = backend->config.max_files,

      .allocation_unit_size = backend->config.allocation_unit_size,
  };

  esp_err_t err =
      esp_vfs_fat_sdmmc_mount(backend->config.mount_point, &host, &slot_config,
                              &mount_config, &backend->card);

  if (err != ESP_OK) {
    backend->card = NULL;
    backend->mounted = false;
    return false;
  }

  backend->mounted = true;
  ESP_LOGI(TAG, "Mounted!");
  return true;
}

void sdmmc_backend_end(sdmmc_backend_t *backend) {
  if (backend == NULL || !backend->mounted) {
    return;
  }

  esp_vfs_fat_sdcard_unmount(backend->config.mount_point, backend->card);

  backend->card = NULL;
  backend->mounted = false;
}

bool sdmmc_backend_mounted(const sdmmc_backend_t *backend) {
  return backend != NULL && backend->mounted;
}
