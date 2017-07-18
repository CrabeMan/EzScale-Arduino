#include "lcd.h"

LiquidCrystal_I2C lcd(0x27,16,2);

void lcdInit() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
}


void lcdPrintLine(uint8_t  row, const char c[]) {
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(c);
}


void lcdPrint(const char c[]) {
  lcd.print(c);
}

