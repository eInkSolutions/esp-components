#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sd_card_cxx.hpp"
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>

static const char *TAG = "TEST_APP";

static void listDirectory(const char *path) {
  DIR *dir = opendir(path);

  if (dir == nullptr) {
    ESP_LOGE(TAG, "Failed to open directory: %s", path);
    return;
  }

  ESP_LOGI(TAG, "Contents of %s:", path);

  struct dirent *entry;

  while ((entry = readdir(dir)) != nullptr) {
    ESP_LOGI(TAG, "  %s", entry->d_name);
  }

  closedir(dir);
}

static void readFile(const char *path) {
  FILE *file = fopen(path, "r");

  if (file == nullptr) {
    ESP_LOGE(TAG, "Failed to open file: %s", path);
    return;
  }

  ESP_LOGI(TAG, "Contents of %s:", path);

  char buffer[256];

  while (fgets(buffer, sizeof(buffer), file) != nullptr) {
    ESP_LOGI(TAG, "%s", buffer);
  }

  fclose(file);
}

extern "C" void app_main(void) {
  // INFO: These are the default values. The config object here can be removed
  // in favor of temporary default config.
  sdcard::Config config{
      .interface = sdcard::Interface::SDMMC,
      .mount_point = "/sdcard",
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
  };
  sdcard::SdCard sdcard(config);

  ESP_LOGI(TAG, "Starting SD card test");

  if (!sdcard.begin()) {
    ESP_LOGE(TAG, "Failed to mount SD card");
    return;
  }

  ESP_LOGI(TAG, "SD card mounted at %s", sdcard.mountPoint());
  // List the filesystem.
  listDirectory(sdcard.mountPoint());

  char file_path[128];
  snprintf(file_path, sizeof(file_path), "%s/test.txt", sdcard.mountPoint());
  readFile(file_path);

  ESP_LOGI(TAG, "Waiting 5 seconds before unmounting...");
  vTaskDelay(pdMS_TO_TICKS(5000));

  // Unmount.
  ESP_LOGI(TAG, "Unmounting SD card...");
  sdcard.end();

  ESP_LOGI(TAG, "Mounted: %s", sdcard.mounted() ? "yes" : "no");

  // Try accessing the filesystem after unmounting.
  ESP_LOGI(TAG, "Trying to access filesystem after unmount...");

  DIR *dir = opendir(sdcard.mountPoint());

  if (dir == nullptr) {
    ESP_LOGI(TAG, "Filesystem is no longer accessible -- unmount successful");
  } else {
    ESP_LOGW(TAG, "Filesystem is still accessible!");
    closedir(dir);
  }

  // Demonstrate that we can mount it again.
  ESP_LOGI(TAG, "Mounting SD card again...");

  if (sdcard.begin()) {
    ESP_LOGI(TAG, "Successfully mounted again");
    ESP_LOGI(TAG, "Mount point: '%s'", sdcard.mountPoint());
    listDirectory(sdcard.mountPoint());
  } else {
    ESP_LOGE(TAG, "Failed to mount SD card again");
  }
}
