set(srcs "sd_card_cxx.cpp")

set(priv_requires fatfs)

if(CONFIG_SDCARD_INTERFACE_SDMMC_1BIT OR CONFIG_SDCARD_INTERFACE_SDMMC_4BIT)
  list(APPEND srcs "sdmmc_backend.cpp")

  list(APPEND priv_requires esp_driver_sdmmc)
endif()

if(CONFIG_SDMMC_DEBUG_PIN_CONNECTIONS)
  list(APPEND srcs "sd_test_io.c")

  list(APPEND priv_requires esp_adc)
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES ${priv_requires}
                    REQUIRES fatfs)
