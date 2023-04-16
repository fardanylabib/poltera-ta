#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <DS3231.h>
#include <LinearRegression.h>
#include "GravityTDS.h"
#define TdsSensorPin A1
#define DS3231_I2C_ADDRESS 104
float calibration_value = 20.24 - 0.7;  //21.34 - 0.7
int phval = 0;
unsigned long int avgval;
int buffer_arr[10], temp;
float ph_act;
//const int phSensorPin = A0;
//float Po = 0;
float nilaiP[6];
float nilaiph[6];
float mph[7];
float bph[7];
float phValue = 0, valueP = 0;
double nilaiKalibrasiph[2] = { 0, 0 };
float nilai[6];
float nilaiTDS[6];
float mTDS[7];
float bTDS[7];
GravityTDS gravityTds;
float tdsValue = 0, value = 0;
double nilaiKalibrasiTDS[2] = { 0, 0 };
int relay = 2;    //nutrisi A
int relay2 = 3;   //nutrisi B
int relay3 = 12;  //pompa air
int relay4 = 13;  //motor
LiquidCrystal_I2C lcd(0x27, 20, 4);
const byte BARIS = 4;
const byte KOLOM = 4;
char buff[16];
DS3231 rtc(SDA, SCL);  //real time clock connect
byte seconds, minutes, hours, day, date, month, year;
char tombol[BARIS][KOLOM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pinBaris[BARIS] = { 11, 10, 9, 8 };
byte pinKolom[KOLOM] = { 7, 6, 5, 4 };
int cursorColom = 0;
Keypad customKeypad = Keypad(makeKeymap(tombol), pinBaris, pinKolom, BARIS, KOLOM);
//rtc
int p = 0;
int A = 0;
char data[16];
char weekDay[4];
byte tMSB, tLSB;

char my_array[100];
char customKey;
int jam;
int menit;
int tahun;
int bulan;
int tanggal;
int menu = 0;
int secupg;
bool blinking = true;

Time time;
Time currentTime;
LinearRegression lr;

bool jalankanAlat() {
  lcd.clear();
  long timeAcuan;
  float bTDS;
  float mTDS;
  EEPROM.get(0, mTDS);
  EEPROM.get(5, bTDS);
  EEPROM.get(50, timeAcuan);
  rtc.begin();  //begin real time clock
  lcd.setCursor(0, 0);
  lcd.print(rtc.getDateStr());  // Print date to LCD
  lcd.setCursor(0, 1);
  lcd.print(rtc.getDOWStr());
  lcd.setCursor(0, 2);
  lcd.print("TDS: ");
  lcd.setCursor(4, 2);
  lcd.print(tdsValue, 0);
  delay(100);
  lcd.setCursor(8, 2);
  lcd.print("PPM");
  float dosis;

  while (1) {
    currentTime = rtc.getTime();
    long timeNumber = rtc.getUnixTime(currentTime);

    long lama = timeNumber - timeAcuan;
    tdsValue = gravityTds.getTdsValue();
    float nilaiTDS = mTDS * tdsValue + bTDS;
    if (lama < 86400000) {
      EEPROM.get(20, dosis);
    } else if (lama > 86400000 && lama < 1209600000) {
      EEPROM.get(25, dosis);
    } else if (lama > 1209600000 && lama < 1814400000) {
      EEPROM.get(30, dosis);
    } else if (lama > 1814400000 && lama < 2419200000) {
      EEPROM.get(35, dosis);
    } else if (lama > 2419200000 && lama < 3024000000) {
      EEPROM.get(40, dosis);
    } else if (lama > 3024000000 && lama < 3628800000) {
      EEPROM.get(45, dosis);
    }
    if (nilaiTDS < dosis) {
      delay(3000);
      digitalWrite(relay3, HIGH);
      digitalWrite(relay, LOW);
      digitalWrite(relay2, LOW);
      delay(300);
      digitalWrite(relay4, LOW);
      delay(5000);
    } else {
      delay(3000);
      digitalWrite(relay3, LOW);
      digitalWrite(relay, HIGH);
      digitalWrite(relay2, HIGH);
      delay(300);
      digitalWrite(relay4, HIGH);
      delay(5000);
    }
  }
}

bool kalibrasiTds() {
  lcd.clear();
  for (int i = 0; i < 6; i++) {
    lcd.setCursor(0, 0);
    lcd.print("nilai : ");
    lcd.print(i + 1);
    float value = getFloatFromKeypad(buff);
    tdsValue = gravityTds.getTdsValue();  // then get the value
    lr.learn(value, tdsValue);

    delay(500);
  }

  lr.getValues(nilaiKalibrasiTDS);
  float mTDS = (float)nilaiKalibrasiTDS[0];
  float bTDS = (float)nilaiKalibrasiTDS[1];
  EEPROM.put(0, mTDS);
  EEPROM.put(5, bTDS);
  return true;
}
bool pengaturanDosis() {
  rtc.begin();  //begin real time clock
  time = rtc.getTime();
  long currentTime = rtc.getUnixTime(time);

  lcd.clear();
  for (int i = 0; i < 6; i++) {
    lcd.setCursor(0, 0);
    lcd.print("Dosis mg ke: ");
    lcd.print(i + 1);
    float value = getFloatFromKeypad(buff);
    int j = i * 5 + 20;
    EEPROM.put(j, value);
  }
  return true;
}
bool kalibrasipH() {
  lcd.clear();
  for (int i = 0; i < 6; i++) {
    lcd.setCursor(0, 0);
    lcd.print("nilai:ke ");
    lcd.print(i + 1);
    float value = getFloatFromKeypad(buff);
    phValue = analogRead(A0);  // then get the value
    lr.learn(valueP, ph_act);
    lcd.setCursor(0, 2);
    lcd.print(ph_act);
    delay(500);
  }
  lr.getValues(nilaiKalibrasiph);
  float mph = (float)nilaiKalibrasiph[0];
  float bph = (float)nilaiKalibrasiph[1];
  EEPROM.put(10, mph);
  EEPROM.put(15, bph);
  return true;
}
char getCharFromKeypad() {
  while (1) {
    char keypressed = customKeypad.getKey();
    if (keypressed != NO_KEY) {
      return keypressed;
    }
  }
}
float getFloatFromKeypad(char* arr) {
  while (1) {
    char keypressed = customKeypad.getKey();
    if (keypressed != NO_KEY) {
      if (keypressed == 'D' || cursorColom > 3) {
        cursorColom = 0;
        lcd.clear();
        return atof(arr);

      } else {
        lcd.setCursor(cursorColom, 1);
        lcd.print(keypressed);
        arr[cursorColom] = keypressed;
        cursorColom++;
      }
    }
  }
}
void setup() {
  //lcd.begin(4, 20);
  // lcd.backlight();
  // lcd.setCursor(0, 0);
  //lcd.print("input nutrisi:");
  lcd.begin(4, 20);
  lcd.backlight();
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);       //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();            //initialization
  pinMode(TdsSensorPin, INPUT);
  lr = LinearRegression();
}
void pH() {
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(35);
    delay(30);
  }
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  avgval = 0;
  for (int i = 2; i < 8; i++)
    avgval += buffer_arr[i];
  float volt = (float)avgval * 3 / 4096.0 / 6;
  //Serial.print("Voltage: ");
  //Serial.println(volt);
  ph_act = -5.70 * volt + calibration_value;

  lcd.print("pH Val: ");
  lcd.println(ph_act);
  delay(1000);
}
void loop() {
  //float tds = analogRead();
  lcd.setCursor(0, 0);
  lcd.print("1. jalankan alat");
  lcd.setCursor(0, 1);
  lcd.print("2. kalibrasi TDS");
  lcd.setCursor(0, 2);
  lcd.print("3. dosis");
  lcd.setCursor(0, 3);
  lcd.print("4. kalibrasi pH");
  char menuTerpilih = getCharFromKeypad();
  switch (menuTerpilih) {
    case '1':
      jalankanAlat();
      break;
    case '2':
      kalibrasiTds();
      break;
    case '3':
      pengaturanDosis();
      break;
    case '4':
      kalibrasipH();
      break;
  }
}