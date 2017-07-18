#if defined(ARDUINO)
SYSTEM_MODE(SEMI_AUTOMATIC);
#endif

SYSTEM_THREAD(ENABLED);

#include <ArduinoJson.h>
static StaticJsonBuffer<500> jsonBuffer;
#include "EzScale-Arduino.h"


void setup() {
  Serial.begin(115200);
  Serial.println("Hello");
  scaleInit();
  buttonsInit();
  lcdInit();
  lcdPrintLine(0, "Hello !");

  bleInit();
  wifiInit();
}

void loop() {
}

