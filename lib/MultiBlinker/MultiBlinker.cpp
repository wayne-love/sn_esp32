#include "MultiBlinker.h"

#include <RemoteDebug.h>

MultiBlinker::MultiBlinker(int led1, int led2, int led3, int led4) {
    ledPins[0] = led1;
    ledPins[1] = led2;
    ledPins[2] = led3;
    ledPins[3] = led4;
    numLeds = 0;
    for (int i = 0; i < 4; i++) {
        if (ledPins[i] != -1) {
            numLeds++;
            pinMode(ledPins[i], OUTPUT);
        }
    }
}

void MultiBlinker::setState(int state) {
    if (numLeds == 0) {
        return;
    }
    if (state == currentState) {
        return;
    }
    currentState = state;
    interval = INTERVAL_MULTIPLIER * state;
}

void MultiBlinker::start() {
    if (numLeds == 0) {
        return;
    }
    running = true;
    xTaskCreate(runTask, "MultiBlinkerTask", 2048, this, 1, &taskHandle);
}

void MultiBlinker::stop() {
    if (numLeds == 0) {
        return;
    }
    running = false;
    if (taskHandle != NULL) {
        vTaskDelete(taskHandle);
        taskHandle = NULL;
    }
}

void MultiBlinker::runTask(void *pvParameters) {
    MultiBlinker *blinker = static_cast<MultiBlinker*>(pvParameters);
    blinker->run();
}

void MultiBlinker::run() {
    while (running) {
        ulong currentTime = millis();
        if (currentTime - lastUpdate >= MULTI_BLINKER_INTERVAL) {
            lastUpdate = currentTime;
            if (currentState == -1) {
                knightRider();
            } else {
                updateLEDs();
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task from hogging the CPU
    }
}

void MultiBlinker::updateLEDs() {
    if (numLeds == 1) {
        // Blink the single LED using the interval value
        bool newState = !digitalRead(ledPins[0]);
        digitalWrite(ledPins[0], newState);
    } else if (numLeds == 4) {
        for (int i = 0; i < 4; i++) {
            digitalWrite(ledPins[i], (currentState & (1 << i)) ? HIGH : LOW);
        }
    }
}

void MultiBlinker::knightRider() {
    static int direction = 1;
    static int position = 0;

    for (int i = 0; i < 4; i++) {
        digitalWrite(ledPins[i], LOW);
    }
    digitalWrite(ledPins[position], HIGH);

    position += direction;
    if (position == 3 || position == 0) {
        direction = -direction;
        // Keep the end LEDs on for a longer duration
        vTaskDelay(150 / portTICK_PERIOD_MS); // Adjust the delay as needed
    }
}