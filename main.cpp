#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DEVICE_AMOUNT 4
#define BUTTON1 0
#define BUTTON2 1
#define PHOTO_RES 2
#define POT 3
#define VIN 5
#define R 10000

#define SCREEN_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

struct Device{
  int pin;
  int value = 0;
  bool isDigital = true;
};

int photoResRawToPhys(int raw);
Device devices[DEVICE_AMOUNT];

void setup() {
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

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
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

  Serial.print("\n");
  delay(200);
}

int photoResRawToPhys(int raw){
  // Conversion rule
  float Vout = float(raw) * (VIN / float(1023));// Conversion analog to voltage
  float RLDR = (R * (VIN - Vout))/Vout; // Conversion voltage to resistance
  int phys=500/(RLDR/1000); // Conversion resitance to lumen
  return phys;
}
