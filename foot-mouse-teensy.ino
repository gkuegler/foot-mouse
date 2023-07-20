/*
BOARD INFO & CONFIGURATION SETTINGS:
Teensy 4.0
USB Type: Serial + Keyboard + Mouse + Joystick
CPU Speed: 24MHz (set to minimum for efficiency)
Optimize: fastest (because why not)
LITTLE-ENDIAN

Upload Steps:
1. Set board family
2. Set USB type (links in needed libraries for mouse & serial)
3. Set desired clock speed
4. Set code optimization level
5. Use "Get Board Info" to find correct COM port
6. Verify & compile code
7. Push reset button on physical board (Paul's utility will auto-upload on
reset)

NOTE:
To get serial communications from the board,
you must set usb type under Tools -> Board -> USB Type to include
a Serial library.
e.g. "Serial + Keyboard + Mouse + Joystick"
you can't monitor serial and send from typical port

pedal up -> closes circuit -> pin pulled low
pedal down -> opens circuit -> pin pulled high via external resistor

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

#define BOUNCE_TIME 20      // milliseconds
#define RESIDENCE_TIME 3000 // milliseconds

/*
UNIT DESCRIPTIONS:
model number 2 is the at-home unit (with 2 foot pedals)
model number 3 is the at-work unit (with 3 foot pedals)
*/
#define MODEL_3_PEDAL 3
#define MODEL_2_PEDAL 2
#define MODEL_1_PEDAL 1

#define MODEL_NUMBER MODEL_3_PEDAL

// WARNING: the os will be hosed if NUM_OF_PEDALS does not match
// the  number of  pedals are not physically plugged into the
// unit. If this  happens:
//   1. unplug microcontroller
//   2. put computer to sleep and wake up to reset modifier keypress
//   3. recompile fixed code and make sure Paul's loader tool is running
//   4. hold reset button while plugging microcontroller back in to pc
//   5. press reset button within 1 second of being plugged
//       in (just to be sure)
// TODO: add an initial check to only activate the pedals one it
// changes  value

#if MODEL_NUMBER == 2
constexpr int NUM_OF_PEDALS = 2;
#define BOARD_ID 3;
#define LBUTTON_PIN 2
#define MBUTTON_PIN 5
#define RBUTTON_PIN 7
#endif // MODEL_NUMBER

#if MODEL_NUMBER == 3
constexpr int NUM_OF_PEDALS = 3; // Set to 3 to enable right click
#define BOARD_ID 3;
#define LBUTTON_PIN 4
#define MBUTTON_PIN 5
#define RBUTTON_PIN 6
#endif // MODEL_NUMBER

#define PEDAL_DOWN 1
#define PEDAL_UP 0

// Opening the serial monitor on Arduino will
// prevent outside scripts from sending serial
// messages to the board.
#define DEBUG 0

// These are from Paul's mouse library.
// I use the mode as the button to press because of legacy reasons.
static_assert(MOUSE_LEFT == 1, "Voice commands will fail");
static_assert(MOUSE_MIDDLE == 4, "Voice commands will fail");
static_assert(MOUSE_RIGHT == 2, "Voice commands will fail");

// BRANCH-TASK:
enum Mode
{
  MODE_MOUSE_LEFT = MOUSE_LEFT,
  MODE_MOUSE_RIGHT = MOUSE_RIGHT,
  MODE_MOUSE_MIDDLE = MOUSE_MIDDLE,
  MODE_MOUSE_DOUBLE = 8,
  MACRO_CTRL_CLICK = 16,
  MACRO_SCROLL = 32,
};

#define PEDAL_1_MODE MOUSE_LEFT
#define PEDAL_2_MODE MOUSE_MIDDLE
#define PEDAL_3_MODE MOUSE_RIGHT

enum MessageCode
{
  MSG_IDENTIFY = 4,
  MSG_SET_BUTTONS = 5,
  MSG_CLEAR_BUTTONS = 6,
  MSG_SET_SECRET = 7,
  MSG_KEYBOARD_TYPE_SECRET = 8,
  MSG_ECHO = 9
};
// clang-format on

// The Arduino compiler does not like the Mozilla style for template
// declarations

// Template for debugging to serial monitor
// template<class T>
// void
// log(unsigned char* msg)
// {
//   if (DEBUG && Serial) {
//     // if DEBUG is on this will fail if no serial monitor
//     Serial.println(msg);
//   }
// }

