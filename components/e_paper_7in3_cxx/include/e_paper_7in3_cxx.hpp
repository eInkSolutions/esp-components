#pragma once

#include "EpdCommands.hpp"
#include "EpdFramebuffer.hpp"
#include "esp_err.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <cstddef>

namespace Epd {

typedef struct {
  int busy_gpio_num; /*!< GPIO num of the BUSY pin */
  bool
      non_copy_mode; /*!< If the bitmap would be copied or not.
                      *   Image rotation and mirror is limited when enabling. */
} esp_lcd_panel7in3_config_t;

class Panel7in3 {
public:
  Panel7in3(esp_lcd_panel_io_handle_t io,
            const esp_lcd_panel_dev_config_t *const panel_dev_config) {
    esp_lcd_new_panel_7in3e(panel_dev_config);
  }

  ~Panel7in3();

  Panel7in3(const Panel7in3 &) = delete;
  Panel7in3 &operator=(const Panel7in3 &) = delete;

  esp_err_t init();

  esp_err_t drawImage(const Epd::Framebuffer &framebuffer);

  esp_err_t refresh(const esp_lcd_panel_io_handle_t &io);
  esp_err_t waitIdle();

  esp_err_t waitUntilIdle();

  using BusyCompleteCallback = void (*)(void *user_ctx);

  esp_err_t registerBusyCompleteCallback(BusyCompleteCallback callback,
                                         void *user_ctx);

  Framebuffer &framebuffer() { return framebuffer_; }

  const Framebuffer &framebuffer() const { return framebuffer_; }

private:
  TickType_t timeout_ = pdMS_TO_TICKS(30000);

  esp_lcd_panel_t panel_{};
  esp_lcd_panel_handle_t panel_handle_ = nullptr;
  esp_lcd_panel_io_handle_t io_ = nullptr;

  gpio_num_t resetGpio_ = GPIO_NUM_NC;
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

  BusyCompleteCallback busyCompleteCallback_ = nullptr;
  void *busyCompleteUserCtx_ = nullptr;

  void sendCommand(const EpdCommands::Command &command);
  void sendData(const uint8_t *data, size_t size);

  esp_err_t esp_lcd_new_panel_7in3e(
      const esp_lcd_panel_dev_config_t *const panel_dev_config);

  esp_err_t sendDrawCommand(esp_lcd_panel_io_handle_t io,
                            const Framebuffer &framebuffer);
  esp_err_t sendRefreshCommand(esp_lcd_panel_io_handle_t io);
  esp_err_t sendPowerOnCommand(esp_lcd_panel_io_handle_t io);
  esp_err_t sendPowerOffCommand(esp_lcd_panel_io_handle_t io);

#ifdef CONFIG_EPD_BUSY_INTERRUPT_ENABLED
  static void busyIsrHandler(void *arg);

  void handleBusyInterrupt();
  // esp_err_t waitForRefreshPolling();
  // esp_err_t waitForRefreshInterrupt();

  esp_err_t configureBusyInterrupt();
  esp_err_t registerBusyInterrupt();
  SemaphoreHandle_t refreshDoneSemaphore_;
#endif // CONFIG_EPD_BUSY_INTERRUPT_ENABLED

  static esp_err_t panelReset(esp_lcd_panel_t *panel);
  esp_err_t reset();
};

} // namespace Epd
