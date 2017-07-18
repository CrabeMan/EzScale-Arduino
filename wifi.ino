#include "wifi.h"


void wifiInit() {
  WiFi.on();
  if (WiFi.hasCredentials()) {
    WiFiAccessPoint ap[1];
    int found = WiFi.getCredentials(ap, 1);
    if (found == 1) {
      Serial.println("Wifi: Credentials Found");
      Serial.print("Wifi: Trying to connect to: ");
      Serial.println(ap[0].ssid);
      connectWifi();
    } else {
       setWifiStatus(WIFI_STATUS_NO_CREDENTIAL);
       lcdPrintLine(0, "Wifi No Credential");
    }
    return;
  } else {
    Serial.println("Wifi No Credential");
    setWifiStatus(WIFI_STATUS_NO_CREDENTIAL);
    lcdPrintLine(0, "Wifi No Credential");
  }
}

static void connectWifi() {
  setWifiStatus(WIFI_STATUS_CONNECTING);
  lcdPrintLine(0, "Wifi Connecting");

  //WiFi.setCredentials("TP-LINK_4DE4","34221921");
  Serial.println("Wifi: Trying to connect");
  WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
  int connectCounter = 0;
  while (WiFi.connecting()) {
    Serial.print(".");
    //lcdPrint(".");
    delay(100);
    if (++connectCounter == 50) {//5 sec
      Serial.println("\nWifi: Connect Timeout");
      setWifiStatus(WIFI_STATUS_CONNECT_TIMEOUT);
      lcdPrintLine(0, "Wifi Connect Timeout");
      WiFi.disconnect();
      return;
    }
  }

  Serial.print(".");
  delay(100);

  /*if (WiFi.RSSI() == 0) {
    Serial.println("\nWifi: Not connected");
    setWifiStatus(WIFI_STATUS_CONNECT_TIMEOUT);
    WiFi.disconnect();
    return;
  }*/

  Serial.println("\nWifi: Connected");
  Serial.println("Wifi: Waiting for an IP address");
  setWifiStatus(WIFI_STATUS_CONNECTED_WAIT_FOR_DHCP);
  
  int dhcpCounter = 0;
  IPAddress localIP = WiFi.localIP();
  while (localIP[0] == 0) {
    localIP = WiFi.localIP();
    Serial.print(".");
    delay(100);

    if (++dhcpCounter == 100) {//10 sec
      Serial.println("\nWifi: DHCP Timeout");
      setWifiStatus(WIFI_STATUS_DHCP_TIMEOUT);
      WiFi.disconnect();
      return;
    }
  }
  Serial.println("\nWifi: Ip Ok");
  lcdPrintLine(0, "Wifi Connected");
  setWifiStatus(WIFI_STATUS_CONNECTED);

  lcdPrintLine(0, "Getting Users");
  pullUsers();
}

void wifiScanCallback(WiFiAccessPoint* wap, void* data) {
  sendNotification(
  String(((char)0x02))
    + "{\"type\":\"wifi-result\","
    + "\"ssid\":\"" + String(wap->ssid) + "\","
    + "\"sec\":" + wap->security + ","
    + "\"chan\":" + wap->channel + ","
    + "\"rssi\":" + wap->rssi
    + "}"
    + String(((char)0x03))
  );
}


void wifiScan() {
  byte currentStatus = characteristic_scale_status_data[1];
  setWifiStatus(WIFI_STATUS_SCANNING);
  WiFi.scan(wifiScanCallback);
  setWifiStatus(currentStatus);
}

void wifiSetCredential(const char* ssid, int security, const char* password) {
  //WiFi.clearCredentials();
  Serial.println("Set credentials and connect to Wifi");
  switch (security) {
    case 0:
      WiFi.setCredentials(ssid);
      break;
  
    case 1:
      WiFi.setCredentials(ssid, password, WEP, WLAN_CIPHER_NOT_SET);
      break;
  
    case 2:
    case 3:
      WiFi.setCredentials(ssid, password);
      break;
  }
  connectWifi();
}

void wifiSendConnectInfo() {
  const char* ssid = WiFi.SSID();
  IPAddress localIP = WiFi.localIP();

  byte bssid[6];
  byte mac[6];
  WiFi.BSSID(bssid);
  WiFi.macAddress(mac);
  
  char bssidChar[17];
  char macChar[17];

  sprintf(bssidChar, "%02X:%02X:%02X:%02X:%02X:%02X", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  sprintf(macChar, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  sendNotification(
  String(((char)0x02))
    + "{\"type\":\"wifi-info\","
    + "\"ssid\":\"" + ssid + "\","
    + "\"bssid\":\"" + bssidChar + "\","
    + "\"ip\":\"" + String(localIP) + "\","
    + "\"mac\":\"" + macChar + "\""
    + "}"
    + String(((char)0x03))
  );
}


