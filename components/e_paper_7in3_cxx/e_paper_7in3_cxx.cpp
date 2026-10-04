#include "e_paper_7in3_cxx.hpp"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_err.h"

const static char *TAG = "EPD_DRIVER";

namespace Epd {
esp_err_t Panel7in3::init() {
#ifdef CONFIG_EPD_BUSY_INTERRUPT_ENABLED

  ESP_RETURN_ON_ERROR(configureBusyInterrupt(), TAG,
                      "Failed to configure BUSY GPIO");

  ESP_RETURN_ON_ERROR(registerBusyInterrupt(), TAG,
                      "Failed to register BUSY ISR");

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

esp_err_t Panel7in3::waitForRefreshComplete() {
#ifdef CONFIG_EPD_BUSY_INTERRUPT_ENABLED

  if (xSemaphoreTake(refreshDoneSemaphore_, portMAX_DELAY) != pdTRUE) {
    return ESP_FAIL;
  }

  return ESP_OK;

#else

  while (gpio_get_level(busyGpio_) == 0) {
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  return ESP_OK;

#endif
}

} // namespace Epd
