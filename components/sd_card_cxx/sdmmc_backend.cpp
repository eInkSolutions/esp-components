#include "sdmmc_backend.hpp"
#include "driver/sdmmc_default_configs.h"
#include "driver/sdmmc_host.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sd_protocol_types.h"
// #include "soc/soc_caps.h"

#ifdef CONFIG_SDMMC_DEBUG_PIN_CONNECTIONS
#include "sd_test_io.h"

const char *names[] = {"CLK", "CMD", "D0", "D1", "D2", "D3"};
const int pins[] = {CONFIG_EXAMPLE_PIN_CLK,
                    CONFIG_EXAMPLE_PIN_CMD,
                    CONFIG_EXAMPLE_PIN_D0
#ifdef CONFIG_SDCARD_INTERFACE_SDMMC_4BIT
                    ,
                    CONFIG_EXAMPLE_PIN_D1,
                    CONFIG_EXAMPLE_PIN_D2,
                    CONFIG_EXAMPLE_PIN_D3
#endif
};

const int pin_count = sizeof(pins) / sizeof(pins[0]);
#endif // CONFIG_SDMMC_DEBUG_PIN_CONNECTIONS

namespace sdcard::internal {

static const char *TAG = "SDMMC_Backend";

bool SdMmcBackend::begin() {
  esp_err_t ret;
  // ESP-IDF SDMMC initialization

  // mount options
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed =
          cfg_.format_if_mount_failed, // TODO: allow Kconfig here?
      .max_files = static_cast<int>(cfg_.max_files),
      .allocation_unit_size = cfg_.allocation_unit_size,
      .disk_status_check_enable = true,
      .use_one_fat = false};

  sdmmc_card_t *card;
  // const char *mount_point = cfg_.mount_point;
  ESP_LOGI(TAG, "Initializing SD card");
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.slot = SDMMC_HOST_SLOT_1;

  ESP_LOGI(TAG, "SD Freq Setting...");
  configureSdcardSpeed(host);
  ESP_LOGI(TAG, "SD Power Setting...");
  // TODO: implement sd card power control
  ESP_LOGI(TAG, "SD Power not yet implemented");

  ESP_LOGI(TAG, "SD Slot Setting...");
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  ESP_LOGI(TAG, "Loaded Default SD Slot Settings");
  configureSdcardPins(slot_config);

  // Set bus width to use:
#ifdef CONFIG_SDCARD_INTERFACE_SDMMC_4BIT
  ESP_LOGI(TAG, "4Bit Mode selection");
  slot_config.width = 4;
#else
  slot_config.width = 1;
#endif

  slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

  ESP_LOGI(TAG, "Mounting filesystem");

  // Logging all onfigurations in debug mode
  ESP_LOGD(TAG, "SDMMC configuration:");
  ESP_LOGD(TAG, "  Mount point: %s", cfg_.mount_point);
  ESP_LOGD(TAG, "  Host slot: %d", host.slot);
  ESP_LOGD(TAG, "  Host frequency: %d Hz", host.max_freq_khz * 1000);
  ESP_LOGD(TAG, "  Bus width: %d", slot_config.width);
  ESP_LOGD(TAG, "  CMD GPIO: %d", slot_config.cmd);
  ESP_LOGD(TAG, "  CLK GPIO: %d", slot_config.clk);
  ESP_LOGD(TAG, "  D0 GPIO: %d", slot_config.d0);
  ESP_LOGD(TAG, "  D1 GPIO: %d", slot_config.d1);
  ESP_LOGD(TAG, "  D2 GPIO: %d", slot_config.d2);
  ESP_LOGD(TAG, "  D3 GPIO: %d", slot_config.d3);
  ESP_LOGD(TAG, "  Format if mount failed: %s",
           mount_config.format_if_mount_failed ? "true" : "false");
  ESP_LOGD(TAG, "  Max files: %d", mount_config.max_files);
  ESP_LOGD(TAG, "  Allocation unit size: %d",
           mount_config.allocation_unit_size);

  // TODO: This should be replaced with the lower level function - just
  // convinient function, but hides the actual state and error of the
  // initialization.
  ret = esp_vfs_fat_sdmmc_mount(cfg_.mount_point, &host, &slot_config,
                                &mount_config, &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount filesystem. ");
    } else {
      ESP_LOGE(TAG,
               "Failed to initialize the card (%s). "
               "Make sure SD card lines have pull-up resistors in place.",
               esp_err_to_name(ret));
    }
    return false;
  }

  card_ = card;
  mounted_ = true;
  ESP_LOGI(TAG, "Filesystem mounted");

  return true;
}

void SdMmcBackend::end() {
  if (!mounted_) {
    ESP_LOGW(TAG, "Noting to unmount");
    return;
  }
  // ESP-IDF SDMMC cleanup
  esp_vfs_fat_sdcard_unmount(cfg_.mount_point, card_);
  ESP_LOGI(TAG, "Card unmounted");
  mounted_ = false;
}

bool SdMmcBackend::mounted() const { return mounted_; }

const char *SdMmcBackend::mountPoint() const { return cfg_.mount_point; }

void SdMmcBackend::configureSdcardSpeed(sdmmc_host_t &host) {
  // #if CONFIG_SDCARD_SPEED_UHS_I_SDR50 || \
  //     CONFIG_SDCARD_SPEED_UHS_I_DDR50 || \
  //     CONFIG_SDCARD_SPEED_UHS_I_SDR104
  //
  // #if SOC_SDMMC_IO_UHS_POWER_EXTERNAL
  //     host.io_voltage = 1.8f;
  // #endif
  //
  // #endif
  ESP_LOGI(TAG,
           "Additional SD-Card speed not yet implemented... using default");
}

void SdMmcBackend::configureSdcardPins(sdmmc_slot_config_t &slot_config) {

#if CONFIG_SDCARD_SDMMC_CUSTOM_PINS

  ESP_LOGI(TAG, "Using custom SDMMC GPIO configuration");

  slot_config.clk = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_CLK_GPIO);

  slot_config.cmd = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_CMD_GPIO);

  slot_config.d0 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D0_GPIO);

#if CONFIG_SDCARD_INTERFACE_SDMMC_4BIT

  slot_config.d1 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D1_GPIO);

  slot_config.d2 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D2_GPIO);

  slot_config.d3 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D3_GPIO);

#endif

#elif SOC_SDMMC_USE_GPIO_MATRIX

  ESP_LOGI(TAG, "Using GPIO-MATRIX with default SDMMC GPIO configuration");

  slot_config.clk = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_CLK_GPIO);

  slot_config.cmd = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_CMD_GPIO);

  slot_config.d0 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D0_GPIO);

#if CONFIG_SDCARD_INTERFACE_SDMMC_4BIT

  slot_config.d1 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D1_GPIO);

  slot_config.d2 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D2_GPIO);

  slot_config.d3 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D3_GPIO);

#endif

#else

  ESP_LOGI(TAG, "Using ESP-IDF default SDMMC GPIO configuration");

#endif
}

} // namespace sdcard::internal
