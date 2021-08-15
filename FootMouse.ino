/*

CODE FOR TEENSY

*/

#include <Mouse.h>

#define BOUNCE_TIME 2 //ms
#define LBUTTON_PIN 2
#define MBUTTON_PIN 5
#define ENABLE_PIN 3
#define DGTL_READ_PRESSED_STATE 0
#define PRESS 0
#define RELEASE 1
#define DEBUG 1


// Variables for left button
byte iButtonResetCountLeft = 0; // main loop will reset button after count
bool bStateLeft = DGTL_READ_PRESSED_STATE;

volatile bool PressedLeft = 0; // 0 is not PressedLeft, IE pulled high
unsigned long ChangeTimeLeft = 0;
unsigned long InterruptTimeLeft = 0;
byte pCountLeft = 0;
byte rCountLeft = 0;

// Variables for middle button
byte iButtonResetCountMiddle = 0; // main loop will reset button after count
bool bStateMiddle = DGTL_READ_PRESSED_STATE;

volatile bool PressedMiddle = 0; // 0 is not PressedLeft, IE pulled high
volatile bool ActivatedMiddle = 0; // 0 is not PressedLeft, IE pulled high
unsigned long ChangeTimeMiddle = 0;
unsigned long ReleasedTimeMiddle = 0;
unsigned long InterruptTimeMiddle = 0;
unsigned long ResetTimeMiddle = 0;
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
        log("change int left press");
        if (InterruptTimeLeft - ChangeTimeLeft > BOUNCE_TIME)
        {
            Mouse.press(MOUSE_LEFT);
            delay(1);
            PressedLeft = 1;
            ChangeTimeLeft = InterruptTimeLeft;
            log(++pCountLeft); // for debugging
        }
    }
    if (PressedLeft == 1) {
        log("change int left release");
        if (InterruptTimeLeft - ChangeTimeLeft > BOUNCE_TIME)
        {
            Mouse.release(MOUSE_LEFT);
            delay(1);
            PressedLeft = 0;
            ChangeTimeLeft = InterruptTimeLeft;
            log(++rCountLeft); // for debugging
        }
    }
}

void buttonChangedMiddle()
{
    //Serial.println("Change"); // for debugging
    InterruptTimeMiddle = millis();
    if (PressedMiddle == 0) {
        log("change int middle press");
        if (InterruptTimeMiddle - ChangeTimeMiddle > BOUNCE_TIME)
        {
            Mouse.press(MOUSE_MIDDLE);
            delay(1);
            PressedMiddle = 1;
            ChangeTimeMiddle = InterruptTimeMiddle;
            log(++pCountMiddle); // for debugging
        }
    }
    if (PressedMiddle == 1) {
        log("change int middle release");
        if (InterruptTimeMiddle - ChangeTimeMiddle > BOUNCE_TIME)
        {
            Mouse.release(MOUSE_MIDDLE);
            delay(1);
            PressedMiddle = 0;
            ChangeTimeMiddle = InterruptTimeMiddle;
            ReleasedTimeMiddle = InterruptTimeMiddle;
            log(++rCountMiddle); // for debugging
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
    attachInterrupt(digitalPinToInterrupt(MBUTTON_PIN), buttonChangedMiddle, CHANGE);
    Mouse.begin();
    Serial.println("Starting Mouse Class");
}

void loop()
{
    if (iButtonResetCountLeft++ == 100)
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
        iButtonResetCountLeft = 0;
    }

    // // Every 50 cycles run a check
    // if (iButtonResetCountMiddle++ == 100 && PressedMiddle)
    // {
    //     // Deactivate the middle mouse, if the pedal has been lifted
    //     // for more than 4 seconds, by sending
    //     // a middle mouse release message
    //     if(0 < millis() - (ReleasedTimeMiddle + 4000))
    //     {
    //         bStateMiddle = digitalRead(MBUTTON_PIN);
    //         if (bStateMiddle == DGTL_READ_PRESSED_STATE)
    //         {
    //             // setting the 'PressedMiddle' equal to 0 is not entirely necessary
    //             // however this will prevent an extraneous mouse release message
    //             // from being sent when I put my foot back on the pedal
    //             // ActivatedMiddle = 0;
    //             Mouse.release(MOUSE_MIDDLE);
    //         }
    //     }
    //     iButtonResetCountMiddle = 0;
    // }
}
