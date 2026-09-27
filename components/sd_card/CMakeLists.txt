idf_component_register(SRCS "sd_card.c" "sdmmc_backend.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_driver_sdmmc
                    REQUIRES fatfs)
# target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++17)
