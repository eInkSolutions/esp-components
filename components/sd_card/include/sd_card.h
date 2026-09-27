#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SDCARD_INTERFACE_AUTO,
  SDCARD_INTERFACE_SDMMC,
  SDCARD_INTERFACE_SDSPI
} sdcard_interface_t;

typedef struct {
  sdcard_interface_t interface;
  const char *mount_point;

  bool format_if_mount_failed;
  size_t max_files;
  size_t allocation_unit_size;
} sdcard_config_t;

typedef struct sdcard_t sdcard_t;

/**
 * Create an SD card instance.
 */
sdcard_t *sdcard_create(const sdcard_config_t *config);

/**
 * Destroy an SD card instance.
 *
 * The card is automatically unmounted if necessary.
 */
void sdcard_destroy(sdcard_t *card);

/**
 * Initialize and mount the SD card.
 */
bool sdcard_begin(sdcard_t *card);

/**
 * Unmount the SD card and release resources.
 */
void sdcard_end(sdcard_t *card);

/**
 * Check whether the SD card is currently mounted.
 */
bool sdcard_mounted(const sdcard_t *card);

/**
 * Get the configured mount point.
 */
const char *sdcard_mount_point(const sdcard_t *card);

#ifdef __cplusplus
}
#endif
