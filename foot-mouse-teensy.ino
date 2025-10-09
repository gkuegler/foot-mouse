/*
BOARD INFO & COMPILE CONFIGURATION SETTINGS:
Teensy 4.0
USB Type: Serial + Keyboard + Mouse + Joystick
CPU Speed: 24MHz (set to minimum for efficiency)
Optimize: fastest (because why not)

CPU DETAILS:
little-endian
sizeof(integer) == 4 bytes

UPLOAD STEPS:
1. Set board family
2. Set USB type (links in needed libraries for mouse & serial)
3. Set desired clock speed
4. Set code optimization level
5. Use "Get Board Info" to find correct COM port
6. Verify & compile code
7. Push reset button on physical board (Paul's utility will
auto-upload on reset)

NOTE:
To get serial communications from the board,
you must set usb type under Tools -> Board -> USB Type to include
a Serial library.
e.g. "Serial + Keyboard + Mouse + Joystick"
you can't monitor serial and send from typical port

pedal up -> closes circuit -> pin pulled low
pedal down -> opens circuit -> pin pulled high via external
resistor

idea for scrolling mode:
1. send single byte message to server
2. server checks current coordinates of mouse
3. if mouse is in left-hand area of screen
4. send single bite message back
    to release the middle click
*/

#include <array>

#include <EEPROM.h>
#include <Keyboard.h>
#include <Mouse.h>

#include "button.h"
#include "constants.h"

#include "test-scan-codes.h"

// FUTURE: Since pedals are detected at startup, I might want to provide a map
// to determine what preset config the pedals are in depending on which are
// connected. This allows me to have say:
// Sheet music page turning mode if 1 & 3 connected.
// Or some other preset if only 2 & 3 are connected.

////////////////////////////////////////////////////////////////
//                      BOARD SELECTION                       //
////////////////////////////////////////////////////////////////

// Select the board and their Buttons and their defaults.

/*
UNIT DESCRIPTIONS:
Model number 1 is the pro-micro board (with 1 foot pedal &
mode switch). It's not supported by this sketch.
Model number 2 is the teensy unit with 3 pedals.
*/
#define FOOT_MOUSE_3_BUTTONS

#if defined(FOOT_MOUSE_3_BUTTONS)
std::array<Button, 3> buttons{ Button(4, MODE_MOUSE_LEFT, INVERTED),
                               Button(5, MODE_MOUSE_MIDDLE, NORMAL),
                               Button(6, MODE_MOUSE_RIGHT, NORMAL) };
/*
 * Dual stereo jacks
 */
#elif defined(FOOT_MOUSE_TRAVEL_4BTN)
std::array<Button, 3> buttons{ Button(4, MODE_MOUSE_LEFT, INVERTED),
                               Button(5, MODE_MOUSE_MIDDLE, NORMAL),
                               Button(6, MODE_MOUSE_RIGHT, NORMAL),
                               Button(6, MODE_MOUSE_ORBIT, NORMAL) };
#endif

////////////////////////////////////////////////////////////////
//                     GLOBAL VARIABLES                       //
////////////////////////////////////////////////////////////////

// TODO: change to standard array
constexpr const int k_buffer_size = MAX_STR_LENGTH;
byte g_input_buffer[k_buffer_size];

/**
 * Copy the contents of source null terminated
 * string into destination.
 * Return the number of characters copied.
 * Returns -1 if the buffer wasn't big enough.
 */
int
string_copy(char* destination, const char* source)
{
  for (int i = 0;; i++) {
    // Protect against buffer overruns.
    if (i >= MAX_STR_LENGTH) {
      return -1;
    }

    const auto c = source[i];
    destination[i] = c;
    if ('\0' == c) {
      return i;
    }
  }
};

/**
 * Check if the button parameters are valid.
 */
bool
valid_button_parameters(const int pedal_index,
                        const int mode,
                        const int inversion)
{
  if (pedal_index < 0 || pedal_index >= static_cast<int>(buttons.size())) {
    return false;
  }
  // Intentionally not checking if above maximum possible value
  // for ease of development.
  if (mode < 0) {
    return false;
  }
  if (inversion < 0 || inversion > 1) {
    return false;
  }
  return true;
}

/**
 * Save a string of characters to persistent storage.
 */
void
set_vault(const char* data)
{
  // Protect against buffer overruns.
  for (int i = 0; i < MAX_STR_LENGTH; i++) {
    byte c = data[i];
    EEPROM.update(i, c);
    if ('\0' == c) {
      break;
    }
  }
  return;
}

/**
 * Type out the contents of persistent storage
 * with keystrokes.
 */
void
type_vault()
{
  for (int i = 0; i < MAX_STR_LENGTH; i++) {
    auto c = (char)EEPROM.read(i);
    if ('\0' != c) {
      Keyboard.write(c);
    } else {
      break;
    }
  }
}

