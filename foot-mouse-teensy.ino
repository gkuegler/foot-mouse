/*

BOARD INFO:
Teensy 4.0
USB Type: Serial + Keyboard + Mouse + Joystick
CPU Speed: 24MHz (set to minimum)
Optimize: fastest

Upload Steps:
1. Set board family
2. Set USB type (links in needed libraries for mouse & serial)
3. Set desired clock speed
4. Set code optimization
5. Use "Get Board Info" to find correct COM port
6. Push reset button on physical board
7. Click Upload

NOTE:
To get serial communications from the board,
you must set usb type under Tools -> Board -> USB Type to include
a Serial library.
e.g. "Serial + Keyboard + Mouse + Joystick"

pedal up -> closes circuit -> pin pulled low
pedal down -> opens circuit -> pin pulled high via external resistor

Serial Sequence Of Ops:
1. send single byte message to server
2. server checks current coordinates of mouse
3. if mouse is in left-hand area of screen
4. send single bite message back
    to release the middle click

*/

#include <Mouse.h>
// #include <Keyboard.h>

#define BOUNCE_TIME 20  // milliseconds

#define MODEL_NUMBER 2

#if MODEL_NUMBER == 2
const int numberOfButtons = 2;
#define LBUTTON_PIN 2
#define MBUTTON_PIN 5
#define RBUTTON_PIN 7
#endif  // MODEL_NUMBER
#if MODEL_NUMBER == 3
// the os will be hosed if numberOfButtons is set to (3) and three pedals
// are not connected.
const int numberOfButtons = 2;  // Set to 3 to enable right click
#define LBUTTON_PIN 4
#define MBUTTON_PIN 5
#define RBUTTON_PIN 6
#endif  // MODEL_NUMBER

#define PEDAL_DOWN 1
#define PEDAL_UP 0

#define DEBUG 1

static_assert(MOUSE_LEFT == 1, "Voice commands will fail");
static_assert(MOUSE_MIDDLE == 4, "Voice commands will fail");
static_assert(MOUSE_RIGHT == 2, "Voice commands will fail");

// Template for debugging to serial monitor
template <class T>
void Log(T msg) {
  if (DEBUG && Serial) {
    // if DEBUG is on this will fail if no serial monitor
    Serial.println(msg);
  }
}

class CButton {
 public:
  int buttonPin;
  int buttonState;  // The way this code is written, the button state doesn't
                    // matter, only if it changes
  unsigned long lastDebounceTime;
  int mode;
  int isInverted;

  CButton(int SetbuttonPin, int Setmode, int SetisInverted) {
    buttonPin = SetbuttonPin;
    mode = Setmode;
    isInverted = SetisInverted;
  }
};

// clang-format off
CButton buttonArray[] = {
  CButton(LBUTTON_PIN, MOUSE_LEFT, true),
  CButton(MBUTTON_PIN, MOUSE_MIDDLE, false),
  CButton(RBUTTON_PIN, MOUSE_RIGHT, false)
};
// clang-format on

/**
 * Receiving serial input is used to change pedal mode
 * through the use of scripts on the host pc.
 */
const byte bufferSize = 3;
byte inputBuffer[bufferSize];
bool newMessageAvailable = false;

void ReceiveSerialInput() {
  static bool receiveInProgress = false;
  static byte index = 0;
  static byte startMarker = 16;
  static byte endMarker = 19;
  static byte rb;

  index = 0;
  receiveInProgress = false;

  if (newMessageAvailable) {
    Log("message rejected, existing message not read yet");
    return;
  }

  if (Serial.available()) {
    Log("starting to read message");
    while (Serial.available() > 0) {
      rb = Serial.read();
      Log(rb);

      if (receiveInProgress) {
        if (rb != endMarker) {
          Log("adding character to buffer");
          inputBuffer[index] = rb;
          index++;

          if (index >= 5) {
            Log("buffer overflow on input");
            return;
          }
        } else {
          break;
        }
      } else if (rb == startMarker)
        receiveInProgress = true;
    }
    Log("end of the loop reached");

    receiveInProgress = false;

    if (index == 0) {
      Log("no start sequence found");
    } else if (index < 3) {
      Log("message not long enough");
    } else {
      newMessageAvailable = true;
      Log("Setting message available: true");
    }
  }
}

void ParseMessage() {
  Log("Starting To Parse Message");
  byte pedalNumber = inputBuffer[0];
  byte mouseMode = inputBuffer[1];
  byte isInverted = inputBuffer[2];

  Log(pedalNumber);
  Log(mouseMode);
  Log(isInverted);


  // TODO: make a control click mouse option
  if (pedalNumber >= 0 && pedalNumber <= 2) {
    if (mouseMode == MOUSE_LEFT || mouseMode == MOUSE_MIDDLE || mouseMode == MOUSE_RIGHT) {
      buttonArray[pedalNumber].mode = mouseMode;  // set desired mouse button
    } else {
      Log("Mouse mode is not valid.");
    }

    if (isInverted == 0 || isInverted == 1) {
      buttonArray[pedalNumber].isInverted = isInverted;  // see file comment for explanation on pedal inversion
    } else {
      Log("Inverted mode is not valid.");
    }
  } else {
    Log("Pedal number is not valid.");
  }

  newMessageAvailable = false;
  Log("Ending Parse message");
}

void setup() {
  Serial.begin(9600);
  Log("Starting Serial Connection...");
  Mouse.begin();
  Log("Starting Mouse Class");

  pinMode(LBUTTON_PIN, INPUT);  // Pin for the button clicks
  pinMode(MBUTTON_PIN, INPUT);  // Pin for the button clicks
  pinMode(RBUTTON_PIN, INPUT);  // Pin for the button clicks

  Log(MOUSE_LEFT);
  Log(MOUSE_MIDDLE);
  Log(MOUSE_RIGHT);
}

void loop() {
  unsigned long CurrentTime = millis();

  // Iterate over the array of buttons to check each one
  for (int i = 0; i < numberOfButtons; i++) {
    int reading = digitalRead(buttonArray[i].buttonPin);

    // If the switch has changed, and it's been long enough since the last
    // button press:
    if (reading != buttonArray[i].buttonState && (CurrentTime - buttonArray[i].lastDebounceTime) > BOUNCE_TIME) {
      buttonArray[i].buttonState = reading;
      buttonArray[i].lastDebounceTime = CurrentTime;
      int mouseButton = buttonArray[i].mode;

      if (buttonArray[i].isInverted) {
        if (reading == PEDAL_UP) {
          Mouse.press(mouseButton);
        } else {
          Mouse.release(mouseButton);
        }
      } else {
        if (reading == PEDAL_UP) {
          Mouse.release(mouseButton);
        } else {
          Mouse.press(mouseButton);
        }
      }
    }
  }

  // Enable programs on my PC to alter the behavior
  // of my foot mouse.
  // Check the serial port for incoming bites every loop
  ReceiveSerialInput();
  if (newMessageAvailable) {
    ParseMessage();
  }
}
