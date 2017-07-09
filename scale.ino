#include "scale.h"

#define WEIGH_AVERAGE_RANGE 5

static HX711 scale(2, 3);


static float weighAverage[WEIGH_AVERAGE_RANGE];
static int weighIndex=0;
static bool catchWeigh = false;


void scaleInit() {
  scale.read();

  scale.set_scale(-19695.0f);
  scale.tare();
}


void scaleReadWeigh() {
  float weigh=scale.get_units(2);
  if (weigh < -0.5) return;
  float roundedWeigh = roundf(weigh * 10) / 10;
  Serial.print(weigh);
  Serial.print("       ");
  Serial.println(roundedWeigh);
  
  weighAverage[weighIndex++] = roundedWeigh;
  weighIndex %= WEIGH_AVERAGE_RANGE;

  float weighAverageTotal = 0;
  bool isStable = true;
  float lastWeigh = -1;
  for(int i = 0; i < WEIGH_AVERAGE_RANGE; i++){
    weighAverageTotal += weighAverage[i];
    if (weighAverage[i] != lastWeigh) {
      if (lastWeigh != -1) {
        isStable = false;
      }
      lastWeigh = weighAverage[i];
    }
  }
  
  
  if (isStable) {
    if (lastWeigh == 0) {
      catchWeigh = true;
    } else if (catchWeigh) {
      Serial.print("GO FOR   ");
      Serial.println(lastWeigh);
      catchWeigh = false;
    }
  }
}