void
type_string(const char* text)
{
  for (int i = 0; i < MAX_STR_LENGTH; i++) {
    const char c = text[i];
    if ('\0' != c) {
      Keyboard.write(c);
    } else {
      break;
    }
  }
}

void
fire_macro(const uint16_t* keycodes, const size_t count)
{
  for (size_t i = 0; i < count; i++) {
    Keyboard.press(keycodes[i]);
  }

  delay(1);

  for (size_t i = 0; i < count; i++) {
    Keyboard.release(keycodes[i]);
  }
}

/**
 * Receiving serial input is used to change pedal mode
 * through the use of scripts on the host pc.
 *
 * Note that I use no log statements to debug serial
 * communications. I can't send serial to the teensy through the
 * normal ports when I'm monitoring serial through the Arduino
 * application.
 */
bool
receive_serial_input()
{
  constexpr byte start_marker = 16;
  constexpr byte end_marker = 17;
  bool found_start_marker = false;
  byte index = 0;
  byte rb;

  // Clear the input buffer.
  memset((void*)g_input_buffer, 0, k_buffer_size);

  // TODO: return the size of the data in the message?
  while (Serial.available() > 0) {
    rb = Serial.read();
    // Serial.print("get byte: ");
    // Serial.write(rb);
    // Serial.write("-\n");
    if (found_start_marker) {
      if (rb != end_marker && index < k_buffer_size) {
        g_input_buffer[index++] = rb;
      } else if (rb == end_marker && index <= k_buffer_size) {
        // A valid end marker was found before
        // the buffer was full.
        return true;
      } else {
        // Buffer full but no end marker.
        // Reject the message.
        // Serial.write("The message was too long.\n");
        return false;
      }
    } else if (rb == start_marker)
      found_start_marker = true;
  }

  // No start marker found or no serial was available
  // or message was not long enough
  return false;
}

// Possible future scheme using structs.
// struct __attribute__((packed)) ButtonParameters{};

/**
 * Decode and handle the message.
 */
void
handle_message()
{
  // First byte is the message code.
  // The remaining bytes are dependent on the message code.
  const int message_code = g_input_buffer[0];
  const byte* data = g_input_buffer + 1;

  switch (message_code) {
    // Return an identifier code to confirm this is the board I
    // want to send serial commands to.
    case MSG_IDENTIFY:
      Serial.print(DEVICE_ID_RESPONSE);
      break;

    // Reset all buttons to defaults.
    case MSG_RESET_BUTTONS_TO_DEFAULT:
      // Lets go of all keys currently pressed. See Keyboard.press().
      Keyboard.releaseAll();
      for (auto& b : buttons) {
        b.reset_to_defaults();
      }
      break;

    // Send hardware character keystrokes to computer.
    case MSG_SEND_ASCII_KEYS:
      type_string((const char*)data);
      break;

    // Load the null terminated c-string received into
    // the onboard EEPROM.
    case MSG_SET_VAULT:
      set_vault((const char*)data);
      break;

    case MSG_KEYBOARD_TYPE_VAULT:
      type_vault();
      break;

    // Echo back the message payload over serial.
    // Used for testing.
    case MSG_ECHO:
      // Return the sent string.
      Serial.println((const char*)data);
      break;

    // Change the mode of a pedal.
    case MSG_SET_BUTTONS: {
      auto mx = reinterpret_cast<const SetButtonMsg*>(data);

      if (valid_button_parameters(mx->pedal_index, mx->mode, mx->inversion)) {
        buttons[mx->pedal_index].set_mode(mx->mode, mx->inversion);
      }
      break;
    }
    case MSG_SET_BUTTONS_EX: {
      const byte pedal_index = data[0];
      // TODO: make this truth come from my message receive function
      const byte cnt = data[1];
      auto keycodes = reinterpret_cast<const uint16_t*>(&data[2]);

      // auto msg = reinterpret_cast<SetButtonMsgEx*>(data);

      auto& btn = buttons[pedal_index];

      if (cnt > btn.keycodes.size()) {
        Serial.println("Data is too big.");
        break;
      }

      memcpy(btn.keycodes.data(), keycodes, cnt * sizeof(uint16_t));
      btn.nKeycodes = cnt;
      btn.mode = MODE_KEYCOMBO;
      btn.inverted = 0;
      break;
    }
  }
}

