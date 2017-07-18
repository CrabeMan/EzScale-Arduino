#ifndef H_SCALE
#define H_SCALE

#include <stdio.h>
#include <string.h>
#include "lib/HX711/HX711.cpp"
#include <Wire.h>
#include <math.h>
#include "HttpClient/firmware/HttpClient.cpp"

struct User {
  char* id;
  char* firstname;
  char* lastname;
};

void scaleInit();
void scaleReadWeigh();

bool pullUsers();
bool saveWeigh(User user, float weigh);
bool syncUser(const char* userId);


void printUserSelected();

void onBtnDown();
void onBtnUp();
void onBtnSet();

#endif

