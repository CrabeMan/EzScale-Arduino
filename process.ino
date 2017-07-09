#include "process.h"

static void processPushNotif(btstack_timer_source_t *ts) {
  pushNotification();
  ble.setTimer(ts, pushNotificationTimerTime);
  ble.addTimer(ts);
}

static void processSerialInput(btstack_timer_source_t *ts) {
  if (Serial.available()) {
    String str = Serial.readStringUntil('\n');
    String json = "{\"type\": \"serial-in\",\"msg\": \"" + str + "\"}";
    sendNotification(formatForSend(json));
  }
  ble.setTimer(ts, serialInputTimerTime);
  ble.addTimer(ts);
}

static void processReadScale(btstack_timer_source_t *ts) {
  scaleReadWeigh();
  ble.setTimer(ts, readScaleTimerTime);
  ble.addTimer(ts);
}
