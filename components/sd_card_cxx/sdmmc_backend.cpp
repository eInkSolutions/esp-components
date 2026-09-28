#include "sdmmc_backend.hpp"

namespace sdcard::internal {

bool SdMmcBackend::begin() {
  // ESP-IDF SDMMC initialization
  return true;
}

void SdMmcBackend::end() {
  // ESP-IDF SDMMC cleanup
  mounted_ = false;
}

bool SdMmcBackend::mounted() const { return mounted_; }

const char *SdMmcBackend::mountPoint() const { return cfg_.mount_point; }

} // namespace sdcard::internal
