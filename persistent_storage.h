#pragma once

#include <Arduino.h>

#include "constants.h"

namespace eeprom {

struct MemButton
{
  byte mode;
  byte trig_direction;
  byte is_initialized;
} __attribute__((packed));

template<int BUTTON_COUNT>
struct MemoryView
{
  std::array<MemButton, BUTTON_COUNT> buttons;
} __attribute__((packed));

#if defined(BOARD_TEENSY4)
#include <EEPROM.h>

class PersistentButtonStorage
{
  const size_t _mem_loc = 1;

public:
  PersistentButtonStorage() {};
  ~PersistentButtonStorage() {};

  void invalidate_buttons(size_t count)
  {
    const size_t member_offset = 2;

    for (size_t i = 0; i < count; i++) {
      EEPROM.update(_mem_loc + (sizeof(MemButton) * i) + member_offset, 0);
    }
  }

  MemButton get_button(int idx)
  {
    int start = _mem_loc + (sizeof(MemButton) * idx);
    byte mode = EEPROM.read(start++);
    byte trig_direction = EEPROM.read(start++);
    byte is_initialized = EEPROM.read(start++);
    return MemButton{ mode, trig_direction, is_initialized };
  }

  bool set_button(int idx, byte mode, byte trig_direction)
  {
    int start = _mem_loc + (sizeof(MemButton) * idx);
    EEPROM.update(start++, mode);
    EEPROM.update(start++, trig_direction);
    EEPROM.update(start++, 1);
    return true;
  }
};

#elif defined(ARDUINO_ARCH_NRF52) // || defined(NRF52840_XXAA) ||
class PersistentButtonStorage
{
public:
  PersistentButtonStorage() {};
  ~PersistentButtonStorage() {};

  void invalidate_buttons(size_t count) {};
  MemButton get_button(int idx) { return MemButton{ 0, 0, 0 }; }
  bool set_button(int idx, byte mode, byte trig_direction) { return true; }
};
#endif
}