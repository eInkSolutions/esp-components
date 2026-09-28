#include "sd_card.h"
#include "sdmmc_backend.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct sdmmc_backend_t sdmmc_backend_t;

struct sdcard_t {
  sdcard_config_t config;
  bool mounted;

  sdmmc_backend_t *sdmmc;
};

sdcard_t *sdcard_create(const sdcard_config_t *config) {
  sdcard_t *card = calloc(1, sizeof(sdcard_t));

  if (card == NULL) {
    return NULL;
  }

  if (config != NULL) {
    card->config = *config;
  } else {
    card->config.interface = SDCARD_INTERFACE_AUTO;
    card->config.mount_point = "/sdcard";
    card->config.format_if_mount_failed = false;
    card->config.max_files = 5;
    card->config.allocation_unit_size = 16 * 1024;
  }

  switch (card->config.interface) {
  case SDCARD_INTERFACE_SDMMC:
    card->sdmmc = sdmmc_backend_create(&card->config);
    break;
  case SDCARD_INTERFACE_SDSPI:
    break;
  case SDCARD_INTERFACE_AUTO:
    break;
  }

  if (card->sdmmc == NULL) {
    free(card);
    return NULL;
  }

  return card;
}

void sdcard_destroy(sdcard_t *card) {
  if (card == NULL) {
    return;
  }

  sdcard_end(card);
  free(card);
}

bool sdcard_begin(sdcard_t *card) {
  if (card == NULL || card->mounted) {
    return false;
  }

  if (!sdmmc_backend_begin(card->sdmmc)) {
    return false;
  }

  /*
   * Initialize and mount the SD card here.
   */

  card->mounted = true;

  return true;
}

void sdcard_end(sdcard_t *card) {
  if (card == NULL || !card->mounted) {
    return;
  }

  /*
   * Unmount and release SD card resources here.
   */
  sdmmc_backend_end(card->sdmmc);

  card->mounted = false;
}

bool sdcard_mounted(const sdcard_t *card) {
  return card != NULL && card->mounted;
}

const char *sdcard_mount_point(const sdcard_t *card) {
  if (card == NULL) {
    return NULL;
  }

  return card->config.mount_point;
}
