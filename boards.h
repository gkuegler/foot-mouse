#pragma once

#include <Arduino.h>

#if defined(TEENSYDUINO)
// Alternate defines for Teensyduino:
// defined(ARDUINO_TEENSY40) || defined(__IMXRT1062__)
// Note that NRF and ESP boards do not have EEPROM.
#define BOARD_TEENSY4
#elif defined(ARDUINO_ARCH_NRF52)
// Alternate defines for NRF architectures:
// defined(NRF52840_XXAA) || defined(ARDUINO_NRF52840)
#define BOARD_NRF52
#endif
