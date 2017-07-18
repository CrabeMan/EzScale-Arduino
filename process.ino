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

    //lcdPrintLine(0,0, str);
  }
  ble.setTimer(ts, serialInputTimerTime);
  ble.addTimer(ts);
}

static void processReadScale(btstack_timer_source_t *ts) {
  scaleReadWeigh();
  ble.setTimer(ts, readScaleTimerTime);
  ble.addTimer(ts);
}

static void processReadButton(btstack_timer_source_t *ts) {
  int btn[3];
  buttonsRead(btn);
  if (btn[0] == LOW) {
    onBtnDown();
  } else if (btn[1] == LOW) {
    onBtnUp();
  } else if (btn[2] == LOW) {
    onBtnSet();
  }
  ble.setTimer(ts, readButtonTimerTime);
  ble.addTimer(ts);
}

