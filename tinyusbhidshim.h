#pragma once

#include <Arduino.h>

#include <Adafruit_TinyUSB.h>

#include "tinyusbkeycodes.h"

#define USING_TINY_USB

namespace HIDCompat {

class KeyboardTinyUsbShim
{
public:
  void begin();
  void write(char c);
  // Also accepts ASCII keys.
  bool press(uint16_t k);
  bool release(uint16_t k);
  bool releaseAll();

private:
  uint8_t _mod = 0;
  uint8_t _keys[6] = { 0 };
  bool send_report();
  void add_key(uint8_t usage);
  void remove_key(uint8_t usage);
  uint8_t ascii_to_hid_usage_id(char c, uint8_t& out_mod);
};

class MouseTinyUsbShim
{
public:
  void begin();
  bool press(uint8_t buttons);
  bool release(uint8_t buttons);
  void click(uint8_t buttons = MOUSE_LEFT);

private:
  uint8_t _buttons = 0;
};
} // namespace HIDCompat