set(srcs "sd_card_cxx.cpp")

set(priv_requires)

if(CONFIG_SDCARD_INTERFACE_SDMMC_1BIT OR CONFIG_SDCARD_INTERFACE_SDMMC_4BIT)
  list(APPEND srcs "sdmmc_backend.cpp")

  list(APPEND priv_requires esp_driver_sdmmc)
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES ${priv_requires}
                    REQUIRES fatfs)
