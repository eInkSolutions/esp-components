#include "sdmmc_backend.hpp"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "sd_protocol_types.h"

namespace sdcard::internal {

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
  // const char mount_point[] = cfg_.mount_point;

  return true;
}

void SdMmcBackend::end() {
  // ESP-IDF SDMMC cleanup
  mounted_ = false;
}

bool SdMmcBackend::mounted() const { return mounted_; }

const char *SdMmcBackend::mountPoint() const { return cfg_.mount_point; }

} // namespace sdcard::internal
