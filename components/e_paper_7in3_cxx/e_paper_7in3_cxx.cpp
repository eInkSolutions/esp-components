#include "e_paper_7in3_cxx.hpp"
#include "EpdCommands.hpp"
#include "EpdFramebuffer.hpp"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "lwip/priv/nd6_priv.h"
#include "soc/gpio_num.h"
#include <cstddef>
#include <new>
#include <sys/cdefs.h>

const static char *TAG = "EPD_DRIVER";

namespace Epd {
esp_err_t Panel7in3::init() {
#ifdef CONFIG_EPD_BUSY_INTERRUPT_ENABLED

  // make sure busy pin is not GPIO36 or GPIO39
  if (busyGpio_ == GPIO_NUM_36 || busyGpio_ == GPIO_NUM_39) {
    ESP_LOGE(TAG, "BUSY GPIO %d is not supported", busyGpio_);
    return ESP_ERR_INVALID_ARG;
  }

  ESP_RETURN_ON_ERROR(configureBusyInterrupt(), TAG,
                      "Failed to configure BUSY GPIO");

  ESP_RETURN_ON_ERROR(registerBusyInterrupt(), TAG,
                      "Failed to register BUSY ISR");

  // make sure busy pin is not GPIO36 or GPIO39
  if (busyGpio_ == GPIO_NUM_36 || busyGpio_ == GPIO_NUM_39) {
    ESP_LOGE(TAG, "BUSY GPIO %d is not supported", busyGpio_);
    return ESP_ERR_INVALID_ARG;
  }

  // Keep the interrupt disabled until a refresh starts.
  ESP_RETURN_ON_ERROR(gpio_intr_disable(busyGpio_), TAG,
                      "Failed to disable BUSY interrupt");

#endif // CONFIG_EPD_BUSY_INTERRUPT_ENABLED

  return ESP_OK;
}

#ifdef CONFIG_EPD_BUSY_INTERRUPT_ENABLED

void Panel7in3::busyIsrHandler(void *arg) {
  auto *panel = static_cast<Panel7in3 *>(arg);

  panel->handleBusyInterrupt();
}

esp_err_t Panel7in3::configureBusyInterrupt() {
  gpio_config_t config = {};

  config.pin_bit_mask = 1ULL << busyGpio_;
  config.mode = GPIO_MODE_INPUT;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;

  // BUSY: LOW = busy, HIGH = idle.
  // Trigger when refresh finishes: LOW -> HIGH.
  config.intr_type = GPIO_INTR_POSEDGE;

  return gpio_config(&config);
}

esp_err_t Panel7in3::registerBusyInterrupt() {
  return gpio_isr_handler_add(busyGpio_, Panel7in3::busyIsrHandler, this);
}

void Panel7in3::handleBusyInterrupt() {
  BaseType_t higherPriorityTaskWoken = pdFALSE;

  xSemaphoreGiveFromISR(refreshDoneSemaphore_, &higherPriorityTaskWoken);

  if (higherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}
#endif // CONFIG_EPD_BUSY_INTERRUPT_ENABLED

esp_err_t Panel7in3::waitUntilIdle() {
#ifdef CONFIG_EPD_BUSY_INTERRUPT_ENABLED

  // Discard stale notifications from previous waits.
  while (xSemaphoreTake(refreshDoneSemaphore_, 0) == pdTRUE) {
  }

  // Enable the interrupt only while waiting for BUSY.
  ESP_RETURN_ON_ERROR(gpio_intr_enable(busyGpio_), TAG,
                      "Failed to enable BUSY interrupt");

  // The display may already be idle.
  if (gpio_get_level(busyGpio_) != 0) {
    ESP_RETURN_ON_ERROR(gpio_intr_disable(busyGpio_), TAG,
                        "Failed to disable BUSY interrupt");
    return ESP_OK;
  }

  // Wait for the ISR to notify us that BUSY went HIGH.
  if (xSemaphoreTake(refreshDoneSemaphore_, timeout_) != pdTRUE) {
    gpio_intr_disable(busyGpio_);
    return ESP_ERR_TIMEOUT;
  }

  ESP_RETURN_ON_ERROR(gpio_intr_disable(busyGpio_), TAG,
                      "Failed to disable BUSY interrupt");

#else

  while (gpio_get_level(busyGpio_) == 0) {
    vTaskDelay(pdMS_TO_TICKS(15));
  }

#endif

  // Only call this after successful completion.
  if (busyCompleteCallback_) {
    busyCompleteCallback_(busyCompleteUserCtx_);
  }

  return ESP_OK;
}

esp_err_t Panel7in3::refresh(const esp_lcd_panel_io_handle_t &io) {
  ESP_RETURN_ON_ERROR(sendRefreshCommand(io), TAG, "Failed to start refresh");

  ESP_RETURN_ON_ERROR(waitUntilIdle(), TAG, "Refresh handling error");

  return ESP_OK;
}

esp_err_t Panel7in3::drawImage(const Epd::Framebuffer &framebuffer) {

  esp_lcd_panel_io_handle_t io = NULL;

  // Make sure display is turned on
  ESP_RETURN_ON_ERROR(sendPowerOnCommand(io), TAG,
                      "error sending PowerOff command to epaper display.");
  ESP_RETURN_ON_ERROR(waitUntilIdle(), TAG, "refresh handling error");

  // TODO: implement more

  ESP_RETURN_ON_ERROR(sendDrawCommand(io, framebuffer), TAG,
                      "data framebuffer transmition err");

  // Refresh screen here
  ESP_RETURN_ON_ERROR(refresh(io), TAG,
                      "error sending refresh command to e-paper");

  // Turn off display to save consumption and not burn epaper display
  ESP_RETURN_ON_ERROR(sendPowerOffCommand(io), TAG,
                      "error sending PowerOff command to epaper display.");
  ESP_RETURN_ON_ERROR(waitUntilIdle(), TAG, "refresh handling error");
  return ESP_OK;
}

esp_err_t Panel7in3::sendDrawCommand(esp_lcd_panel_io_handle_t io,
                                     const Framebuffer &framebuffer) {
  EpdCommands::Command startCommand = EpdCommands::DataStart;

  ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(io, (int)startCommand.opcode,
                                                framebuffer.data(),
                                                framebuffer.size()),
                      TAG, "data framebuffer transmition err");

  return ESP_OK;
}

esp_err_t Panel7in3::sendRefreshCommand(esp_lcd_panel_io_handle_t io) {
  EpdCommands::Command refreshCommand = EpdCommands::DataRefresh;

  ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, (int)refreshCommand.opcode,
                                                refreshCommand.data,
                                                refreshCommand.dataSize),
                      TAG, "refresh command error");
  return ESP_OK;
}

