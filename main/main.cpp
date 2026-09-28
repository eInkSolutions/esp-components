#include "esp_log.h"
#include <cstdio>

extern "C" void app_main() {
  ESP_LOGI("APP", "Hello from App");
  std::printf("Hello from C++!\n");
}
