/*

CODE FOR TEENSY

NOTE:
To get serial communications from the board,
you must set usb type under Tools -> Board -> USB Type to include
a Serial library.
e.g. "Serial + Keyboard + Mouse + Joystick"

Serial Update:
sequence of ops:
send single byte message to server
server checks current coordinates of mouse
if mouse is in left-hand area of screen
send single bite message back
    to release the middle click

*/

#include <Mouse.h>

#define BOUNCE_TIME 2 // milliseconds
#define MIDDLE_CLICK_SLEEP_TIME 3500 // milliseconds

#define LBUTTON_PIN 2
#define MBUTTON_PIN 5
#define ENABLE_PIN 3 // only used on model with a switch, depreciated

#define DGTL_READ_PRESSED_STATE 0
#define PRESS 0
#define RELEASE 1

#define DEBUG 1
#define NDEBUG
// Comment out below line to disable middle click
#define MIDDLE_CLICK_ENABLED

// Writing the values is redundant
// outside programs need to know these values
// so i have them listed explicitly
enum return_message_value {
    LEFT_CLICK = 0,
    LEFT_PRESS = 1,
    LEFT_RELEASE = 2,
    MIDDLE_CLICK = 3,
    MIDDLE_PRESS = 4,
    MIDDLE_RELEASE = 5,
    MIDDLE_MODE_SWITCH = 6,
};

enum send_message_value {
    MIDDLE_DISABLE = 11,
};

enum middle_function {
    MODE_MIDDLE = 20,
    MODE_MIDDLE_INVERTED = 21,
    MODE_RIGHT = 22,
    MODE_RIGHT_INVERTED = 23,
    MODE_LEFT = 24,
    MODE_LEFT_INVERTED = 25,
};

byte iButtonResetCount = 0; // main loop will reset button after count
int IncomingByte = 0;

// Variables for left button
bool bStateLeft = DGTL_READ_PRESSED_STATE;

volatile bool PressedLeft = 0; // 0 is not PressedLeft, IE pulled high
unsigned long ChangeTimeLeft = 0;
unsigned long InterruptTimeLeft = 0;
byte pCountLeft = 0;
byte rCountLeft = 0;

// Variables for middle button
byte MiddleFunction = MODE_MIDDLE_INVERTED;
byte iButtonResetCountMiddle = 0; // main loop will reset button after count
bool bStateMiddle = DGTL_READ_PRESSED_STATE;
volatile bool bPressedFlag = 0;

volatile bool PressedMiddle = 0; // 0 is not PressedLeft, IE pulled high
volatile bool ActivatedMiddle = 0; // 0 is not PressedLeft, IE pulled high
unsigned long ChangeTimeMiddle = 0;
unsigned long InterruptTimeMiddle = 0;
unsigned long ResetTimeMiddle = 0;
unsigned long RoundTrip = 0;
byte pCountMiddle = 0;
byte rCountMiddle = 0;

// Template for debugging to serial monitor
template <class T>
void log(T msg)
{
    if (DEBUG && Serial)
    //if (DEBUG)
    {
        // if DEBUG is on this will fail if no serial monitor
        Serial.println(msg);
    }
}

void buttonChangedLeft()
{
    //Serial.println("Change"); // for debugging
    InterruptTimeLeft = millis();
    if (PressedLeft == 0) {
        // log("change int left press");
        if (InterruptTimeLeft - ChangeTimeLeft > BOUNCE_TIME)
        {
            Mouse.press(MOUSE_LEFT);
            delay(1);
            PressedLeft = 1;
            ChangeTimeLeft = InterruptTimeLeft;
            // log(++pCountLeft); // for debugging
        }
    }
    if (PressedLeft == 1) {
        // log("change int left release");
        if (InterruptTimeLeft - ChangeTimeLeft > BOUNCE_TIME)
        {
            Mouse.release(MOUSE_LEFT);
            delay(1);
            PressedLeft = 0;
            ChangeTimeLeft = InterruptTimeLeft;
            // log(++rCountLeft); // for debugging
        }
    }
}

