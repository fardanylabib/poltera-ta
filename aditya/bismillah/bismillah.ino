#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <DS3231.h>
#include <LinearRegression.h>
#include "GravityTDS.h"
#define TdsSensorPin A1
int pHSense = A0;
int samples = 10;
float adc_resolution = 1024.0;
float nilaiTDS = 0;
float mTDS[7];
float bTDS[7];
GravityTDS gravityTds;
float tdsValue = 0;
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
#define DS3231_I2C_ADDRESS 104
byte seconds, minutes, hours, day, date, month, year;
char tombol[BARIS][KOLOM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '.', '0', '#', 'D' }
};
float ph(float voltage) {
  return 7 + ((4.5 - voltage) / 0.18);
}
byte pinBaris[BARIS] = { 11, 10, 9, 8 };
byte pinKolom[KOLOM] = { 7, 6, 5, 4 };
int cursorColom = 0;
Keypad customKeypad = Keypad(makeKeymap(tombol), pinBaris, pinKolom, BARIS, KOLOM);
Time time;
Time currentTime;
LinearRegression lr;
int measurings = 0;
long timeAcuan;
bool jalankanAlat() {
  lcd.clear();
  // long timeAcuan;
  float bTDS;
  float mTDS;
  EEPROM.get(0, mTDS);
  EEPROM.get(5, bTDS);
  EEPROM.get(50, timeAcuan);
  rtc.begin();  //begin real time clock
  float dosis;

  while (1) {
    char keypressed = customKeypad.getKey();
    if (keypressed == 'A') {
      Serial.println("di tekan A");
      delay(1000);
      digitalWrite(relay3, LOW);
      digitalWrite(relay4, LOW);
      digitalWrite(relay, LOW);
      digitalWrite(relay2, LOW);
      return true;
    }
    currentTime = rtc.getTime();
    long timeNumber = rtc.getUnixTime(currentTime);
    long lama = timeNumber - timeAcuan;
    tdsValue = gravityTds.getTdsValue();
    float nilaiTDS = ((tdsValue - bTDS) / mTDS);
    lcd.setCursor(0, 0);
    lcd.print("PPM mg ini: ");
    lcd.print(dosis);
    lcd.setCursor(0, 1);
    lcd.print("PPM Sensor: ");
    if (nilaiTDS < 0) {
      nilaiTDS = 0;
    }
    lcd.print(nilaiTDS);
    delay(2000);
    int measurings = 0;
    for (int i = 0; i < samples; i++) {
      measurings += analogRead(pHSense);
      delay(10);
    }
    float voltage = 5 / adc_resolution * measurings / samples;
    lcd.setCursor(0, 2);
    lcd.print("pH= ");
    lcd.print(ph(voltage));
    delay(2000);

    if (lama < 604800) {
      EEPROM.get(20, dosis);
      Serial.println("masuk 1");
    } else if (lama > 604800 && lama < 1209600) {
      EEPROM.get(25, dosis);
      Serial.println("masuk 2");
    } else if (lama > 1209600 && lama < 1814400) {
      EEPROM.get(30, dosis);
      Serial.println("masuk 3");
    } else if (lama > 1814400 && lama < 2419200) {
      EEPROM.get(35, dosis);
      Serial.println("masuk 4");
    } else if (lama > 2419200 && lama < 3024000) {
      EEPROM.get(40, dosis);
      Serial.println("masuk 4");
    } else if (lama > 3024000 && lama < 3628800) {
      EEPROM.get(45, dosis);
      Serial.println("masuk 1");
    }
    gravityTds.update();                  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value

    if (nilaiTDS <= dosis) {
      Serial.println("semprot");
      digitalWrite(relay3, LOW);
      digitalWrite(relay4, LOW);
      digitalWrite(relay, HIGH);
      digitalWrite(relay2, LOW);
      delay(150);
      digitalWrite(relay4, HIGH);
      digitalWrite(relay, LOW);
      delay(5000);
      digitalWrite(relay4, LOW);
      digitalWrite(relay2, HIGH);
      delay(150);
      digitalWrite(relay4, HIGH);
      digitalWrite(relay2, LOW);
      delay(5000);
    } else {
      Serial.println("pompa aktif");
      digitalWrite(relay3, HIGH);
      digitalWrite(relay, LOW);
      digitalWrite(relay2, LOW);
      digitalWrite(relay4, LOW);
    }
  }
}
bool kalibrasiTds() {
  lcd.clear();
  for (int i = 0; i < 7; i++) {
    digitalWrite(relay, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay4, LOW);
    delay(150);
    digitalWrite(relay, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay4, HIGH);
    delay(5000);
    digitalWrite(relay4, LOW);
    gravityTds.update();                  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value
    lcd.setCursor(0, 3);
    lcd.print(tdsValue, 0);
    lcd.print("ppm");
    Serial.println(tdsValue, 0);
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("nilai : ");
    lcd.print(i + 1);
    float value = getFloatFromKeypad(buff);
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
  lcd.clear();
  for (int i = 0; i < 6; i++) {
    lcd.setCursor(0, 0);
    lcd.print("Dosis mg ke: ");
    lcd.print(i + 1);
    float dosis = getFloatFromKeypad(buff);
    int j = i * 5 + 20;
    EEPROM.put(j, dosis);
  }
  return true;
}
bool aturWaktu() {
  lcd.clear();
  lcd.begin(4, 20);  // initialize the lcd
  lcd.backlight();
  currentTime = rtc.getTime();
  long timeNumber = rtc.getUnixTime(currentTime);
  EEPROM.put(50, timeNumber);
  Serial.println("p");
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
  lcd.begin(4, 20);
  lcd.backlight();
  Serial.begin(115200);
  rtc.begin();
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(4.41);      //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  pinMode(TdsSensorPin, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  lr = LinearRegression();
}

void loop() {
  //float tds = analogRead();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1. jalankan alat");
  lcd.setCursor(0, 1);
  lcd.print("2. kalibrasi TDS");
  lcd.setCursor(0, 2);
  lcd.print("3. dosis");
  lcd.setCursor(0, 3);
  lcd.print("4. Atur Waktu");
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
      aturWaktu();
      break;
  }
}