// void
// log(int msg)
// {
//   if (DEBUG && Serial) {
//     // if DEBUG is on this will fail if no serial monitor
//     Serial.println(msg);
//   }
// }

// void
// log(void* msg)
// {
//   if (DEBUG && Serial) {
//     // if DEBUG is on this will fail if no serial monitor
//     Serial.println(msg);
//   }
// }

class Button
{
public:
  int pin;
  int mode;
  int is_inverted;

  int default_mode;
  int default_is_inverted;

  // The way this code is written, the button state doesn't matter,
  // only if it changes.
  int state = 0;
  unsigned long last_debounce_time = 0;
  bool is_inactive = false;

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
    switch (mode) {
      // For left, right, and middle button modes, the mode
      // number corresponds  to the Mouse library button constant.
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
Button button_array[] = {  // button defaults
  Button(LBUTTON_PIN, PEDAL_1_MODE, true),
  Button(MBUTTON_PIN, PEDAL_2_MODE, false),
  Button(RBUTTON_PIN, PEDAL_3_MODE, false)
};
// clang-format on

// BRANCH-TASK: create a secret buffer
constexpr const int k_secret_size = 256;
byte g_secret_buffer[k_secret_size];

constexpr const int k_buffer_size = 256;
byte g_input_buffer[k_buffer_size];

////////////////////////////////////////////////////////////////

// clang-format off
template<class T>
void clear_array(T& array, int size) {
  // clang-format on
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}

int
strcpy(byte* destination, const byte* source)
{
  for (int i = 0;; i++) {
    const byte c = source[i];
    if ('\0' != c) {
      destination[i] = c;
    } else {
      return i;
    }
  }
};

bool
valid_button_parameters(const int pedal_index,
                        const int mode,
                        const int inversion)
/**
 * Check if the button parameters are valid.
 */
{
  if (pedal_index < 0 || pedal_index >= NUM_OF_PEDALS) {
    return false;
  }
  if (mode <= 0) { // not checking if above maximum mode constant value
    return false;
  }
  if (inversion < 0 || inversion > 1) {
    return false;
  }
  return true;
}

bool
set_secret(const byte* data)
{
  clear_array(g_secret_buffer, k_secret_size);
  if (strcpy(g_secret_buffer, data) == 0) {
    return false;
  } else {
    return true;
  }
}

byte*
get_secret()
{
  return g_secret_buffer;
}

/**
 * Receiving serial input is used to change pedal mode
 * through the use of scripts on the host pc.
 *
 * Note that I use no log statements to debug serial communications.
 * I can't send serial to the teensy through the normal ports when I'm
 * monitoring serial through the Arduino application.
 */

bool
receive_serial_input()
{
  constexpr byte start_marker = 16;
  constexpr byte end_marker = 17;
  bool found_start_marker = false;
  byte index = 0;
  byte rb;

  if (Serial.available()) {
    clear_array(g_input_buffer, k_buffer_size);
  }

  // TODO: return the size of the data in the message
  while (Serial.available() > 0) {
    rb = Serial.read();
    if (found_start_marker) {
      if (rb != end_marker && index < k_buffer_size) {
        g_input_buffer[index++] = rb;
      } else if (rb == end_marker && index <= k_buffer_size) {
        // buffer is full and valid end marker was found after body
        return true;
      } else {
        // buffer full but no end marker
        // or end marker found with an incomplete buffer
        // reject message
        // Serial.write("The message was too long.\n");
        return false;
      }
    } else if (rb == start_marker)
      found_start_marker = true;
  }

  // no start marker found or no serial was available
  // or message was not long enough
  return false;
}

struct __attribute__((packed)) MSG_STRUCT
{
  byte message_code;
  byte data[255];
};

struct __attribute__((packed)) MSG_ECHO_STRUCT
{
  const char* msg;
};

// struct __attribute__((packed)) ButtonParameters
// {
//   const byte pedal_index;
//   const byte mode;
//   const byte inversion;
// };

class ButtonParameters
{
public:
  const int pedal_index;
  const int mode;
  const int inversion;
  ButtonParameters(byte* data)
    : pedal_index(static_cast<const int>(data[0]))
    , mode(static_cast<const int>(data[1]))
    , inversion(static_cast<const int>(data[2]))
  {
  }
};

void
parse_message()
{
  // First byte is the message code.
  // The remaining (3) bytes are dependent on the message code.
  byte* data = g_input_buffer;
  // HEADER
  int offset = 1;
  const auto message_code = data[0];
  // TODO: convert to switch statement
  // switch (name)
  if (MSG_IDENTIFY == message_code) {
    // Return an identifier code
    // I only use one board at a time.
    Serial.write("footmouse\n");
  } else if (MSG_CLEAR_BUTTONS == message_code) {
    // Special case command to reset to defaults.
    for (int i = 0; i < NUM_OF_PEDALS; ++i) {
      button_array[i].reset_to_defaults();
    }
  } else if (MSG_SET_SECRET == message_code) {
    // TASK: enable variable length messages up to a max byte count.
    // set_secret(static_cast<const byte*>(&g_input_buffer + 1))
  } else if (MSG_KEYBOARD_TYPE_SECRET == message_code) {
    // An empty block
  } else if (MSG_ECHO == message_code) {
    // Serial.write("footmouse\n");
    // const char* message = reinterpret_cast<char*>(&g_input_buffer + 1);
    // auto data = reinterpret_cast<MSG_ECHO_STRUCT*>(g_input_buffer + 1);
    Serial.print(reinterpret_cast<char*>(data + offset));
    // Serial.write("\n");
    // log("reinterpret_cast message:");
    // log(message);
  } else if (MSG_SET_BUTTONS == message_code) {
    const auto pedal_index = static_cast<int>(data[offset]);
    const auto mode = static_cast<int>(data[offset + 1]);
    const auto inversion = static_cast<int>(data[offset + 2]);

    if (valid_button_parameters(pedal_index, mode, inversion)) {
      button_array[pedal_index].set_mode(mode, inversion);
    }
  }
}

void
pedal_operation(int mode, bool engage)
{
  switch (mode) {
    // for left, right, and middle button modes, the mode number
    // corresponds to the Mouse library button constant
    case MODE_MOUSE_LEFT:
    case MODE_MOUSE_RIGHT:
    case MODE_MOUSE_MIDDLE:
      if (engage) {
        Mouse.press(mode);
      } else {
        Mouse.release(mode);
      }
      break;
    case MODE_MOUSE_DOUBLE: // double-click
      Mouse.click();
      Mouse.click();
      break;
    case MACRO_CTRL_CLICK: // fire off the keyboard macro
      if (engage) {
        Keyboard.press(MODIFIERKEY_CTRL);
        delay(20);
        Mouse.click(MOUSE_LEFT);
        delay(20);
        Keyboard.release(MODIFIERKEY_CTRL);
      }
      break;
    case MACRO_SCROLL: // toggle scrolling with desktop program
      Keyboard.press(KEY_F18);
      Keyboard.release(KEY_F18);
      break;
  }
}

void
setup()
{
  Serial.begin(9600);
  // log("starting foot-mouse");

  // not sure why don't need to call Keyboard.begin() as well
  Mouse.begin();

  // I use external pull-up resistors, I find they are more reliable
  pinMode(LBUTTON_PIN, INPUT);
  pinMode(MBUTTON_PIN, INPUT);
  pinMode(RBUTTON_PIN, INPUT);
}

void
loop()
{
  unsigned long current_time = millis();

  // Iterate over the array of buttons to check each one
  for (int i = 0; i < NUM_OF_PEDALS; i++) {
    auto& button = button_array[i];
    int reading = digitalRead(button.pin);

    // If the switch has changed, and it's been long enough since
    // the last  button press.
    if (reading != button.state &&
        (current_time - button.last_debounce_time) > BOUNCE_TIME) {

      button.is_inactive = false;
      button.state = reading;
      button.last_debounce_time = current_time;

      pedal_operation(button.mode, button.should_engage(reading));
    }

    // Clearing action when the battle has been engaged for too long.
    //
    // The pedal has now been engaged for too long.
    // } else if (button.is_inactive == false && reading == button.state &&
    //            button.should_engage(reading) &&
    //            (current_time - button.last_debounce_time) > RESIDENCE_TIME)
    //            {
    //   button.is_inactive = true;
    //   // Keyboard.press(MODIFIERKEY_CTRL);
    //   // delay(20);
    //   // Keyboard.release(MODIFIERKEY_CTRL);
    //   pedal_operation(button.mode, false);
    // }
  }

  // Enable programs on my PC to alter the behavior of my foot
  // mouse.  Check the serial port for incoming bytes every loop.
  if (receive_serial_input()) {
    parse_message();
  }
}
