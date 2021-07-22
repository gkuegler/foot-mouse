/*

Typical is to upload via COM8
upload will fail if tried through COM1

*/

#include <Mouse.h>

#define BOUNCE_TIME 2 //ms
#define BUTTON_PIN 2
#define ENABLE_PIN 3
#define DGTL_READ_PRESSED 0
#define PRESS 0
#define RELEASE 1
#define DEBUG 1

bool bstate = DGTL_READ_PRESSED;
byte i = 0;

volatile bool pressed = 0; // 0 is not pressed, IE pulled high
unsigned long change_time = 0;
unsigned long interrupt_time = 0;
byte pCount = 0;
byte rCount = 0;

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

void sendMouseEvent(int type)
{
    //if (digitalRead(ENABLE_PIN) == 1)
    //{
        if (type == PRESS)
        {
            Mouse.press();
        }
        else
        {
            Mouse.release();
        }
    //}
    delay(1);
}

void bChange()
{
    //Serial.println("Change"); // for debugging
    interrupt_time = millis();
    if (pressed == 0) {
        log("change int press");
        if (interrupt_time - change_time > BOUNCE_TIME)
        {
            sendMouseEvent(PRESS);
            pressed = 1;
            change_time = interrupt_time;
            log(++pCount); // for debugging
        }
    }
    if (pressed == 1) {
        log("change int release");
        if (interrupt_time - change_time > BOUNCE_TIME)
        {
            sendMouseEvent(RELEASE);
            pressed = 0;
            change_time = interrupt_time;
            log(++rCount); // for debugging
        }
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting Serial Connection...");
    // Setup for the mouseclick
    pinMode(BUTTON_PIN, INPUT); // Pin for the button clicks
    //pinMode(ENABLE_PIN, INPUT_PULLUP); // Bring pin low to mouse functions
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), bChange, CHANGE);
    Mouse.begin();
    Serial.println("Starting Mouse Class");
}

void loop()
{
    if (i++ == 50)
    {
        i = 0;
        bstate = digitalRead(BUTTON_PIN);
        if (bstate == !DGTL_READ_PRESSED)
        {
            pressed = 0;
        }
    }
}
