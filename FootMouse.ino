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
    MIDDLE_RELEASE = 5
};

enum send_message_value {
    MIDDLE_DISABLE = 11,
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
byte iButtonResetCountMiddle = 0; // main loop will reset button after count
bool bStateMiddle = DGTL_READ_PRESSED_STATE;
volatile bool bPressedFlag = 0;

volatile bool PressedMiddle = 0; // 0 is not PressedLeft, IE pulled high
volatile bool ActivatedMiddle = 0; // 0 is not PressedLeft, IE pulled high
unsigned long ChangeTimeMiddle = 0;
unsigned long ReleasedTimeMiddle = 0;
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

bool checkIfMouseInArea()
{   
    Serial.send_now();
    Serial.write(MIDDLE_DISABLE);
    Serial.available();
    IncomingByte = Serial.read();

    if (MIDDLE_DISABLE == IncomingByte) return true;
    else return false;
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
            // Don't press the middle mouse button
            // if in the designated area
            // Mouse.press(MOUSE_MIDDLE);
            // delay(1);
            bPressedFlag = 1;
            PressedMiddle = 1;
            ChangeTimeMiddle = InterruptTimeMiddle;
            // log(++pCountMiddle); // for debugging
        }
    }
    if (PressedMiddle == 1) {
        // log("change int middle release");
        if (InterruptTimeMiddle - ChangeTimeMiddle > BOUNCE_TIME)
        {
            Mouse.release(MOUSE_MIDDLE);
            delay(1);
            PressedMiddle = 0;
            ChangeTimeMiddle = InterruptTimeMiddle;
            ReleasedTimeMiddle = InterruptTimeMiddle;
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
    // Pick up the ISR flag press the middle button
    if (bPressedFlag)
    {
        if (checkIfMouseInArea())
        {
            Mouse.press(MOUSE_MIDDLE);
            bPressedFlag = 0;
        }

    }

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

    // Every 50 cycles run a check
    if (iButtonResetCountMiddle++ == 100)
    {
        // Deactivate the middle mouse, if the pedal has been lifted
        // for more than 4 seconds, by sending
        // a middle mouse release message
        bStateMiddle = digitalRead(MBUTTON_PIN);
        if (bStateMiddle == DGTL_READ_PRESSED_STATE)
        {
            if((MIDDLE_CLICK_SLEEP_TIME < millis() - ReleasedTimeMiddle) && (PressedMiddle))
            {
                // setting the 'PressedMiddle' equal to 0 is not entirely necessary
                // however this will prevent an extraneous mouse release message
                // from being sent when I put my foot back on the pedal
                // ActivatedMiddle = 0;
                Mouse.release(MOUSE_MIDDLE);
            }
        }
        iButtonResetCountMiddle = 0;
    }

    // Enable programs on my PC to alter the behavior
    // of my foot mouse.
    // Check the serial port for incoming bites every loop
    // if (Serial.available())
    // {
    // IncomingByte = Serial.read();
    // if (-1 != IncomingByte)
    // {
    //     switch (IncomingByte)
    //     {
    //         case MIDDLE_RELEASE:
    //             Mouse.release(MOUSE_MIDDLE);
    //             break;
    //         default:
    //             break;
    //     }
    // }
    // }
}
