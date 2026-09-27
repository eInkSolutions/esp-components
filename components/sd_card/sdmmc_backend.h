#pragma once

#include "include/sd_card.h"

typedef struct sdmmc_backend_t sdmmc_backend_t;

sdmmc_backend_t *sdmmc_backend_create(const sdcard_config_t *config);

void sdmmc_backend_destroy(sdmmc_backend_t *backend);

bool sdmmc_backend_begin(sdmmc_backend_t *backend);

void sdmmc_backend_end(sdmmc_backend_t *backend);
