#include <stdio.h>
#include <string.h>

#if defined(ARDUINO)
SYSTEM_MODE(SEMI_AUTOMATIC);
#endif

/*
   BLE peripheral preferred connection parameters:
       - Minimum connection interval = MIN_CONN_INTERVAL * 1.25 ms, where MIN_CONN_INTERVAL ranges from 0x0006 to 0x0C80
       - Maximum connection interval = MAX_CONN_INTERVAL * 1.25 ms,  where MAX_CONN_INTERVAL ranges from 0x0006 to 0x0C80
       - The SLAVE_LATENCY ranges from 0x0000 to 0x03E8
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

#define CHARACTERISTIC_SCALE_STATUS_MAX_LEN          8
#define CHARACTERISTIC1_MAX_LEN                     20
#define CHARACTERISTIC2_MAX_LEN                     20
#define TXRX_BUF_LEN                                20


/******************************************************
                 Status Definitions
 ******************************************************/
#define SCALE_STATUS_INIT 0x41;
#define SCALE_STATUS_READY 0x42;

#define WIFI_STATUS_INIT 0x41;
#define WIFI_STATUS_NO_CREDENTIAL 0x42;
#define WIFI_STATUS_CONNECTING 0x43;
#define WIFI_STATUS_CONNECTED_WAIT_FOR_DHCP 0x44;
#define WIFI_STATUS_CONNECTED 0x45;

#define WIFI_STATUS_CONNECT_TIMEOUT 0x50;




/******************************************************
                 Variable Definitions
 ******************************************************/
static uint8_t service1_uuid[16]    = { 0x71, 0x3d, 0x00, 0x00, 0x50, 0x3e, 0x4c, 0x75, 0xba, 0x94, 0x31, 0x48, 0xf1, 0x8d, 0x94, 0x1e };
static uint8_t service1_characteristic_scale_status_uuid[16] = { 0x71, 0x3d, 0x00, 0x06, 0x50, 0x3e, 0x4c, 0x75, 0xba, 0x94, 0x31, 0x48, 0xf1, 0x8d, 0x94, 0x1e };

static uint8_t service1_tx_uuid[16] = { 0x71, 0x3d, 0x00, 0x03, 0x50, 0x3e, 0x4c, 0x75, 0xba, 0x94, 0x31, 0x48, 0xf1, 0x8d, 0x94, 0x1e };
static uint8_t service1_rx_uuid[16] = { 0x71, 0x3d, 0x00, 0x02, 0x50, 0x3e, 0x4c, 0x75, 0xba, 0x94, 0x31, 0x48, 0xf1, 0x8d, 0x94, 0x1e };

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

/*
   BLE peripheral advertising parameters:
       - advertising_interval_min: [0x0020, 0x4000], default: 0x0800, unit: 0.625 msec
       - advertising_interval_max: [0x0020, 0x4000], default: 0x0800, unit: 0.625 msec
       - advertising_type:
             BLE_GAP_ADV_TYPE_ADV_IND
             BLE_GAP_ADV_TYPE_ADV_DIRECT_IND
             BLE_GAP_ADV_TYPE_ADV_SCAN_IND
             BLE_GAP_ADV_TYPE_ADV_NONCONN_IND
       - own_address_type:
             BLE_GAP_ADDR_TYPE_PUBLIC
             BLE_GAP_ADDR_TYPE_RANDOM
       - advertising_channel_map:
             BLE_GAP_ADV_CHANNEL_MAP_37
             BLE_GAP_ADV_CHANNEL_MAP_38
             BLE_GAP_ADV_CHANNEL_MAP_39
             BLE_GAP_ADV_CHANNEL_MAP_ALL
       - filter policies:
             BLE_GAP_ADV_FP_ANY
             BLE_GAP_ADV_FP_FILTER_SCANREQ
             BLE_GAP_ADV_FP_FILTER_CONNREQ
             BLE_GAP_ADV_FP_FILTER_BOTH

   Note:  If the advertising_type is set to BLE_GAP_ADV_TYPE_ADV_SCAN_IND or BLE_GAP_ADV_TYPE_ADV_NONCONN_IND,
          the advertising_interval_min and advertising_interval_max should not be set to less than 0x00A0.
*/
static advParams_t adv_params = {
  .adv_int_min   = 0x0030,
  .adv_int_max   = 0x0030,
  .adv_type      = BLE_GAP_ADV_TYPE_ADV_IND,
  .dir_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
  .dir_addr      = {0, 0, 0, 0, 0, 0},
  .channel_map   = BLE_GAP_ADV_CHANNEL_MAP_ALL,
  .filter_policy = BLE_GAP_ADV_FP_ANY
};

