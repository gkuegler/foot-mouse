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

// BRANCH NOTES:
// BRANCH-TASK: use the EEPROM library to access 1080 bytes for
// storing the secret phrase.
#include <EEPROM.h>
#include <Keyboard.h>
#include <Mouse.h>

#define BOUNCE_TIME    20   // milliseconds
#define RESIDENCE_TIME 3000 // milliseconds
#define MAX_STR_LENGTH 256

// NOTE: Opening the serial monitor on Arduino IDE will lock
// serial access two external scripts.
#define DEBUG 1

/*
UNIT DESCRIPTIONS:
Model number 1 is the pro-micro board (with 1 foot pedal &
mode switch). It's not supported by this sketch.
Model number 2 is the teensy unit with 3 pedals.

WARNING: the os will be hosed all of the button
pins are properly pulled high. If this happens:
1. Unplug microcontroller.
2. Put computer to sleep and wake up to reset modifier keypress.
3. Recompile fixed code and make sure Paul's loader tool is running.
4. Hold reset button while plugging microcontroller back in to pc.
*/

#define NUM_OF_PEDALS    3
#define PEDAL_PIN_LEFT   4
#define PEDAL_PIN_MIDDLE 5
#define PEDAL_PIN_RIGHT  6

#define PEDAL_DOWN 1
#define PEDAL_UP   0

// These constants are from Paul's mouse library 'Mouse.h'.
// I use these constants as my mode enum to save memory and
// for code brevity.
static_assert(MOUSE_LEFT == 1, "Mouse constants have changed.");
static_assert(MOUSE_RIGHT == 2, "Mouse constants have changed.");
static_assert(MOUSE_MIDDLE == 4, "Mouse constants have changed.");

enum PedalMode
{
  MODE_MOUSE_LEFT = MOUSE_LEFT,     // left mouse button
  MODE_MOUSE_RIGHT = MOUSE_RIGHT,   // right mouse button
  MODE_MOUSE_MIDDLE = MOUSE_MIDDLE, // middle mouse button
  MODE_MOUSE_DOUBLE = 8,            // double left-click
  MODE_CTRL_CLICK = 16,             // control left-click
  MODE_SCROLL_BAR = 32,             // uses special function
                                    // keys to trigger script running on desktop
  MODE_SCROLL_ANYWHERE = 64
};

enum MessageCode
{
  MSG_IDENTIFY = 4,
  MSG_SET_BUTTONS = 5,
  MSG_RESET_BUTTONS_TO_DEFAULT = 6,
  MSG_ECHO = 7,
  MSG_SEND_ASCII_KEYS = 8,
  MSG_SET_VAULT = 10,
  MSG_KEYBOARD_TYPE_VAULT = 11,
};

// The Arduino compiler does not like the Mozilla style for
// template declarations. I need to temporarily turn formatting
// off.
// clang-format off

// Template for debugging to serial monitor
template<class T>
void log(T msg)
// clang-format on
{
  if (DEBUG && Serial) {
    log(msg);
  }
}

class Button
{
public:
  int pin;
  int mode;
  int is_inverted;

  int default_mode;
  int default_is_inverted;

  int previous_mode;
  int previous_is_inverted;

  int state = 0;
  unsigned long last_change_time = 0;
  bool is_inactive = false;

  Button() = delete;
  Button(int pin_number, int mode_, int is_inverted_)
  {
    pin = pin_number;
    mode = mode_;
    is_inverted = is_inverted_;

    default_mode = mode_;
    default_is_inverted = is_inverted_;
  }

  void set_mode(int mode_, int is_inverted_)
  {
    mode = mode_;
    is_inverted = is_inverted_;
  }

  void reset_to_defaults()
  {
    // Clear any currently enabled buttons.
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
    is_inverted = default_is_inverted;
  }

  bool should_engage(int reading)
  {
    return (is_inverted && reading == PEDAL_UP) ||
           (!is_inverted && reading == PEDAL_DOWN);
  }
};

////////////////////////////////////////////////////////////////
//                      GLOBAL VARIABLES                      //
////////////////////////////////////////////////////////////////

// clang-format off
// Buttons and their defaults.
Button button_array[] = { 
  Button(PEDAL_PIN_LEFT, MODE_MOUSE_LEFT, true),
  Button(PEDAL_PIN_MIDDLE, MODE_MOUSE_MIDDLE, false),
  Button(PEDAL_PIN_RIGHT, MODE_MOUSE_RIGHT, false)
};
// clang-format on

constexpr const int k_buffer_size = MAX_STR_LENGTH;
byte g_input_buffer[k_buffer_size];

/*
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
  if (pedal_index < 0 || pedal_index >= NUM_OF_PEDALS) {
    return false;
  }
  // Intentionally not checking if above maximum possible value
  // for ease of development.
  if (mode <= 0) {
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
  log((char*)g_input_buffer);

  switch (message_code) {
    // Return an identifier code to confirm this is the board I
    // want to send serial commands to.
    case MSG_IDENTIFY:
      Serial.print("footmouse\n");
      break;

    // Reset all buttons to defaults.
    case MSG_RESET_BUTTONS_TO_DEFAULT:
      for (int i = 0; i < NUM_OF_PEDALS; i++) {
        button_array[i].reset_to_defaults();
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
      Serial.println((const char*)data);
      // Serial.println("echo test");
      break;

    // Change the mode of a pedal.
    case MSG_SET_BUTTONS:
      const auto pedal_index = static_cast<int>(data[0]);
      const auto mode = static_cast<int>(data[1]);
      const auto inversion = static_cast<int>(data[2]);

      if (valid_button_parameters(pedal_index, mode, inversion)) {
        button_array[pedal_index].set_mode(mode, inversion);
      }
      break;
  }
}

void
pedal_operation(int mode, bool engage)
{
  switch (mode) {
    // For left, right, and middle button modes, the mode enum
    // corresponds to the Mouse Library button constant value.
    case MODE_MOUSE_LEFT:
    case MODE_MOUSE_RIGHT:
    case MODE_MOUSE_MIDDLE:
      if (engage) {
        Mouse.press(mode);
      } else {
        Mouse.release(mode);
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
        Mouse.click(MOUSE_LEFT);
        delay(20);
        Keyboard.release(MODIFIERKEY_CTRL);
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
  }
}

void
setup()
{
  Serial.begin(9600);

  // Note: Not sure why don't need to call
  // Keyboard.begin() as well
  Mouse.begin();

  // Set up input pins.
  // I use external pull-up resistors, they are more stable.
  pinMode(PEDAL_PIN_LEFT, INPUT);
  pinMode(PEDAL_PIN_MIDDLE, INPUT);
  pinMode(PEDAL_PIN_RIGHT, INPUT);

  // For EEPROM testing.
  // set_vault("EEPROM test value");
}

void
loop()
{
  unsigned long current_time = millis();

  // Check each button.
  for (int i = 0; i < NUM_OF_PEDALS; i++) {
    auto& button = button_array[i];
    int reading = digitalRead(button.pin);

    // If the switch has changed, and it's been long enough since
    // the last  button press.
    if (reading != button.state &&
        (current_time - button.last_change_time) > BOUNCE_TIME) {

      button.is_inactive = false;
      button.state = reading;
      button.last_change_time = current_time;

      pedal_operation(button.mode, button.should_engage(reading));
    }
  }

  if (Serial.available()) {
    if (receive_serial_input()) {
      handle_message();
    }
  }
}
