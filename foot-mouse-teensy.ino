/*
BOARD INFO & CONFIGURATION SETTINGS:
Teensy 4.0
USB Type: Serial + Keyboard + Mouse + Joystick
CPU Speed: 24MHz (set to minimum for efficiency)
Optimize: fastest (because why not)

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

#include <Keyboard.h>
#include <Mouse.h>

#define BOUNCE_TIME 20 // milliseconds

// model number 2 is the at-home unit (with 2 foot pedals)
// model number 3 is the at-work unit (with 3 foot pedals)
#define MODEL_NUMBER 3

#if MODEL_NUMBER == 2
constexpr int k_number_of_pedals = 2;
#define LBUTTON_PIN 2
#define MBUTTON_PIN 5
#define RBUTTON_PIN 7
#endif // MODEL_NUMBER

// WARNING: the os will be hosed if k_number_of_pedals is set to (3) and three
// pedals are not connected. If this happens:
//   1. unplug microcontroller
//   2. put computer to sleep and wake up to reset stop modifiers
//   3. recompile fixed code and make sure Paul's loader tool is running
//   4. hold reset button while plugging microcontroller back in 
  // 5. press reset button within 1 second of being plugged in
#if MODEL_NUMBER == 3
constexpr int k_number_of_pedals = 2; // Set to 3 to enable right click
#define LBUTTON_PIN 4
#define MBUTTON_PIN 5
#define RBUTTON_PIN 6
#endif // MODEL_NUMBER

#define PEDAL_DOWN 1
#define PEDAL_UP 0

#define DEBUG 1

// These are from Paul's mouse library.
// I use the mode as the button to press because of legacy reasons.
static_assert(MOUSE_LEFT == 1, "Voice commands will fail");
static_assert(MOUSE_MIDDLE == 4, "Voice commands will fail");
static_assert(MOUSE_RIGHT == 2, "Voice commands will fail");

enum Mode
{
  MACRO_CTRL_CLICK = 8,
  MACRO_SCROLL = 9,
};

enum MessageCode
{
  MSG_IDENTIFY = 4,
  MSG_SET_BUTTONS = 5,
};

// the Arduino compiler does not like the Mozilla style for template
// declarations
// clang-format off
// Template for debugging to serial monitor
template<class T>
void Log(T msg) {
  if (DEBUG && Serial) {
    // if DEBUG is on this will fail if no serial monitor
    Serial.println(msg);
  }
}
// clang-format on

class CButton
{
public:
  int pin;
  int mode;
  int is_inverted;

  int default_mode;
  int default_is_inverted;

  int state = 0; // The way this code is written, the button state doesn't
                 // matter, only if it changes
  unsigned long last_debounce_time = 0;

  CButton(int pin_number, int mode_, int is_inverted_)
  {
    pin = pin_number;
    mode = mode_;
    is_inverted = is_inverted_;

    default_mode = mode_;
    default_is_inverted = is_inverted_;
  }

  void resetToDefaults()
  {
    mode = default_mode;
    is_inverted = default_is_inverted;
  }
};

// clang-format off
CButton button_array[] = {  // button defaults
  CButton(LBUTTON_PIN, MOUSE_LEFT, true),
  CButton(MBUTTON_PIN, MOUSE_MIDDLE, false),
  CButton(RBUTTON_PIN, MOUSE_RIGHT, false)
};
// clang-format on

/**
 * Receiving serial input is used to change pedal mode
 * through the use of scripts on the host pc.
 *
 * Note that I use no log statements to debug serial communications.
 * I can't send serial to the teensy through the normal ports when I'm
 * monitoring serial through the Arduino application.
 */
constexpr const size_t k_buffer_size = 4;
byte g_input_buffer[k_buffer_size];

bool
ReceiveSerialInput()
{
  constexpr byte start_marker = 16;
  constexpr byte end_marker = 17;
  bool found_start_marker = false;
  byte index = 0;
  byte rb;

  while (Serial.available() > 0) {
    rb = Serial.read();
    if (found_start_marker) {
      if (rb != end_marker && index < k_buffer_size) {
        g_input_buffer[index++] = rb;
      } else if (rb == end_marker && index == k_buffer_size) {
        // buffer is full and valid end marker found after body
        return true;
      } else {
        // buffer full but no end marker
        // or end marker found with an incomplete buffer
        // reject message
        return false;
      }
    } else if (rb == start_marker)
      found_start_marker = true;
  }

  // no start marker found or no serial was available
  // or message was not long enough
  return false;
}

// parse a 3-byte buffer
void
ParseMessage()
{
  // TODO: add in heartbeat msg code
  const auto message_code = g_input_buffer[0];
  const auto pedal_index = g_input_buffer[1];
  const auto mode = g_input_buffer[2];
  const auto inversion = g_input_buffer[3];

  if (MSG_IDENTIFY == message_code) {
    // return an identifier code
  } else if (MSG_SET_BUTTONS == message_code) {
    // special case command to reset to default
    if (pedal_index == 0 && mode == 0 && inversion == 0) {
      for (int i = 0; i < k_number_of_pedals; ++i) {
        button_array[i].resetToDefaults();
      }
      return;
    }

    // validate message parameters
    bool valid_pedal_index =
      pedal_index >= 0 || pedal_index < k_number_of_pedals;
    bool valid_mode = mode > 0;
    bool valid_inversion = inversion == 0 || inversion == 1;

    if (valid_pedal_index && valid_mode && valid_inversion) {
      button_array[pedal_index].mode = static_cast<int>(mode);
      button_array[pedal_index].is_inverted = static_cast<bool>(inversion);
    }
  }
}

void
setup()
{
  Serial.begin(9600);
  Mouse.begin(); // not sure why don't need to call Keyboard.begin() as wellall

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
  for (int i = 0; i < k_number_of_pedals; i++) {
    auto& button = button_array[i];
    int reading = digitalRead(button.pin);

    // If the switch has changed, and it's been long enough since the last
    // button press.
    if (reading != button.state &&
        (current_time - button.last_debounce_time) > BOUNCE_TIME) {

      button.state = reading;
      button.last_debounce_time = current_time;

      bool engage = ((button.is_inverted && reading == PEDAL_UP) ||
                     (!button.is_inverted && reading == PEDAL_DOWN));

      switch (button.mode) {
        // for left, right, and middle button modes, the mode number corresponds
        // to the Mouse library button constant
        case MOUSE_LEFT:
        case MOUSE_RIGHT:
        case MOUSE_MIDDLE:
          if (engage) {
            Mouse.press(button.mode);
          } else {
            Mouse.release(button.mode);
          }
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
  }

  // Enable programs on my PC to alter the behavior
  // of my foot mouse.
  // Check the serial port for incoming bytes every loop
  if (ReceiveSerialInput()) {
    ParseMessage();
  }
}
