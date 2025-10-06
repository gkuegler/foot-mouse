#ifndef FOOT_MOUSE_CONSTANTS_H
#define FOOT_MOUSE_CONSTANTS_H

#include <Keyboard.h>
#include <Mouse.h>

#define GLITCH_SAMPLE_CNT 5
#define POLL_PERIOD_US    20
#define DEBOUNCE_RESET    20000 // microseconds
#define MAX_STR_LENGTH    256

// NOTE: Opening the serial monitor on Arduino IDE will lock
// serial access two external scripts.
#define DEBUG 1

#define DEVICE_ID_RESPONSE "footmouse\n"

// Yahmaho foot pedal behaviors.
#define DIGITAL_READ_DISCONNECTED_PEDAL 1
#define DIGITAL_READ_PEDAL_DOWN         1
#define DIGITAL_READ_PEDAL_UP           0

#define NORMAL   false // press foot down to engage action
#define INVERTED true  // lift foot up to engage engage action

// Tempoary keyboard shortcut programming. The special key is used to send
// keystrokes to the computer (i.e. F24) that can't be replicated on my keyboard
// when I am trying to set keyboard shortcuts on devices (and in programs) that
// require the key to be pressed and captured. For example, programming Logitech
// mouses.
// #define SPECIAL_KEY KEY_F23
// #define PROGRAM_SPECIAL

// These constants are from Paul's mouse library 'Mouse.h'.
// I use these constants as my mode enum to save memory and
// for code brevity.
static_assert(MOUSE_LEFT == 1, "Mouse constants have changed.");
static_assert(MOUSE_RIGHT == 2, "Mouse constants have changed.");
static_assert(MOUSE_MIDDLE == 4, "Mouse constants have changed.");

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
  MODE_MOUSE_LEFT = MOUSE_LEFT,
  MODE_MOUSE_RIGHT = MOUSE_RIGHT,
  MODE_MOUSE_MIDDLE = MOUSE_MIDDLE,
  MODE_MOUSE_DOUBLE = 8,
  MODE_CTRL_CLICK = 15,
  MODE_SHIFT_CLICK = 18,
  MODE_SHIFT_MIDDLE_CLICK = 19,
  MODE_SCROLL_BAR = 32,
  MODE_SCROLL_ANYWHERE = 64,
  MODE_FUNCTION = 65,
  MODE_ORBIT = 67
};

enum MessageCode
{
  MSG_IDENTIFY = 4,
  MSG_SET_BUTTONS = 5,
  MSG_RESET_BUTTONS_TO_DEFAULT = 6,
  MSG_ECHO = 7,
  MSG_SEND_ASCII_KEYS = 8,
  MSG_SET_VAULT = 10,
  MSG_KEYBOARD_TYPE_VAULT = 11
};

#endif // FOOT_MOUSE_CONSTANTS_H