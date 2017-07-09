#ifndef H_WIFI
#define H_WIFI

void wifiInit();

static void connectWifi();
void wifiScanCallback(WiFiAccessPoint* wap, void* data);

void wifiScan();
void wifiSetCredential(const char* ssid, int security, const char* password);
void wifiSendConnectInfo();

#endif
