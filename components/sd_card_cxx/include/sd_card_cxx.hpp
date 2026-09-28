#pragma once

#include <cstddef>
#include <cstdint>

namespace sdcard {
enum class Interface { Auto, SDMMC, SDSPI };

struct Config {
  Interface interface = Interface::Auto;
  const char *mount_point = "/sdcard";

  bool format_if_mount_failed = false;
  std::size_t max_files = 5;
  std::size_t allocation_unit_size = 16 * 1024;
};

class SdCard {
public:
  explicit SdCard(const Config &cfg = {});

  ~SdCard();

  SdCard(const SdCard &) = delete;
  SdCard &operator=(const SdCard &) = delete;

  SdCard(SdCard &&) = delete;
  SdCard &operator=(SdCard &&) = delete;

  /**
   * Initialize and mount the SD card.
   */
  bool begin();

  /**
   * Unmount the SD card and release resources.
   */
  void end();

  /**
   * Check whether the SD card is currently mounted.
   */
  bool mounted() const;

  /**
   * Get the configured mount point.
   */
  const char *mountPoint() const;

private:
  struct Impl;
  Impl *impl_;
};

} // namespace sdcard
  //
  //
