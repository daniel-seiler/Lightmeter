#ifndef LIGHTMETER_CONFIG_HPP
#define LIGHTMETER_CONFIG_HPP

#include <Arduino.h>

// Prints the raw inputs and the frame time on the serial port
//#define LIGHTMETER_DEBUG

#define BUTTON1_PIN 2
#define BUTTON2_PIN 3
#define PHOTO_RES_PIN A0
#define POT_PIN A1

#define SCREEN_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define ANALOG_MAX 1023

#define FRAME_MS 33
#define DEBOUNCE_MS 20
#define ANALOG_SAMPLE_MS 10
#define SAVE_DELAY_MS 3000

// Added to the measured EV, in 1/16 stops
#define EV_CALIBRATION 32

#endif //LIGHTMETER_CONFIG_HPP
