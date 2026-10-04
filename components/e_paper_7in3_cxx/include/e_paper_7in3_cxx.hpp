#pragma once

#include "EpdCommands.hpp"
#include "EpdFramebuffer.hpp"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Epd {

class Panel7in3 {
public:
  Panel7in3(esp_lcd_panel_io_handle_t io, int reset_gpio, int busy_gpio);

  ~Panel7in3();

  esp_err_t reset();
  esp_err_t init();

  esp_err_t drawBitmap(int xStart, int yStart, int xEnd, int yEnd,
                       const void *data);

  esp_err_t refresh();
  esp_err_t waitIdle();

  esp_err_t waitForRefreshComplete();

private:
  esp_lcd_panel_t panel_;
  esp_lcd_panel_io_handle_t io_;

  gpio_num_t resetGpio_;
  gpio_num_t busyGpio_;

  bool resetLevel_;
  bool busyLevel_;

  uint16_t width_;
  uint16_t height_;

  bool mirrorX_;
  bool mirrorY_;
  bool swapXY_;
  bool invertColor_;

  Framebuffer framebuffer_;

  void sendCommand(const EpdCommands::Command &command);
  void sendData(const uint8_t *data, size_t size);

#ifdef CONFIG_EPD_BUSY_INTERRUPT_ENABLED
  static void busyIsrHandler(void *arg);

  void handleBusyInterrupt();
  // esp_err_t waitForRefreshPolling();
  // esp_err_t waitForRefreshInterrupt();

  esp_err_t configureBusyInterrupt();
  esp_err_t registerBusyInterrupt();
  SemaphoreHandle_t refreshDoneSemaphore_;
#endif // CONFIG_EPD_BUSY_INTERRUPT_ENABLED
};

} // namespace Epd
