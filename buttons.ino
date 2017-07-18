#include "buttons.h"

const int btnPinDown = 6;
const int btnPinUp = 5;
const int btnPinSet = 4;

void buttonsInit() {
  pinMode(btnPinDown, INPUT);
  pinMode(btnPinUp, INPUT);
  pinMode(btnPinSet, INPUT);
}


void buttonsRead(int r[3]) {
  r[0] = digitalRead(btnPinDown);
  r[1] = digitalRead(btnPinUp);
  r[2] = digitalRead(btnPinSet);
}


