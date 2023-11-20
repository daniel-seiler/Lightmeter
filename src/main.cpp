#include <stdio.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "bitmap.hpp"
#include "bitmap_generated.hpp"

#define DEVICE_AMOUNT 4
#define BUTTON1 0
#define BUTTON2 1
#define PHOTO_RES 2
#define POT 3
#define VIN 5
#define R 10000
#define POT_MAX 1023.0
#define F_ARR_SIZE 19
#define T_ARR_SIZE 19
#define ISO_ARR_SIZE 6

#define SCREEN_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

enum State {
    SET_F,
    SET_T,
    SET_ISO
} state, prevState;

struct Device {
    int pin;
    int value = 0;
    bool isDigital = true;
};

uint16_t iso[] = {100, 200, 400, 800, 1600, 3000};
float fNum[] = {0.5, 0.7, 1.0, 1.4, 2.0, 2.8, 4.0, 5.6, 8.0, 11.0, 16.0,
                22.0, 32.0, 45.0, 64.0, 90.0, 128.0, 180.0, 256.0};
float exposureTime[] = {0.00012499, 0.00024999, 0.0004999, 0.0009999, 0.001999, 0.003999,
                        0.00799, 0.016666666, 0.033333333, 0.066666666, 0.125, 0.25, 0.5,
                        1, 2, 4, 8, 15, 30};
float currentF, currentT;
uint16_t currentISO;

int photoResRawToPhys(int raw);

int analogToIndex(float analogVal, float analogMax, int arrSize);

void readInputs();

void exposureToString(float exposure, char *buffer);

void drawScreen(void);

void drawIso(uint16_t iso);

void drawT(float t);

void drawF(float f);

Device devices[DEVICE_AMOUNT];

void setup() {
    state = prevState = State::SET_F;
    currentF = fNum[0];
    currentT = exposureTime[0];
    currentISO = iso[1];
    devices[BUTTON1].pin = 2;
    devices[BUTTON2].pin = 3;
    devices[PHOTO_RES].pin = A0;
    devices[PHOTO_RES].isDigital = false;
    devices[POT].pin = A1;
    devices[POT].isDigital = false;

    for (int i = 0; i < DEVICE_AMOUNT; i++) {
        pinMode(devices[i].pin, INPUT);
    }

    Serial.begin(9600);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    drawScreen();
}

void loop() {
    readInputs();

    if (devices[BUTTON1].value == 1 && devices[BUTTON2].value == 1) {
        prevState = (state == State::SET_ISO) ? prevState : state;
        state = State::SET_ISO;
    } else {
        if (state == State::SET_ISO) {
            state = prevState;
        }
        if (devices[BUTTON1].value == 1 && state == prevState) {
            if (state == State::SET_F) {
                state = State::SET_T;
            } else {
                state = State::SET_F;
            }
        }
        if (devices[BUTTON1].value == 0 && state != prevState) {
            prevState = state;
        }
    }

    switch(state) {
        case State::SET_F:
            currentF = fNum[analogToIndex(devices[POT].value, POT_MAX, F_ARR_SIZE)];
            break;
        case State::SET_T:
            currentT = exposureTime[analogToIndex(devices[POT].value, POT_MAX, T_ARR_SIZE)];
            break;
        case State::SET_ISO:
            currentISO = iso[analogToIndex(devices[POT].value, POT_MAX, ISO_ARR_SIZE)];
            break;
    }

    display.clearDisplay();
    drawScreen();
    drawIso(currentISO);
    drawF(currentF);
    drawT(currentT);
    display.display();

    delay(200);
}

int photoResRawToPhys(int raw) {
    // Conversion rule
    float Vout = float(raw) * (VIN / POT_MAX);// Conversion analog to voltage
    float RLDR = (R * (VIN - Vout)) / Vout; // Conversion voltage to resistance
    int phys = 500 / (RLDR / 1000); // Conversion resitance to lumen
    return phys;
}

int analogToIndex(float analogVal, float analogMax, int arrSize) {
    int calculatedIndex = (analogVal <= 0) ? 0 : analogVal / analogMax * arrSize;
    return (calculatedIndex >= arrSize) ? arrSize - 1 : calculatedIndex;
}

void readInputs() {
    for (int i = 0; i < DEVICE_AMOUNT; i++) {
        if (devices[i].isDigital) {
            devices[i].value = digitalRead(devices[i].pin);
        } else {
            devices[i].value = analogRead(devices[i].pin);
        }

        devices[PHOTO_RES].value = photoResRawToPhys(devices[PHOTO_RES].value);

        Serial.print(devices[i].value);
        Serial.print(" : ");
    }
    Serial.print("\r\n");
}

void exposureToString(float exposure, char *buffer) {
    if (0 == (int) exposure) {
        int denominator = (int) (1 / exposure);
        sprintf(buffer, "1/%d", denominator);
    } else {
        sprintf(buffer, "   %d", (int) (exposure));
    }
}

void drawScreen(void) {
    bitmap::drawGearLeft(&display, bitmap::GEAR_LEFT_X, bitmap::GEAR_LEFT_Y);
    bitmap::drawGearRight(&display, bitmap::GEAR_RIGHT_X, bitmap::GEAR_RIGHT_Y);
    bitmap::drawCircleLeft(&display, bitmap::CIRCLE_LEFT_X, bitmap::CIRCLE_LEFT_Y);
    bitmap::drawCircleRight(&display, bitmap::CIRCLE_RIGHT_X, bitmap::CIRCLE_RIGHT_Y);
    //bitmap::drawArrowActiveTl(&display, bitmap::ARROW_TL_X, bitmap::ARROW_TL_Y);
    //bitmap::drawArrowActiveTr(&display, bitmap::ARROW_TR_X, bitmap::ARROW_TR_Y);
    //bitmap::drawArrowActiveBl(&display, bitmap::ARROW_BL_X, bitmap::ARROW_BL_Y);
    //bitmap::drawArrowActiveBr(&display, bitmap::ARROW_BR_X, bitmap::ARROW_BR_Y);
    bitmap::drawArrowInactiveTl(&display, bitmap::ARROW_TL_X, bitmap::ARROW_TL_Y);
    bitmap::drawArrowInactiveTr(&display, bitmap::ARROW_TR_X, bitmap::ARROW_TR_Y);
    bitmap::drawArrowInactiveBl(&display, bitmap::ARROW_BL_X, bitmap::ARROW_BL_Y);
    bitmap::drawArrowInactiveBr(&display, bitmap::ARROW_BR_X, bitmap::ARROW_BR_Y);
}

void drawIso(uint16_t iso) {
    if (iso < 1000) {
        // Set cursor for 3-digit iso
        display.setCursor(55, 2);
    } else {
        // Set cursor for 4-digit iso
        display.setCursor(52, 2);
    }
    display.print(iso);
    // Draw line over "ISO" text
    display.drawLine(55, 10, 72, 10, SSD1306_WHITE);
    display.setCursor(55, 11);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.print("ISO");
    display.setTextColor(SSD1306_WHITE);
}

void drawT(float t) {
    char buffer[7];
    exposureToString(t, buffer);
    display.setCursor(0, 29);
    display.print("T");
    display.setCursor(12, 29);
    display.print(buffer);
}

void drawF(float f) {
    display.setCursor(116, 29);
    display.print("f/");
    display.setCursor(85, 29);
    display.print(f);
}
