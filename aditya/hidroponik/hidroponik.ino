#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); 
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};

byte rowPins[ROWS]    = {19, 18, 5, 17}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte colPins[COLS]    = {16, 4, 0, 2};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int nutrisi[6];



void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("input nutrisi");
  
  for(int i = 0; i < 6; i++){
      nutrisi[i] = 0;
    }
  }
void loop() {
  char key = keypad.getKey();

  if(key){
    Serial.println(key);

    int week = key - '0';

    if(week >= 1 && week <= 6){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("minggu ke "); lcd.print(week);
      lcd.setCursor(0, 1);
      lcd.print("input nutrisi");

      while(!keypad.getKey());
      nutrisi[week-1] = keypad.getKey() - '0';

      lcd.setCursor(0, 2);
      lcd.print("tingkat nutrisi; "); lcd.print(nutrisi[week-1]); lcd.print(" g");
    }
  }

}
