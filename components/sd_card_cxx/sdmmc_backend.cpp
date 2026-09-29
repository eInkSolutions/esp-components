#include "sdmmc_backend.hpp"
#include "driver/sdmmc_host.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sd_protocol_types.h"
// #include "soc/soc_caps.h"

namespace sdcard::internal {

static const char *TAG = "SDMMC_Backend";

bool SdMmcBackend::begin() {
  esp_err_t ret;
  // ESP-IDF SDMMC initialization

  // mount options
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false, // TODO: allow Kconfig here?
      .max_files = static_cast<int>(cfg_.max_files),
      .allocation_unit_size = cfg_.allocation_unit_size,
      .disk_status_check_enable = true,
      .use_one_fat = false};

  sdmmc_card_t *card;
  // const char *mount_point = cfg_.mount_point;
  ESP_LOGI(TAG, "Initializing SD card");
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();

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

#if SOC_SDMMC_USE_GPIO_MATRIX
  ESP_LOGI(TAG, "Using GPIO-MATRIX");
  slot_config.clk = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_CLK_GPIO);
  slot_config.cmd = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_CMD_GPIO);
  slot_config.d0 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D0_GPIO);

#if CONFIG_SDCARD_INTERFACE_SDMMC_4BIT
  ESP_LOGI(TAG, "Using 4Bit Bus-Width");
  slot_config.d1 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D1_GPIO);
  slot_config.d2 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D2_GPIO);
  slot_config.d3 = static_cast<gpio_num_t>(CONFIG_SDCARD_SDMMC_D3_GPIO);

#endif // CONFIG_SDCARD_INTERFACE_SDMMC_4BIT

#else
  ESP_LOGI(TAG, "Non GPIO-MATRIX. Using defaults.");

#endif // SOC_SDMMC_USE_GPIO_MATRIX
}

} // namespace sdcard::internal
