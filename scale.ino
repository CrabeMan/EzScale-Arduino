#include "scale.h"

#define WEIGH_AVERAGE_RANGE 5
#define API_IP "192.168.1.42"
#define API_PORT 3000

static HX711 scale(D2, D3);

static float weighAverage[WEIGH_AVERAGE_RANGE];
static int weighIndex=0;
static bool catchWeigh = false;


HttpClient http;
http_header_t headers[] = {
    { "Content-Type", "application/json" },
    { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { "Authorization" , "7ddf32e17a6ac5ce04a8ecbf782ca509"},
    { NULL, NULL } //End
};

User* usersList = NULL;
int userSelected = -1;
int usersCount;

bool readyForSend = false;
float weighToSend = -1;


float lastWeighPrinted = -1;

void scaleInit() {
  scale.read();
  scale.set_scale(-19695.0f);
  scale.tare();
}


void scaleReadWeigh() {
  float weigh=scale.get_units(1);
  if (weigh == -1337) return;
  if (weigh < 0) {
    weigh = 0;
  }
  
  float roundedWeigh = roundf(weigh * 10) / 10;
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
  if (weighToSend != -1 && readyForSend == true) {
    return;
  }

  float weighAverage = weighAverageTotal / WEIGH_AVERAGE_RANGE;
  if (weighAverage < 0) {
    weighAverage = 0;
  }
  Serial.println(weighAverage);
  if (isStable) {
    if (lastWeigh == 0) {
      catchWeigh = true;
    } else if (catchWeigh) {
      Serial.print("GO FOR   ");
      Serial.println(lastWeigh);
      weighToSend = lastWeigh;
      readyForSend = true;
      catchWeigh = false;

      char buffer[8];
      sprintf(buffer, "%5.1f Ok", weighAverage);
      lcdPrintLine(1, buffer);

      printUserSelected();
      return;
    }
  }

  if (lastWeighPrinted != weighAverage) {
    char buffer[5];
    sprintf(buffer, "%5.1f", weighAverage);
    lcdPrintLine(1, buffer);
    lastWeighPrinted = weighAverage;
  }
}

bool pullUsers() {
  http_request_t request;
  http_response_t response;
  request.hostname = API_IP;
  request.port = API_PORT;
  request.path = "/scales/users";
  
  http.get(request, response, headers);
  if (response.status < 0) {
    return false;
  }
  Serial.print("Application>\tResponse status: ");
  Serial.println(response.status);
  
  Serial.print("Application>\tHTTP Response Body: ");
  Serial.println(response.body);
  
  StaticJsonBuffer<500> jsonBuffer;
  JsonArray& usersJson = jsonBuffer.parseArray(response.body);
  usersCount = usersJson.size();

  if (usersList != NULL) {
    delete usersList;
  }

  if (usersCount == 0) {
    lcdPrintLine(0, "No Linked Users");
  }
  
  usersList = (User*) malloc(usersCount * sizeof(User));
  for (int i = 0; i < usersCount; i++) {
    JsonObject& userJson = usersJson[i];
    const char* id = userJson["id"];
    const char* firstname = userJson["firstname"];
    const char* lastname = userJson["lastname"];
    Serial.print(id);
    Serial.print("       ");
    Serial.print(firstname);
    Serial.print("       ");
    Serial.println(lastname);

    usersList[i].id = (char*) malloc(strlen(id) * sizeof(char));
    strcpy(usersList[i].id, id);

    usersList[i].firstname = (char*) malloc(strlen(firstname) * sizeof(char));
    strcpy(usersList[i].firstname, firstname);

    usersList[i].lastname = (char*) malloc(strlen(lastname) * sizeof(char));
    strcpy(usersList[i].lastname, lastname);
  }

  userSelected = 0;
  printUserSelected();
  return true;
}


bool saveWeigh(User user, float weigh) {
  http_request_t request;
  http_response_t response;
  request.hostname = API_IP;
  request.port = API_PORT;
  request.path = "/scales/weightings";
  char body[150] = "";
  sprintf(body, "{\"idUser\":\"%s\",\"weight\": \"%f\"}", user.id, weigh);
  Serial.println(body);
  request.body = body;
  
  // Get request
  http.post(request, response, headers);
  Serial.print("Application>\tResponse status: ");
  Serial.println(response.status);

  if (response.status < 0) return false;
  
  Serial.print("Application>\tHTTP Response Body: ");
  Serial.println(response.body);

  return true;
}


bool syncUser(const char* userId) {
  http_request_t request;
  http_response_t response;
  request.hostname = API_IP;
  request.port = API_PORT;
  request.path = "/scales/user";
  char body[200] = "";
  sprintf(body, "{\"idUser\":\"%s\",\"scaleName\": \"%s %s\"}", userId, SCALE_PRODUCT_NAME, SCALE_PRODUCT_TAG);
  Serial.println(body);
  request.body = body;
  
  // Get request
  http.post(request, response, headers);
  Serial.print("Application>\tResponse status: ");
  Serial.println(response.status);

  if (response.status < 0) return false;
  
  Serial.print("Application>\tHTTP Response Body: ");
  Serial.println(response.body);

  return true;
}

void printUserSelected() {
  char line[16];
  sprintf(line, "%s %s", usersList[userSelected].firstname, usersList[userSelected].lastname);
  lcdPrintLine(0, line);
}


void onBtnDown() {
  if (userSelected == 0) {
    userSelected = usersCount-1;
  } else {
    userSelected--;
  }
  printUserSelected();
}


void onBtnUp() {
  userSelected++;
  userSelected%=usersCount;
  printUserSelected();
}

void onBtnSet() {
  if (usersList != NULL && weighToSend != -1 && readyForSend == true && userSelected > -1 && userSelected < usersCount) {
    lcdPrintLine(0, "Sending weigh");
    if (saveWeigh(usersList[userSelected], weighToSend) == true) {
      lcdPrintLine(0, "Weigh Saved");
    } else {
      lcdPrintLine(0, "Error while save");
    }
    readyForSend = false;
    weighToSend = -1;
  }
}

