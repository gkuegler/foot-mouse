#ifndef FOOTMOUSE_BUTTON_H
#define FOOTMOUSE_BUTTON_H

#include "constants.h"

class Button
{
public:
  int pin;
  int mode;
  int inverted;
  bool enabled = true;

  size_t nKeycodes;
  std::array<uint16_t, 128> keycodes;
  // unsigned long timout_ms = 3 * 60 * 1000;

  const int default_mode;
  const int default_inverted;

  int state = DIGITAL_READ_PEDAL_UP;
  uint32_t glitch_buf = 0;
  unsigned long last_change_time = 0;

  Button() = delete;
  Button(int pin, int mode, int inverted)
    : pin(pin)
    , mode(mode)
    , inverted(inverted)
    , default_mode(mode)
    , default_inverted(inverted)
  {
  }

  void set_mode(int mode_, int inverted_)
  {
    mode = mode_;
    inverted = inverted_;
  }

  void reset_to_defaults()
  {
    // Release any potenitally help down mouse buttons.
    switch (mode) {
      // For left, right, and middle button modes, the mode
      // number corresponds  to the Mouse library button
      // constant.
      case MODE_MOUSE_LEFT:
      case MODE_MOUSE_RIGHT:
      case MODE_MOUSE_MIDDLE:
        Mouse.release(mode);
        break;
    }
    mode = default_mode;
    inverted = default_inverted;
  }

  /**
   * Apply inversion settings to the pedal position
   */
  bool should_engage()
  {
    return (inverted && (state == DIGITAL_READ_PEDAL_UP)) ||
           (!inverted && (state == DIGITAL_READ_PEDAL_DOWN));
  }

  /**
   * De-Bouncing Filter.
   * Return true if an action should be triggered.
   */
  bool debounce(int digital_read, unsigned long now)
  {
    if (!enabled) {
      return false;
    }

    // Can't implement a timeout feature without edge detection rather than
    // state detection. if ((now - last_change_time) > timout_ms) {
    //   // TODO: this will break button behavior if I use edge debouncing
    //   // detection.
    //   state = !state;
    // }

    // The bit mask is responsible for ignoring short glitches on the GPIO pins.
    // A minimum number of sequential samples must be all high or all low to
    // change state. The glitch duration is determined by POLL_PERIOD_US *
    // GLITCH_SAMPLE_CNT.
    const uint32_t mask =
      static_cast<uint32_t>(-1) >> ((sizeof(uint32_t) * 8) - GLITCH_SAMPLE_CNT);

    glitch_buf = mask & ((glitch_buf << 1) | digital_read);

    // The debounce reset is used to acheive longer debounce times on the pedal
    // reset.
    if ((now - last_change_time) < DEBOUNCE_RESET) {
      return false;
    }

    if (state == 1 && glitch_buf == 0) {
      state = 0;
      last_change_time = now;
      return true;
    } else if (state == 0 && glitch_buf == mask) {
      state = 1;
      last_change_time = now;
      return true;
    }

    return false;
  }
};

#endif // FOOTMOUSE_BUTTON_H