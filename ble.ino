#include "ble.h"


/*
   BLE peripheral preferred connection parameters:
       - Minimum connection interval = MIN_CONN_INTERVAL * 1.25 ms, where MIN_CONN_INTERVAL ranges from 0x0006 to 0x0C80
       - Maximum connection interval = MAX_CONN_INTERVAL * 1.25 ms,  where MAX_CONN_INTERVAL ranges from 0x0006 to 0x0C80
       - The SLAVE_LATENCY ranges freadScaleTimerTimerom 0x0000 to 0x03E8
       - Connection supervision timeout = CONN_SUPERVISION_TIMEOUT * 10 ms, where CONN_SUPERVISION_TIMEOUT ranges from 0x000A to 0x0C80
*/
#define MIN_CONN_INTERVAL          0x0028 // 50ms.
#define MAX_CONN_INTERVAL          0x0190 // 500ms.
#define SLAVE_LATENCY              0x0000 // No slave latency.
#define CONN_SUPERVISION_TIMEOUT   0x03E8 // 10s.

#define SPACER " "

#define SCALE_PRODUCT_NAME "EzScale"
#define SCALE_PRODUCT_SUBNAME "Mk.1"
#define SCALE_PRODUCT_TAG "AAAAAA"

#define BLE_DEVICE_NAME SCALE_PRODUCT_NAME SPACER SCALE_PRODUCT_SUBNAME SPACER SCALE_PRODUCT_TAG
#define BLE_PERIPHERAL_APPEARANCE  BLE_APPEARANCE_GENERIC_WEIGHT_SCALE
#define BLE_OBJECT_NAME SCALE_PRODUCT_NAME SPACER SCALE_PRODUCT_SUBNAME
#define BLE_OBJECT_ID "ezscale-mk1"

#define CHARACTERISTIC_SCALE_STATUS_MAX_LEN          2
#define CHARACTERISTIC1_MAX_LEN                     20


/******************************************************
                 Status Definitions
 ******************************************************/
static byte SCALE_STATUS_INIT                   = 0x00;
static byte SCALE_STATUS_READY                  = 0x01;

static byte WIFI_STATUS_INIT                    = 0x00;
static byte WIFI_STATUS_NO_CREDENTIAL           = 0x01;
static byte WIFI_STATUS_CONNECTING              = 0x02;
static byte WIFI_STATUS_CONNECTED_WAIT_FOR_DHCP = 0x03;
static byte WIFI_STATUS_CONNECTED               = 0x04;

static byte WIFI_STATUS_CONNECT_TIMEOUT         = 0x10;
static byte WIFI_STATUS_DHCP_TIMEOUT            = 0x11;

static byte WIFI_STATUS_SCANNING                = 0x15;




/******************************************************
                 Variable Definitions
 ******************************************************/
static uint8_t service1_uuid[16]    =                          { 0x71, 0x3d, 0x00, 0x00, 0x50, 0x3e, 0x4c, 0x75, 0xba, 0x94, 0x31, 0x48, 0xf1, 0x8d, 0x94, 0x1e };
static uint8_t service1_characteristic_scale_status_uuid[16] = { 0x71, 0x3d, 0x00, 0x02, 0x50, 0x3e, 0x4c, 0x75, 0xba, 0x94, 0x31, 0x48, 0xf1, 0x8d, 0x94, 0x1e };
static uint8_t service1_json_gate_uuid[16] =                   { 0x71, 0x3d, 0x00, 0x01, 0x50, 0x3e, 0x4c, 0x75, 0xba, 0x94, 0x31, 0x48, 0xf1, 0x8d, 0x94, 0x1e };

// GAP and GATT characteristics value
static uint8_t  appearance[2] = {
  LOW_BYTE(BLE_PERIPHERAL_APPEARANCE),
  HIGH_BYTE(BLE_PERIPHERAL_APPEARANCE)
};

static uint8_t  change[4] = {
  0x00, 0x00, 0xFF, 0xFF
};

static uint8_t  conn_param[8] = {
  LOW_BYTE(MIN_CONN_INTERVAL), HIGH_BYTE(MIN_CONN_INTERVAL),
  LOW_BYTE(MAX_CONN_INTERVAL), HIGH_BYTE(MAX_CONN_INTERVAL),
  LOW_BYTE(SLAVE_LATENCY), HIGH_BYTE(SLAVE_LATENCY),
  LOW_BYTE(CONN_SUPERVISION_TIMEOUT), HIGH_BYTE(CONN_SUPERVISION_TIMEOUT)
};

