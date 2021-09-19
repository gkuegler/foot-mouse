/*

CODE FOR TEENSY

Pedal Down = voltage High
pedal up -> closes circuit -> pin pulled low
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
#define RBUTTON_PIN 7

#define ENABLE_PIN 3 // only used on model with a switch, depreciated

#define PEDAL_DOWN 1
#define PEDAL_UP 0

#define DEBUG 1

static_assert(MOUSE_LEFT == 1, "Voice commands will fail");
static_assert(MOUSE_MIDDLE == 4, "Voice commands will fail");
static_assert(MOUSE_RIGHT == 2, "Voice commands will fail");

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


// Template for debugging to serial monitor
template <class T>
void log(T msg)
{
    if (DEBUG && Serial)
    {
        // if DEBUG is on this will fail if no serial monitor
        Serial.println(msg);
    }
}

class CButton{
public:
    int buttonPin;
    int buttonState; // The way this code is written, the button state doesn't matter, only if it changes
    unsigned long lastDebounceTime;
    int mode;
    int isInverted;

    CButton(int SetbuttonPin, int Setmode, int SetisInverted)
    {
        buttonPin = SetbuttonPin;
        mode = Setmode;
        isInverted = SetisInverted;
    }
};

CButton buttonOne(LBUTTON_PIN, MOUSE_LEFT, true);

int numberOfButtons = 3;
CButton buttonArray[] = {
    CButton(LBUTTON_PIN, MOUSE_LEFT, true),
    CButton(MBUTTON_PIN, MOUSE_MIDDLE, false),
    CButton(RBUTTON_PIN, MOUSE_RIGHT, false)
};

const byte bufferSize = 3;
byte inputBuffer[bufferSize];
bool newMessageAvailable = false;

void ReceiveSerialInput()
{
    static bool receiveInProgress = false;
    static byte index = 0;
    // static byte startMarker = '<';
    // static byte endMarker = '>'
    static byte startMarker = 16;
    static byte endMarker = 19;
    static byte rb;

    index = 0;
    receiveInProgress = false;

    if (newMessageAvailable)
    {h
        Log("message rejected, existing message not read yet");
        return;
    }


    if (Serial.available()) 
    {
        Log("starting to read message");
        while (Serial.available() > 0)
        {
            rb = Serial.read();
            Log(rb);

            if (receiveInProgress)
            {
                if (rb != endMarker)
                {
                    Log("adding character to buffer");
                    inputBuffer[index] = rb;
                    index++;

                    if (index >= 5) 
                    {
                        Log("buffer overflow on input");
                        return;
                    }
                }
                else break;
            }
            else if (rb == startMarker) receiveInProgress = true;
        }
        Log("end of the loop reached");

        receiveInProgress = false;

        if (index == 0) Log("no start sequence found");
        else if (index < 3) Log("message not long enough");
        else
        {
            newMessageAvailable = true;
            Log("Setting message available: true");
        }
    }

}

void ParseMessage()
{
    Log("Starting To Parse Message");
    byte pedalNumber = inputBuffer[0];
    byte mouseMode = inputBuffer[1];
    byte isInverted = inputBuffer[2];

    Log(pedalNumber);
    Log(mouseMode);
    Log(isInverted);

    if (pedalNumber >= 0 && pedalNumber <= 2)
    {
        if (mouseMode == MOUSE_LEFT || mouseMode == MOUSE_MIDDLE || mouseMode == MOUSE_RIGHT)
        {
            buttonArray[pedalNumber].mode = mouseMode;
        }
        else Log("Mouse mode is not valid.");

        if (isInverted == 0 || isInverted == 1)
        {
            buttonArray[pedalNumber].isInverted = isInverted;
        }
        else Log("Inverted mode is not valid.");
    }
    else Log("Petal number is not valid.");

    newMessageAvailable = false;
    Log("Ending Parse message");
}

// void ParseMessage()
// {
//     Log("Starting To Parse Message");
//     byte pedalNumber = inputBuffer[0];
//     byte mouseMode = inputBuffer[1];
//     byte isInverted = inputBuffer[2];

//     Log(pedalNumber);
//     Log(mouseMode);
//     Log(isInverted);

//     if (pedalNumber == '0')
//     {
//         if (mouseMode == 'L')
//         {
//             buttonArray[1].mode = MOUSE_LEFT;
//         }
//         else Log("Mouse mode is not valid.");

//         if (isInverted == '0')
//         {
//             buttonArray[1].isInverted = 0;
//         }
//         else Log("Inverted mode is not valid.");
//     }
//     else Log("Pedal number is not valid.");

//     newMessageAvailable = false;
//     
// }

void setup()
{
    Serial.begin(9600);
    log("Starting Serial Connection...");
    Mouse.begin();
    log("Starting Mouse Class");

    pinMode(LBUTTON_PIN, INPUT); // Pin for the button clicks
    pinMode(MBUTTON_PIN, INPUT); // Pin for the button clicks
    pinMode(RBUTTON_PIN, INPUT); // Pin for the button clicks

    log(MOUSE_LEFT);
    log(MOUSE_MIDDLE);
    log(MOUSE_RIGHT);

}

void loop()
{
    unsigned long CurrentTime = millis();

    // Iterate over the array of buttons to check each one
    for (int i = 0; i < numberOfButtons; i++)
    {
        int reading = digitalRead(buttonArray[i].buttonPin);

        // If the switch has changed, and it's been long enough since the last button press:
        if (reading != buttonArray[i].buttonState && (CurrentTime - buttonArray[i].lastDebounceTime) > BOUNCE_TIME)
        {
            buttonArray[i].buttonState = reading;
            buttonArray[i].lastDebounceTime = CurrentTime;
            int mouseButton = buttonArray[i].mode;

            if (buttonArray[i].isInverted)
            {
                if (reading == PEDAL_UP) Mouse.press(mouseButton);
                else Mouse.release(mouseButton);
            }
            else
            {
                if (reading == PEDAL_UP) Mouse.release(mouseButton);
                else Mouse.press(mouseButton);
            }
        }
    }

    // Enable programs on my PC to alter the behavior
    // of my foot mouse.
    // Check the serial port for incoming bites every loop
    ReceiveSerialInput();
    if (newMessageAvailable)
    {
        ParseMessage();
    }

}