esp_err_t Panel7in3::sendPowerOffCommand(esp_lcd_panel_io_handle_t io) {
  EpdCommands::Command powerOffCommand = EpdCommands::PowerOff;

  ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, (int)powerOffCommand.opcode,
                                                powerOffCommand.data,
                                                powerOffCommand.dataSize),
                      TAG, "power off command error");
  return ESP_OK;
}

esp_err_t Panel7in3::sendPowerOnCommand(esp_lcd_panel_io_handle_t io) {
  EpdCommands::Command powerOnCommand = EpdCommands::PowerOn;

  ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, (int)powerOnCommand.opcode,
                                                powerOnCommand.data,
                                                powerOnCommand.dataSize),
                      TAG, "power on command error");
  return ESP_OK;
}

esp_err_t Panel7in3::registerBusyCompleteCallback(BusyCompleteCallback callback,
                                                  void *user_ctx) {
  if (callback == nullptr) {
    ESP_LOGE(TAG, "Refresh-done callback must not be null");
    return ESP_ERR_INVALID_ARG;
  }

  busyCompleteCallback_ = callback;
  busyCompleteUserCtx_ = user_ctx;

  return ESP_OK;
}

esp_err_t Panel7in3::panelReset(esp_lcd_panel_t *panel) {
  auto *self = __containerof(panel, Panel7in3, panel_);
  return self->reset();
}

esp_err_t Panel7in3::reset() {
  if (resetGpio_ == GPIO_NUM_NC) {
    return ESP_ERR_INVALID_STATE;
  }

  ESP_RETURN_ON_ERROR(gpio_set_level(resetGpio_, resetLevel_), TAG,
                      "Failed to set reset GPIO");
  vTaskDelay(pdMS_TO_TICKS(20));

  ESP_RETURN_ON_ERROR(gpio_set_level(resetGpio_, !resetLevel_), TAG,
                      "Failed to set reset GPIO");
  vTaskDelay(pdMS_TO_TICKS(2));

  ESP_RETURN_ON_ERROR(gpio_set_level(resetGpio_, resetLevel_), TAG,
                      "Failed to set reset GPIO");
  vTaskDelay(pdMS_TO_TICKS(20));

  return waitUntilIdle();
}

esp_err_t Panel7in3::esp_lcd_new_panel_7in3e(
    const esp_lcd_panel_dev_config_t *const panel_dev_config) {

  ESP_RETURN_ON_FALSE(panel_dev_config, ESP_ERR_INVALID_ARG, TAG,
                      "panel_dev_config is NULL");

  auto *panel = new (std::nothrow) esp_lcd_panel_t{};

  ESP_RETURN_ON_FALSE(panel, ESP_ERR_NO_MEM, TAG,
                      "Failed to allocate panel context");

  const auto *epaper_config = static_cast<const esp_lcd_panel7in3_config_t *>(
      panel_dev_config->vendor_config);

  ESP_RETURN_ON_FALSE(epaper_config, ESP_ERR_INVALID_ARG, TAG,
                      "Missing e-paper configuration");

  // panel int function and other fields

  busyGpio_ = static_cast<gpio_num_t>(epaper_config->busy_gpio_num);
  resetGpio_ = static_cast<gpio_num_t>(panel_dev_config->reset_gpio_num);
  resetLevel_ = panel_dev_config->flags.reset_active_high;
  busyLevel_ = false;

  // Register the ESP-IDF interface callbacks.
  panel_.del = panelDel;
  panel_.reset = panelReset;
  panel_.init = panelInit;
  panel_.draw_bitmap = panelDrawBitmap;
  panel_.invert_color = panelInvertColor;
  panel_.set_gap = panelSetGap;
  panel_.mirror = panelMirror;
  panel_.swap_xy = panelSwapXY;
  panel_.disp_on_off = panelDispOnOff;

  panel_handle_ = &panel_;

  return ESP_OK;
}
} // namespace Epd