static uint8_t adv_data[] = {
  0x02,
  BLE_GAP_AD_TYPE_FLAGS,
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,

  0x08,
  BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
  'B', 'i', 's', 'c', 'u', 'i', 't',

  0x11,
  BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
  0x1e, 0x94, 0x8d, 0xf1, 0x48, 0x31, 0x94, 0xba, 0x75, 0x4c, 0x3e, 0x50, 0x00, 0x00, 0x3d, 0x71
};


static uint16_t character_scale_status_handle = 0x0000;

static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;


static uint8_t characteristic_scale_status_data[CHARACTERISTIC_SCALE_STATUS_MAX_LEN] = { 0x00 };

static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN] = { 0x01 };
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN] = { 0x00 };

static btstack_timer_source_t serialInputTimer;

char rx_buf[TXRX_BUF_LEN];
static uint8_t rx_state = 0;

static int outMsgNum = 0;

static String inBuffer = "";
static int inMsg = -1;
static int inBatchPos = 0;

/******************************************************
                 Function Definitions
 ******************************************************/
void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Device connected!");
      break;
    default: break;
  }
}

void deviceDisconnectedCallback(uint16_t handle) {
  Serial.println("Disconnected.");
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  Serial.print("Write value handler: ");
  Serial.println(value_handle, HEX);

  if (character1_handle == value_handle) {
    Serial.println((char*) buffer);
    Serial.println(size);
    /*int inMsgCurrent = value_handle[0];
    int inBatchPosCurrent = value_handle[1];

    if ((inMsgCurrent == inMsg || inMsgCurrent == -1) && inBatchPosCurrent == inBatchPos + 1) {
      for (int i = 3; i < 20; i++) {
        if (value_handle[i] != 0x00) {
          inBuffer += buffer[i];
        } else {
          onMessageReceive(inBuffer);
          inBuffer = "";
          inMsgCurrent = -1;
          inBatchPos = -1;
          break;
        }
      }

      inBatchPos = inBatchPosCurrent;
      inMsg = inMsgCurrent;
    }*/
  }
  return 0;
}

uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size) {

  Serial.print("Reads attribute value, handle: ");
  Serial.println(value_handle, HEX);

  if (character_scale_status_handle == value_handle) {   // Characteristic value handle.
    Serial.println(buffer_size);
    Serial.println(sizeof(characteristic2_data));
    Serial.println((char*)characteristic2_data);

    memcpy(buffer, characteristic_scale_status_data, CHARACTERISTIC_SCALE_STATUS_MAX_LEN);
    return CHARACTERISTIC_SCALE_STATUS_MAX_LEN;
  }

  return 0;
}

/*void m_uart_rx_handle() {   //update characteristic data
  ble.sendNotify(character2_handle, rx_buf, CHARACTERISTIC2_MAX_LEN);
  memset(rx_buf, 0x00,20);
  rx_state = 0;
  }*/

static void notifySerialInput(btstack_timer_source_t *ts) {
  if (Serial.available()) {
    String str = Serial.readStringUntil('\n');
    String json = "{\"type\": \"serial-in\",\"msg\": \"" + str + "\"}";
    Serial.println(json);
    sendNotification(character2_handle, json);
    //setScaleStatus(00, "05812");
  }
  ble.setTimer(ts, 200);
  ble.addTimer(ts);
}

