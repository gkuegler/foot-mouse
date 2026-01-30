#pragma once

#define AUTO_DISABLE_BTN_ON_START

#define GLITCH_SAMPLE_CNT 5
#define POLL_PERIOD_US    20
#define DEBOUNCE_RESET    20000 // microseconds
#define MAX_PAYLOAD_SIZE  512

#define KEEP_AWAKE_PERIOD_S 120
#define KEEP_AWAKE_KEY      KEY_F22

#define DEVICE_ID_RESPONSE "footmouse\n"

// Yahmaha foot pedal behaviors.
#define DIGITAL_READ_DISCONNECTED_PEDAL 1
#define DIGITAL_READ_PEDAL_DOWN         1
#define DIGITAL_READ_PEDAL_UP           0

#define NORMAL   false // press foot down to engage action
#define INVERTED true  // lift foot up to engage engage action

/**
 * PEDAL MODE DESCRIPTIONS:
 * MODE_MOUSE_LEFT: left mouse button
 * MODE_MOUSE_RIGHT: right mouse button
 * MODE_MOUSE_MIDDLE: middle mouse button
 * MODE_MOUSE_DOUBLE: double left-click
 * MODE_CTRL_CLICK: control left-click
 * MODE_SCROLL_BAR: Locks mouse to to the horizontal scroll bar area. The Teensy
 * sends one of the rarely function keys (F18) and a program running on the
 * desktop captures this keypress to control the cursor. MODE_SCROLL_ANYWHERE:
 * Triggers scroll wheel up/down events depending on the vertical position of
 * the cursor. The Teensy sends one of the rarely function keys (F20) and a
 * program running on the desktop captures this keypress to control the cursor.
 */
enum PedalMode
{
  MODE_NONE = 0,
  MODE_MOUSE_LEFT = 1,
  MODE_MOUSE_RIGHT = 2,
  MODE_MOUSE_MIDDLE = 4,
  MODE_MOUSE_DOUBLE = 8,
  MODE_CTRL_CLICK = 15,
  MODE_SHIFT_CLICK = 18,
  MODE_SHIFT_MIDDLE_CLICK = 19,
  MODE_SCROLL_BAR = 32,
  MODE_SCROLL_ANYWHERE = 64,
  MODE_FUNCTION = 65,
  MODE_ORBIT = 67,
  MODE_KEYCOMBO = 68
};

enum CmdCode
{
  CMD_IDENTIFY = 4,
  CMD_SET_BUTTON_MODE = 5,
  CMD_SET_KEYCOMBO = 51,
  CMD_RESET_BUTTONS_TO_DEFAULT = 6,
  CMD_ECHO = 7,
  CMD_SEND_ASCII_KEYS = 8,
  CMD_SET_VAULT = 10,
  CMD_KEYBOARD_TYPE_VAULT = 11,
  CMD_RETURN_CRC = 12,
  CMD_KEEP_AWAKE_ENABLE = 13,
  CMD_KEEP_AWAKE_DISABLE = 14
};