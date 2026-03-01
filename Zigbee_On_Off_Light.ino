// Zigbee On/Off Light für ESP32-C6 (Waveshare ESP32-C6-Zero)
// WS2812 an GPIO8
// ZigbeeMode=ed und PartitionScheme=zigbee müssen gesetzt sein

#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"
#include <Adafruit_NeoPixel.h>

/* Zigbee light bulb configuration */
#define ZIGBEE_LIGHT_ENDPOINT 10

/* WS2812 configuration */
#define PIXEL_PIN   8
#define PIXEL_COUNT 1

uint8_t button = BOOT_PIN;

/* Zigbee light object */
ZigbeeLight zbLight = ZigbeeLight(ZIGBEE_LIGHT_ENDPOINT);

/* NeoPixel object */
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

/********************* WS2812 control **************************/
void setLED(bool value) {
  if (value) {
    // AN → Weiß
    strip.setPixelColor(0, strip.Color(255, 255, 255));
  } else {
    // AUS
    strip.setPixelColor(0, 0);
  }
  strip.show();
}

/********************* Arduino Setup **************************/
void setup() {
  Serial.begin(115200);

  // Init WS2812
  strip.begin();
  strip.setBrightness(64);   // 0..255
  strip.show();              // aus

  // Init button for factory reset
  pinMode(button, INPUT_PULLUP);

  // Zigbee device name and model
  zbLight.setManufacturerAndModel("Espressif", "ZBLightBulb");

  // Callback when Zigbee light state changes
  zbLight.onLightChange(setLED);

  Serial.println("Adding ZigbeeLight endpoint to Zigbee Core");
  Zigbee.addEndpoint(&zbLight);

  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  }

  Serial.println("Connecting to network");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.println("Connected to Zigbee network");

  // Initial state anwenden
  setLED(zbLight.getLightState());
}

/********************* Main Loop **************************/
void loop() {

  // Button für Factory Reset oder manuelles Toggle
  if (digitalRead(button) == LOW) {
    delay(100);
    int startTime = millis();

    while (digitalRead(button) == LOW) {
      delay(50);

      // >3 Sekunden gedrückt → Factory Reset
      if ((millis() - startTime) > 3000) {
        Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }

    // Kurzer Druck → Licht toggeln
    zbLight.setLight(!zbLight.getLightState());
  }

  delay(100);
}
