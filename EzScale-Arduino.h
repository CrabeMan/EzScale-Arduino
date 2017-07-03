#include <stdio.h>
#include <string.h>
#include <QueueList.h>
#include <ArduinoJson.h>

void pushNotificationMessage(const uint8_t batch[20], int batchSize);
String formatForSend(String jsonBase);
void sendNotification(String json);

void setStatus(byte scaleStaus, byte wifiStatus);
void setScaleStatus(byte scaleStaus);
void setWifiStatus(byte wifiStatus);

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle);
void deviceDisconnectedCallback(uint16_t handle);

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size);
uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size);

static void processPushNotif(btstack_timer_source_t *ts);
static void processSerialInput(btstack_timer_source_t *ts);

void onMessageWrite(String json);
static void connectWifi();
void wifiScanCallback(WiFiAccessPoint* wap, void* data);

void setup();
void loop();
