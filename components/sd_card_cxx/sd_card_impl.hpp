#pragma once

#include "include/sd_card_cxx.hpp"
namespace sdcard::internal {

template <typename Backend> class SdCardImpl {
public:
  explicit SdCardImpl(const Config &cfg) : backend_(cfg) {}

  bool begin() { return backend_.begin(); }

  void end() { backend_.end(); }

  bool mounted() const { return backend_.mounted(); }

  const char *mountPoint() const { return backend_.mountPoint(); }

private:
  Backend backend_;
};
} // namespace sdcard::internal
