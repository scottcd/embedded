#ifndef BOARD_PROFILE_H
#define BOARD_PROFILE_H

#include "driver/gpio.h"

#define BOARD_PROFILE_NAME "esp32-s3-n16-r8"

#define BOARD_FLASH_SIZE_MB 16
#define BOARD_PSRAM_SIZE_MB 8

// Default reference-app assumption for an unconnected DHT22 bring-up.
// Change this once the actual wiring is known for your board.
#define BOARD_DHT22_GPIO GPIO_NUM_4
#define BOARD_DHT22_READ_INTERVAL_MS 5000

#endif
