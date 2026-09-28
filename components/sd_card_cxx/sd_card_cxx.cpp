#include "include/sd_card_cxx.hpp"
#include "sd_card_impl.hpp"
#include "sdkconfig.h"

#if CONFIG_SDCARD_INTERFACE_SDMMC_1BIT || CONFIG_SDCARD_INTERFACE_SDMMC_4BIT
#include "sdmmc_backend.hpp"
#endif

namespace sdcard {
#if CONFIG_SDCARD_INTERFACE_SDMMC_1BIT || CONFIG_SDCARD_INTERFACE_SDMMC_4BIT

using SelectedBackend = internal::SdMmcBackend;

#endif

struct SdCard::Impl : internal::SdCardImpl<SelectedBackend> {
  using internal::SdCardImpl<SelectedBackend>::SdCardImpl;
};

SdCard::SdCard(const Config &cfg) : impl_(new Impl(cfg)) {}

/**
 * Initialize and mount the SD card.
 */
bool SdCard::begin() { return impl_->begin(); }

/**
 * Unmount the SD card and release resources.
 */
void SdCard::end() { impl_->end(); }

/**
 * Check whether the SD card is currently mounted.
 */
bool SdCard::mounted() const { return impl_->mounted(); }

/**
 * Get the configured mount point.
 */
const char *SdCard::mountPoint() const { return impl_->mountPoint(); }

SdCard::~SdCard() { delete impl_; }

} // namespace sdcard