static advParams_t adv_params = {
  .adv_int_min   = 0x0030,
  .adv_int_max   = 0x0030,
  .adv_type      = BLE_GAP_ADV_TYPE_ADV_IND,
  .dir_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
  .dir_addr      = {0, 0, 0, 0, 0, 0},
  .channel_map   = BLE_GAP_ADV_CHANNEL_MAP_ALL,
  .filter_policy = BLE_GAP_ADV_FP_ANY
};
uint8_t test[] = {
  SCALE_PRODUCT_NAME[0], SCALE_PRODUCT_NAME[1], SCALE_PRODUCT_NAME[2], SCALE_PRODUCT_NAME[3], SCALE_PRODUCT_NAME[4], SCALE_PRODUCT_NAME[5], SCALE_PRODUCT_NAME[6], 
};

static uint8_t adv_data[] = {
  0x02,
  BLE_GAP_AD_TYPE_FLAGS,
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,

  0x08,
  BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
  SCALE_PRODUCT_NAME[0], SCALE_PRODUCT_NAME[1], SCALE_PRODUCT_NAME[2], SCALE_PRODUCT_NAME[3], SCALE_PRODUCT_NAME[4], SCALE_PRODUCT_NAME[5], SCALE_PRODUCT_NAME[6],

  0x11,
  BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
  0x1e, 0x94, 0x8d, 0xf1, 0x48, 0x31, 0x94, 0xba, 0x75, 0x4c, 0x3e, 0x50, 0x00, 0x00, 0x3d, 0x71
};


static uint16_t character_scale_status_handle = 0x0000;
static uint16_t character_json_gate_handle = 0x0000;


static uint8_t characteristic_scale_status_data[CHARACTERISTIC_SCALE_STATUS_MAX_LEN] = {SCALE_STATUS_INIT, WIFI_STATUS_INIT};

static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN] = { 0x01 };

static btstack_timer_source_t serialInputTimer;
static int serialInputTimerTime = 200;

static btstack_timer_source_t pushNotificationTimer;
static int pushNotificationTimerTime = 100;

static btstack_timer_source_t readScaleTimer;
static int readScaleTimerTime = 100;

static btstack_timer_source_t readButtonTimer;
static int readButtonTimerTime = 100;

static String inBuffer = NULL;

struct NotificationMessage {
  uint8_t msg[20];
  int size;
};
static QueueList <NotificationMessage> notificationsQueue;

void bleInit() {
  ble.init();
  ble.debugLogger(true);
  ble.debugError(true);

  // Register BLE callback functions
  ble.onConnectedCallback(deviceConnectedCallback);
  ble.onDisconnectedCallback(deviceDisconnectedCallback);
  ble.onDataWriteCallback(gattWriteCallback);
  ble.onDataReadCallback(gattReadCallback);

  // Add GAP service and characteristics
  ble.addService(BLE_UUID_GAP);
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME, ATT_PROPERTY_READ, (uint8_t*)BLE_DEVICE_NAME, sizeof(BLE_DEVICE_NAME));
  ble.addCharacteristic(0x2ABE, ATT_PROPERTY_READ, (uint8_t*)BLE_OBJECT_NAME, sizeof(BLE_OBJECT_NAME)); //Object Name
  ble.addCharacteristic(0x2AC3, ATT_PROPERTY_READ, (uint8_t*) BLE_OBJECT_ID, sizeof(BLE_OBJECT_ID)); //Object ID

  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_APPEARANCE, ATT_PROPERTY_READ, appearance, sizeof(appearance));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_PPCP, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));

  ble.addService(BLE_UUID_GATT);
  ble.addCharacteristic(BLE_UUID_GATT_CHARACTERISTIC_SERVICE_CHANGED, ATT_PROPERTY_INDICATE, change, sizeof(change));

  ble.addService(service1_uuid);
  character_scale_status_handle = ble.addCharacteristicDynamic(service1_characteristic_scale_status_uuid, ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ, characteristic_scale_status_data, CHARACTERISTIC_SCALE_STATUS_MAX_LEN);
  character_json_gate_handle = ble.addCharacteristicDynamic(service1_json_gate_uuid, ATT_PROPERTY_NOTIFY | ATT_PROPERTY_WRITE | ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
  
  ble.setAdvertisementParams(&adv_params);
  ble.setAdvertisementData(sizeof(adv_data), adv_data);
  ble.startAdvertising();
  Serial.println("BLE start advertising.");

  serialInputTimer.process = &processSerialInput;
  ble.setTimer(&serialInputTimer, serialInputTimerTime);
  ble.addTimer(&serialInputTimer);

  pushNotificationTimer.process = &processPushNotif;
  ble.setTimer(&pushNotificationTimer, pushNotificationTimerTime);
  ble.addTimer(&pushNotificationTimer);

  readScaleTimer.process = &processReadScale;
  ble.setTimer(&readScaleTimer, readScaleTimerTime);
  ble.addTimer(&readScaleTimer);

  readButtonTimer.process = &processReadButton;
  ble.setTimer(&readButtonTimer, readButtonTimerTime);
  ble.addTimer(&readButtonTimer);

  setStatus(SCALE_STATUS_INIT, WIFI_STATUS_INIT);
  setScaleStatus(SCALE_STATUS_READY);
}

