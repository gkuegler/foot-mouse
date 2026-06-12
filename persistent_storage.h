#pragma once

#include <Arduino.h>

#include <EEPROM.h>

#include "constants.h"

// constexpr int flashed_index = 0;
constexpr int starting_index = 0;

struct MemButton
{
  uint8_t mode;
  uint8_t trig_direction;
} __attribute__((packed));

template<int BUTTON_COUNT>
struct MemoryView
{
  std::array<MemButton, BUTTON_COUNT> buttons;
} __attribute__((packed));

bool
is_memory_initialized()
{
  return EEPROM[flashed_index];
}

void
load_memory(uint8_t* buf, size_t size)
{
  // Don't load settings if mem isn't initialized.
  // if (EEPROM[flashed_index] == 0) {
  //   EEPROM.write(0, 1);
  //   return;
  // }

  for (size_t i = 0; i < size; i++) {
    buf[i] = EEPROM[i];
  }
}

void
update_memory(uint8_t* buf, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    EEPROM.update(i, buf[i]);
  }
}

// MemButton
// get_button(int idx)
// {
//   const int start = starting_index + (sizeof(MemButton) * idx);
//   int mode = EEPROM.read(start);
//   int trig_direction = EEPROM.read(start + 1);
//   return MemButton{ mode, trig_direction };
// }

// bool
// set_button(int idx, int mode, int trig_direction)
// {
//   const int start = starting_index + (sizeof(MemButton) * idx);
//   EEPROM.update(start, mode);
//   EEPROM.update(start + 1, trig_direction);
// }
