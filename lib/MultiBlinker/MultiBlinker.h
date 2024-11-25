#ifndef MULTIBLINKER_H
#define MULTIBLINKER_H

#include <Arduino.h>

// These are the four LEDs on the PCB
#ifdef SPACTRLPCB
    const int PCB_LED1 = 14;
    const int PCB_LED2 = 41;
    const int PCB_LED3 = 42;
    const int PCB_LED4 = 43;
#endif

// Define error state constants
                                                // LED State:
const int KNIGHT_RIDER = -1;                    // Knight Rider animation or 2000ms blink
const int STATE_NONE = 0;                       // ON: (nothing)
const int STATE_STARTED_WIFI_AP     = 15;       // ON: ALL or 3750ms blink
const int STATE_WIFI_NOT_CONNECTED  = 1;        // ON: 4 or 250ms blink
const int STATE_MQTT_NOT_CONNECTED  = 4;        // ON: 2 or 1000ms blink

const int INTERVAL_MULTIPLIER = 250;
const int MULTI_BLINKER_INTERVAL = 100;

class MultiBlinker {
public:
    MultiBlinker(int led1 = -1, int led2 = -1, int led3 = -1, int led4 = -1);
    void setState(int state);
    void start();
    void stop();

private:
    static void runTask(void *pvParameters);
    void run();
    void updateLEDs();
    void knightRider();

    int ledPins[4];
    int numLeds;
    int interval = 0;
    int currentState = STATE_NONE;
    int previousState = -2; // Initialize to a value that is not a valid state
    bool running = false;
    ulong lastUpdate = 0;
    TaskHandle_t taskHandle = NULL;
};

#endif // MULTIBLINKER_H