#include "include/sd_card_cxx.hpp"
#include "sd_card_impl.hpp"
#include "sdkconfig.h"
#include "sdmmc_backend.hpp"
#include <stdio.h>

namespace sdcard {
#if CONFIG_SDCARD_INTERFACE_SDMMC_1BIT || CONFIG_SDCARD_INTERFACE_SDMMC_4BIT

using SelectedBackend = internal::SdMmcBackend;

#endif

struct SdCard::Impl : internal::SdCardImpl<SelectedBackend> {
  using internal::SdCardImpl<SelectedBackend>::SdCardImpl;
};
} // namespace sdcard