void pushNotificationMessage(const uint8_t batch[20], int batchSize) {
  NotificationMessage notif;
  for (int i = 0; i < 20; i++) {
    notif.msg[i] = batch[i];
  }
  notif.size = batchSize;
  notificationsQueue.push(notif);
}

String formatForSend(String jsonBase) {
  return String(((char)0x02)) + jsonBase + String(((char)0x03));
}

void sendNotification(String json) {
  Serial.println("sendNotification: " + json);
  int jsonLength = strlen(json);
  int batchSize = 20;
  int posCounter = 0;
  
  uint8_t batch[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  while (posCounter < jsonLength) {
    int batchContentSize = min(jsonLength - posCounter, batchSize);
    String str2 = json.substring(posCounter, posCounter + batchContentSize);
    for (int i = 0; i < batchContentSize; i++) {
      batch[i] = str2[i];
    }

    posCounter += batchContentSize;
    pushNotificationMessage(batch, batchContentSize);
    //ble.sendNotify(handle, batch, batchContentSize);
  }
}

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Ble: Device connected");
      break;
    default: break;
  }
}

void deviceDisconnectedCallback(uint16_t handle) {
  Serial.println("Ble: Device Disconnected");
}



int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  if (character_json_gate_handle == value_handle) {
    if (size > 0) {
      int i = 0;
      if (inBuffer == NULL && buffer[0] == 0x02) {
        i++;
        inBuffer = "";
      }

      for (; i < size; i++) {
        if (buffer[i] == 0x03) {
          onMessageWrite(inBuffer);
          inBuffer = NULL;
          return 0;
        }
        inBuffer += ((char)buffer[i]);
      }
    }
  }
  return 0;
}

uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size) {
  if (character_scale_status_handle == value_handle) {   // Characteristic value handle.
    memcpy(buffer, characteristic_scale_status_data, CHARACTERISTIC_SCALE_STATUS_MAX_LEN);
    return CHARACTERISTIC_SCALE_STATUS_MAX_LEN;
  }

  return 0;
}



void onMessageWrite(String json) {
  Serial.println("Read Message: " + json);
  JsonObject& jsonRoot = jsonBuffer.parseObject(json);
  const char* typeChar = jsonRoot["type"];
  String type = String(typeChar);
  if (type == "wifi-scan") { //WIFI SCAN
    wifiScan();
  } else if (type == "wifi-credential") { //WIFI CREDENTIAL
    const char* ssid = jsonRoot["ssid"];
    const char* secChar = jsonRoot["sec"];
    const char* password = jsonRoot["pwd"];
    wifiSetCredential(ssid, atoi(secChar), password);
  } else if (type == "wifi-info") {
    wifiSendConnectInfo();
  } else if (type == "user-sync") {
    const char* userId = jsonRoot["user-id"];
    bool result = syncUser(userId);
    String json = "{\"type\":\"user-sync\",\"result\":" + ((result == true) ? String("true") : String("false")) + "}";
    sendNotification(formatForSend(json));
  }
}


void pushNotification() {
  if (!notificationsQueue.isEmpty()) {
    NotificationMessage notif = notificationsQueue.pop();
    ble.sendNotify(character_json_gate_handle, notif.msg, notif.size);
  }
}

void setStatus(byte scaleStaus, byte wifiStatus) {
  characteristic_scale_status_data[0] = scaleStaus;
  characteristic_scale_status_data[1] = wifiStatus;
  ble.sendNotify(character_scale_status_handle, characteristic_scale_status_data, CHARACTERISTIC_SCALE_STATUS_MAX_LEN);
}

void setScaleStatus(byte scaleStaus) {
  characteristic_scale_status_data[0] = scaleStaus;
  ble.sendNotify(character_scale_status_handle, characteristic_scale_status_data, CHARACTERISTIC_SCALE_STATUS_MAX_LEN);
}

void setWifiStatus(byte wifiStatus) {
  characteristic_scale_status_data[1] = wifiStatus;
  ble.sendNotify(character_scale_status_handle, characteristic_scale_status_data, CHARACTERISTIC_SCALE_STATUS_MAX_LEN);
}

