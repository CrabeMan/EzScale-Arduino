#ifndef H_LCD
#define H_LCD

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

void lcdInit();
void lcdPrintLine(uint8_t  row, const char c[]);
void lcdPrint(const char c[]);

#endif
