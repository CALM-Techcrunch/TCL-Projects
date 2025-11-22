 
#include <LiquidCrystal_I2C.h>
#include "LS_config.h"

LiquidCrystal_I2C lcd(0x27,16,2);

void lcd_init(void){
  // Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  Row1("Tech Crunch LTD");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("L1");
  lcd.setCursor(5,0);
  lcd.print("L2");
  lcd.setCursor(10,0);
  lcd.print("L3");
  string1("OFF");
  string2("OFF");
  string3("OFF");
}

void Row1(char firstPrint[]){
  clearLine(1);
  lcd.setCursor(0,0);
  lcd.print(firstPrint);
}

void Row2(char secondPrint[]){
  clearLine(1);
  lcd.setCursor(0,1);
  lcd.print(secondPrint);
}

void clearLine(int line) {
  lcd.setCursor(0, line); 
  lcd.print("                "); // Write 16 spaces to overwrite the line
}

void Rly_status(char one[],char two[],char three[]){
   
//  Second line
 lcd.setCursor(0,1);
 lcd.print(one);
 lcd.setCursor(5,1);
 lcd.print(two);
 lcd.setCursor(10,1);
 lcd.print(three);

}

void string1(const char one[]){
  lcd.setCursor(0,1);
  lcd.print("   ");

  lcd.setCursor(0,1);
  lcd.print(one);
}

void string2(const char two[]){
  lcd.setCursor(5,1);
  lcd.print("   ");

  lcd.setCursor(5,1);
 lcd.print(two);
}

void string3(const char three[]){
  lcd.setCursor(10,1);
  lcd.print("   ");

  lcd.setCursor(10,1);
 lcd.print(three);
}


