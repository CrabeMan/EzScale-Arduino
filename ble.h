#ifndef H_BLE
#define H_BLE

#include <stdio.h>
#include <string.h>

#include "lib/QueueList.h"


void bleInit();

void pushNotificationMessage(const uint8_t batch[20], int batchSize);
String formatForSend(String jsonBase);
void sendNotification(String json);

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle);
void deviceDisconnectedCallback(uint16_t handle);


int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size);
uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size);

void onMessageWrite(String json);

void pushNotification();

void setStatus(byte scaleStaus, byte wifiStatus);
void setScaleStatus(byte scaleStaus);
void setWifiStatus(byte wifiStatus);

#endif
