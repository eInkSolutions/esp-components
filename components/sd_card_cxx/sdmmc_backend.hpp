#pragma once

#include "include/sd_card_cxx.hpp"

namespace sdcard::internal {

class SdMmcBackend {
public:
  explicit SdMmcBackend(const Config &cfg) : cfg_(cfg) {}

  bool begin();
  void end();

  bool mounted() const;
  const char *mountPoint() const;

private:
  const Config &cfg_;
  bool mounted_ = false;
};

} // namespace sdcard::internal