void buttonChangedMiddle()
{
    //Serial.println("Change"); // for debugging
    InterruptTimeMiddle = millis();
    if (PressedMiddle == 0) {
        // log("change int middle press");
        if (InterruptTimeMiddle - ChangeTimeMiddle > BOUNCE_TIME)
        {
            switch (MiddleFunction)
            {
                case MODE_MIDDLE:
                    Mouse.press(MOUSE_MIDDLE);
                    break;
                case MODE_MIDDLE_INVERTED:
                    Mouse.release(MOUSE_MIDDLE);
                    break;
                case MODE_RIGHT:
                    Mouse.press(MOUSE_RIGHT);
                    break;
                case MODE_RIGHT_INVERTED:
                    Mouse.release(MOUSE_RIGHT);
                    break;
                case MODE_LEFT:
                    Mouse.press(MOUSE_LEFT);
                    break;
                case MODE_LEFT_INVERTED:
                    Mouse.release(MOUSE_LEFT);
                    break;
            }
            // Mouse.press(MOUSE_MIDDLE);
            delay(1);
            PressedMiddle = 1;
            ChangeTimeMiddle = InterruptTimeMiddle;
            // log(++pCountMiddle); // for debugging
        }
    }
    if (PressedMiddle == 1) {
        // log("change int middle release");
        if (InterruptTimeMiddle - ChangeTimeMiddle > BOUNCE_TIME)
        {
            switch (MiddleFunction)
            {
                case MODE_MIDDLE:
                    Mouse.release(MOUSE_MIDDLE);
                    break;
                case MODE_MIDDLE_INVERTED:
                    Mouse.press(MOUSE_MIDDLE);
                    break;
                case MODE_RIGHT:
                    Mouse.release(MOUSE_RIGHT);
                    break;
                case MODE_RIGHT_INVERTED:
                    Mouse.press(MOUSE_RIGHT);
                    break;
                case MODE_LEFT:
                    Mouse.release(MOUSE_LEFT);
                    break;
                case MODE_LEFT_INVERTED:
                    Mouse.press(MOUSE_LEFT);
                    break;
            }
            // Mouse.release(MOUSE_MIDDLE);
            delay(1);
            PressedMiddle = 0;
            ChangeTimeMiddle = InterruptTimeMiddle;
            // log(++rCountMiddle); // for debugging
        }
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting Serial Connection...");
    // Setup for the mouseclick
    pinMode(LBUTTON_PIN, INPUT); // Pin for the button clicks
    pinMode(MBUTTON_PIN, INPUT); // Pin for the button clicks
    //pinMode(ENABLE_PIN, INPUT_PULLUP); // Bring pin low to mouse functions
    attachInterrupt(digitalPinToInterrupt(LBUTTON_PIN), buttonChangedLeft, CHANGE);
#ifdef MIDDLE_CLICK_ENABLED
    attachInterrupt(digitalPinToInterrupt(MBUTTON_PIN), buttonChangedMiddle, CHANGE);
#endif
    Mouse.begin();
    Serial.println("Starting Mouse Class");
}

void loop()
{

    if (iButtonResetCount++ == 100)
    {
        bStateLeft = digitalRead(LBUTTON_PIN);
        bStateMiddle = digitalRead(MBUTTON_PIN);
        // Reset the button state to not pressed
        // if debounced routine bugged or some interrupts were missed
        if (bStateLeft == !DGTL_READ_PRESSED_STATE)
        {
            PressedLeft = 0;
        }

        if (bStateMiddle == !DGTL_READ_PRESSED_STATE)
        {
            PressedMiddle = 0;
        }
        iButtonResetCount = 0;
    }

    // Deactivate the middle mouse, if the pedal has been lifted
    // for more than 4 seconds, by sending
    // a middle mouse release message
    if (iButtonResetCountMiddle++ == 100)
    {
        bStateMiddle = digitalRead(MBUTTON_PIN);
        if (bStateMiddle == DGTL_READ_PRESSED_STATE)
        {
            if((MIDDLE_CLICK_SLEEP_TIME < millis() - ChangeTimeMiddle) && (PressedMiddle))
            {
                // setting the 'PressedMiddle' equal to 0 is not entirely necessary
                // however this will prevent an extraneous mouse release message
                // from being sent when I put my foot back on the pedal
                // ActivatedMiddle = 0;
                Mouse.release(MOUSE_MIDDLE);
                Mouse.release(MOUSE_MIDDLE);
                Mouse.release(MOUSE_MIDDLE);
            }
        }
        iButtonResetCountMiddle = 0;
    }

    // Enable programs on my PC to alter the behavior
    // of my foot mouse.
    // Check the serial port for incoming bites every loop
    if (Serial.available())
    {
        IncomingByte = Serial.read();
        if (-1 != IncomingByte)
        {
            switch (IncomingByte)
            {
                case MODE_MIDDLE:
                    MiddleFunction = MODE_MIDDLE;
                    break;
                case MODE_MIDDLE_INVERTED:
                    MiddleFunction = MODE_MIDDLE_INVERTED;
                    break;
                case MODE_RIGHT:
                    MiddleFunction = MODE_RIGHT;
                    break;
                case MODE_RIGHT_INVERTED:
                    MiddleFunction = MODE_RIGHT_INVERTED;
                    break;
                case MODE_LEFT:
                    MiddleFunction = MODE_LEFT;
                    break;
                case MODE_LEFT_INVERTED:
                    MiddleFunction = MODE_LEFT_INVERTED;
                    break;
                default:
                    break;
            }
        }
    }
}
