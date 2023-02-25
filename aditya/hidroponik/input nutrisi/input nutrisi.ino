#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
int nilai[6];
LiquidCrystal_I2C lcd(0x27, 20, 4);
const byte BARIS = 4;
const byte KOLOM = 4;
char buff[16];
char tombol[BARIS][KOLOM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pinBaris[BARIS] = { 11,10,9,8 };
byte pinKolom[KOLOM] = { 7,6,5,4 };
int cursorColom = 0;
Keypad customKeypad = Keypad(makeKeymap(tombol), pinBaris, pinKolom, BARIS, KOLOM);

void setup() {
  lcd.begin(4, 20);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("input nutrisi:");
}

void loop() {
  //float tds = analogRead();

  float nilai = getFloatFromKeypad(buff);
  
}

float getFloatFromKeypad(char* arr){
  while(1){
    char keypressed = customKeypad.getKey();
    if (keypressed != NO_KEY ) {
      if (keypressed=='D' || cursorColom>3){
        cursorColom = 0;
        lcd.clear();
        lcd.print("masukkan ke");
        return atof(arr);
        
      }else{
        lcd.setCursor(cursorColom, 1);
        lcd.print(keypressed);
        arr[cursorColom] = keypressed;
        cursorColom++;
      }
    }
  }
}