void
send_input(int mode, bool engage, Button& btn)
{
  switch (mode) {
    // For left, right, and middle button modes, the mode enum
    // corresponds to the Mouse Library button constant value.
    case MODE_MOUSE_LEFT:
    case MODE_MOUSE_MIDDLE:
      if (engage) {
        Mouse.press(mode);
      } else {
        Mouse.release(mode);
      }
      break;

    case MODE_MOUSE_RIGHT:
      if (engage) {
        Mouse.click(MOUSE_RIGHT);
      }
      break;

    case MODE_MOUSE_DOUBLE:
      Mouse.click();
      Mouse.click();
      break;

    case MODE_CTRL_CLICK:
      if (engage) {
        Keyboard.press(MODIFIERKEY_CTRL);
        delay(20);
        Mouse.press(MOUSE_LEFT);
      } else {
        Keyboard.release(MODIFIERKEY_CTRL);
        Mouse.release(MOUSE_LEFT);
      }
      break;

    case MODE_SHIFT_CLICK:
      if (engage) {
        Keyboard.press(MODIFIERKEY_SHIFT);
        delay(20);
        Mouse.press(MOUSE_LEFT);
      } else {
        Keyboard.release(MODIFIERKEY_SHIFT);
        Mouse.release(MOUSE_LEFT);
      }
      break;

    case MODE_SHIFT_MIDDLE_CLICK:
      if (engage) {
        Keyboard.press(MODIFIERKEY_SHIFT);
        delay(20);
        Mouse.press(MOUSE_MIDDLE);
      } else {
        Keyboard.release(MODIFIERKEY_SHIFT);
        Mouse.release(MOUSE_MIDDLE);
      }
      break;

    // This hotkey locks the mouse to the left or right side of the
    // display where a scrollbar or a scroll map is.
    // Is implemented in my head tracking to mouse program
    // called TrackIRMouse.
    case MODE_SCROLL_BAR:
      Keyboard.press(KEY_F18);
      Keyboard.release(KEY_F18);
      break;

    // Autohotkey script used to trigger scrollwheel commans.
    // Scroll up/down messages are sent at a speed relative to
    // how far near the top or bottom my mouse pointer is.A
    case MODE_SCROLL_ANYWHERE:
      if (engage) {
        Keyboard.press(KEY_F20);
      } else {
        Keyboard.release(KEY_F20);
      }
      break;

    case MODE_ORBIT:
      if (engage) {
        Keyboard.press(MODIFIERKEY_SHIFT);
        delay(20);
        Mouse.press(MOUSE_MIDDLE);
      } else {
        Keyboard.release(MODIFIERKEY_SHIFT);
        Mouse.release(MOUSE_MIDDLE);
      }
      break;

    // Fire a preset key combo.
    case MODE_KEYCOMBO:
      if (engage) {
        fire_macro(btn.keycodes.data(), btn.nKeycodes);
      }
      break;
    default:
      break;
  }
}

void
setup()
{
  Serial.begin(9600);

  // Note both Mouse.begin() and Keyboard.begin() are empty methods, but it's
  // important to call them in case the implementation changes. '.begin()' is
  // arduino standard.
  Mouse.begin();
  Keyboard.begin();

  // Set up input pins.
  // I use external pull-up resistors, they are more stable.
  for (auto& btn : buttons) {
    pinMode(btn.pin, INPUT);

    // Disable a button if no pedal is plugged into the jack.
    // Alternatively, hold a pedal down on boot to disable that pedal.
    if (digitalRead(btn.pin) == DIGITAL_READ_DISCONNECTED_PEDAL) {
      btn.enabled = false;
    }
  }

  // For EEPROM testing.
  // set_vault("EEPROM test value");
}

unsigned long previous = 0;

// TESTING: Measuring elapsed time between loop iterations.
// 2025-03-31 Test Results:
// (24MHz Debug Build) Avg = 4.6 µs; Max = 12 µs; Min = 4 µs
// (600MHz Fastest with LTO) Avg = 0.1 µs; Max = 1 µs; Min = 0 µs
// #define TEST_LOOP_ELAPSED_TIME
#ifdef TEST_LOOP_ELAPSED_TIME
const int TIMES_ARRAY_SIZE = 128;
int idx_times_arr = 0;
unsigned long times[TIMES_ARRAY_SIZE];
#endif // TEST_LOOP_ELAPSED_TIME

void
loop()
{
  unsigned long now = micros();

#ifdef TEST_LOOP_ELAPSED_TIME
  times[idx_times_arr++] = now;
  if (idx_times_arr >= 128) {
    for (size_t i = 0; i < TIMES_ARRAY_SIZE; i++) {
      Serial.print(times[i]);
      Serial.print("\n");
    }
    Serial.print("Done printing times.\n");
    Serial.flush();
    abort();
  }
#endif // TEST_LOOP_ELAPSED_TIME

  if ((now - previous) > POLL_PERIOD_US) {
    previous = now;

    // Check each button.
    for (auto& btn : buttons) {
      if (btn.debounce(digitalRead(btn.pin), now)) {
        send_input(btn.mode, btn.should_engage(), btn);
      }
    }
  }

  if (Serial.available()) {

    if (receive_serial_input()) {
      handle_message();
    }
  }
}
