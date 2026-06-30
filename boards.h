#pragma once

#include <Arduino.h>

#if defined(TEENSYDUINO)
#define BOARD_TEENSY4
#elif defined(ARDUINO_ARCH_NRF52) // || defined(NRF52840_XXAA) ||
#define BOARD_NRF52
#endif
