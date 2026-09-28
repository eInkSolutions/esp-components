#pragma once

#include "driver/sdmmc_host.h"
#include "include/sd_card_cxx.hpp"
#include "sd_protocol_types.h"

namespace sdcard::internal {

class SdMmcBackend {
public:
  explicit SdMmcBackend(const Config &cfg) : cfg_(cfg) {}

  ~SdMmcBackend() { end(); }

  bool begin();
  void end();

  bool mounted() const;
  const char *mountPoint() const;

private:
  Config cfg_;
  bool mounted_ = false;
  sdmmc_card_t *card_ = nullptr;

  void configureSdcardSpeed(sdmmc_host_t &host);
  void configureSdcardPins(sdmmc_slot_config_t &slot_config);
};

} // namespace sdcard::internal