static void setScaleStatus(const int status, const char* weight) {
  Serial.println("setScaleStatus");
  characteristic_scale_status_data[0] = 0x41;
  characteristic_scale_status_data[1] = 0x41;
  characteristic_scale_status_data[2] = 0x20;
  characteristic_scale_status_data[3] = weight[0];
  characteristic_scale_status_data[4] = weight[1];
  characteristic_scale_status_data[5] = weight[2];
  characteristic_scale_status_data[6] = weight[3];
  characteristic_scale_status_data[7] = weight[4];
  ble.sendNotify(character_scale_status_handle, characteristic_scale_status_data, CHARACTERISTIC2_MAX_LEN);
}

void sendNotification(uint16_t handle, String json) {
  int jsonLength = strlen(json);
  int batchSize = 18;
  int msgPos = ++outMsgNum;
  int posCounter = 0;
  int batchCounter = 0;
  while (posCounter < jsonLength) {
    Serial.println(jsonLength);
    Serial.println(posCounter);
    int batchContentSize = min(jsonLength - posCounter, batchSize);
    Serial.println(batchContentSize);
    String str2 = json.substring(posCounter, posCounter + batchContentSize);
    Serial.println(str2);
    uint8_t data[20] = {msgPos, batchCounter + '0', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < batchContentSize; i++) {
      data[2 + i] = str2[i];
    }

    for (uint8_t index = 0; index < 20; index++) {
      Serial.print(data[index], HEX);
    }
    Serial.println();


    posCounter += batchContentSize;
    //ble.sendNotify(handle, data, 20);
  }
}


void onMessageReceive(String str) {
  Serial.print("onMessageReceive: ");
  Serial.print(str);
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("Simple Chat demo.");

  //ble.debugLogger(true);
  // Initialize ble_stack.
  ble.init();

  // Register BLE callback functions
  ble.onConnectedCallback(deviceConnectedCallback);
  ble.onDisconnectedCallback(deviceDisconnectedCallback);
  ble.onDataWriteCallback(gattWriteCallback);
  ble.onDataReadCallback(gattReadCallback);

  // Add GAP service and characteristics
  ble.addService(BLE_UUID_GAP);
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME, ATT_PROPERTY_READ, (uint8_t*)BLE_DEVICE_NAME, sizeof(BLE_DEVICE_NAME));
  ble.addCharacteristic(0x2ABE/* Object Name */, ATT_PROPERTY_READ, (uint8_t*)BLE_OBJECT_NAME, sizeof(BLE_OBJECT_NAME));
  ble.addCharacteristic(0x2AC3/* Object ID */, ATT_PROPERTY_READ, (uint8_t*) BLE_OBJECT_ID, sizeof(BLE_OBJECT_ID));

  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_APPEARANCE, ATT_PROPERTY_READ, appearance, sizeof(appearance));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_PPCP, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));

  // Add GATT service and characteristics
  ble.addService(BLE_UUID_GATT);
  ble.addCharacteristic(BLE_UUID_GATT_CHARACTERISTIC_SERVICE_CHANGED, ATT_PROPERTY_INDICATE, change, sizeof(change));

  // Add user defined service and characteristics
  ble.addService(service1_uuid);
  character_scale_status_handle = ble.addCharacteristicDynamic(service1_characteristic_scale_status_uuid, ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ, characteristic_scale_status_data, CHARACTERISTIC_SCALE_STATUS_MAX_LEN);

  character1_handle = ble.addCharacteristicDynamic(service1_tx_uuid, ATT_PROPERTY_NOTIFY | ATT_PROPERTY_WRITE | ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
  character2_handle = ble.addCharacteristicDynamic(service1_rx_uuid, ATT_PROPERTY_NOTIFY | ATT_PROPERTY_READ, characteristic2_data, CHARACTERISTIC2_MAX_LEN);

  // Set BLE advertising parameters
  ble.setAdvertisementParams(&adv_params);

  // // Set BLE advertising data
  ble.setAdvertisementData(sizeof(adv_data), adv_data);

  // BLE peripheral starts advertising now.
  ble.startAdvertising();
  Serial.println("BLE start advertising.");

  // set one-shot timer
  serialInputTimer.process = &notifySerialInput;
  ble.setTimer(&serialInputTimer, 500);//100ms
  ble.addTimer(&serialInputTimer);
}

void loop() {

}

