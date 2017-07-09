#if defined(ARDUINO)
SYSTEM_MODE(SEMI_AUTOMATIC);
#endif

#include "EzScale-Arduino.h"


void setup() {
  Serial.begin(115200);
  scaleInit();

  //ble.debugLogger(true);
  // Initialize ble_stack.
  bleInit();

  wifiInit();
}

void loop() {
}

//HttpClient http;
//http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
//    { "Accept" , "*/*"},
//    { NULL, NULL } // NOTE: Always terminate headers will NULL
//};
/*http_request_t request;
http_response_t response;
void loop() {
    if (nextTime > millis()) {
        return;
    }

    Serial.println();
    Serial.println("Application>\tStart of Loop.");
    // Request path and body can be set at runtime or at setup.
    request.hostname = "api.timezonedb.com";
    request.port = 80;
    request.path = "/v2/list-time-zone";

    // The library also supports sending a body with your request:
    //request.body = "{\"key\":\"value\"}";

    // Get request
    http.get(request, response, headers);
    Serial.print("Application>\tResponse status: ");
    Serial.println(response.status);

    Serial.print("Application>\tHTTP Response Body: ");
    Serial.println(response.body);

    nextTime = millis() + 10000;
}*/

