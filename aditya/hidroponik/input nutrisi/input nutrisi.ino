#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <DS3231.h>
#include <LinearRegression.h>
#include "GravityTDS.h"
#define IRQ (2)
#define RESET (3)
#define TdsSensorPin A1
//float calibration_value = 20.24 - 0.7;  //21.34 - 0.7
int pHSense = A0;
int samples = 10;
float adc_resolution = 1024.0;
double nilaiKalibrasiph[2] = { 0, 0 };
float nilai[6];
float nilaiTDS[6];
float mTDS[7];
float bTDS[7];
GravityTDS gravityTds;
float tdsValue = 0, value = 0;
float dosis;
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

byte pinBaris[BARIS] = { 11, 10, 9, 8 };
byte pinKolom[KOLOM] = { 7, 6, 5, 4 };
int cursorColom = 0;
Keypad customKeypad = Keypad(makeKeymap(tombol), pinBaris, pinKolom, BARIS, KOLOM);
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
int measurings = 0;
bool jalankanAlat() {
  lcd.clear();
  long timeAcuan;
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
      delay(100);
      return true;
    }
    currentTime = rtc.getTime();
    long timeNumber = rtc.getUnixTime(currentTime);
    long lama = timeNumber - timeAcuan;
    tdsValue = gravityTds.getTdsValue();
    float nilaiTDS = mTDS * tdsValue + bTDS;

    if (lama < 86400000) {
      EEPROM.get(20, dosis);
      Serial.println("masuk 1");
    } else if (lama > 86400000 && lama < 1209600000) {
      EEPROM.get(25, dosis);
      Serial.println("masuk 2");
    } else if (lama > 1209600000 && lama < 1814400000) {
      EEPROM.get(30, dosis);
      Serial.println("masuk 3");
    } else if (lama > 1814400000 && lama < 2419200000) {
      EEPROM.get(35, dosis);
      Serial.println("masuk 4");
    } else if (lama > 2419200000 && lama < 3024000000) {
      EEPROM.get(40, dosis);
      Serial.println("masuk 4");
    } else if (lama > 3024000000 && lama < 3628800000) {
      EEPROM.get(45, dosis);
      Serial.println("masuk 1");
    }
    int measurings = 0;

    for (int i = 0; i < samples; i++) {
      measurings += analogRead(pHSense);
      delay(10);
    }

    float voltage = 5 / adc_resolution * measurings / samples;
    gravityTds.update();                  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value
    lcd.setCursor(0, 0);
    lcd.print("PPM mg ini: ");
    lcd.print(dosis);
    lcd.setCursor(0, 1);
    lcd.print("PPM Sensor: ");
    lcd.print(nilaiTDS);
    lcd.setCursor(0, 2);
    lcd.print(ph(voltage));
    lcd.print("pH");
    delay(2000);
    if (nilaiTDS < dosis) {
      Serial.println("semprot");
      digitalWrite(relay3, LOW);
      digitalWrite(relay4, LOW);
      digitalWrite(relay, HIGH);
      digitalWrite(relay2, HIGH);
      delay(800);
      digitalWrite(relay4, HIGH);
      digitalWrite(relay, LOW);
      digitalWrite(relay2, LOW);
      delay(2000);
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
  for (int i = 0; i < 6; i++) {
    gravityTds.update();                  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value
    lcd.print(tdsValue, 0);
    lcd.print("ppm");
    delay(1000);
    lcd.setCursor(0, 3);
    lcd.print("nilai : ");
    lcd.print(i + 1);
    float value = getFloatFromKeypad(buff);
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
  // char keypressed = customKeypad.getKey();
  // if (keypressed == 'A') {
  //   Serial.println("di tekan A");
  //   delay(100);
  //   return true;
  currentTime = rtc.getTime();
  long timeNumber = rtc.getUnixTime(currentTime);
  EEPROM.put(50, timeNumber);
  delay(1000);
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

// char chekCharFromKeypad(){
// char keypressed = customKeypad.getKey();
// if (keypressed != NO_KEY){

// }}

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
  Serial.begin(115200);
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(3.7);       //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  pinMode(TdsSensorPin, INPUT);
  lr = LinearRegression();
}
float ph(float voltage) {
  return 7 + ((3.21 - voltage) / 0.18);
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
void DisplaySetHour() {
  int Credit, Num;
  // time setting
  p = 0;
  lcd_show(0, 0, "Set Hour:", 0, 1);
  jam = GetNum(2, 12, 2);
  if (jam < 0 or jam > 23) {
    menu = 1;
  }
  //Serial.println(jam);
  delay(100);
}

void DisplaySetMinute() {
  // Setting the minutes
  p = 0;
  lcd_show(0, 0, "Set Minute:", 0, 1);
  menit = GetNum(2, 12, 2);
  if (menit < 0 or menit > 59) {
    menu = 2;
  }
  //Serial.println(menit);
  delay(100);
}

void DisplaySetYear() {
  // setting the year
  p = 0;
  lcd_show(0, 0, "Set Year:", 0, 1);
  tahun = GetNum(4, 12, 2);
  if (tahun < 0 or tahun > 99) {
    menu = 5;
  }
  Serial.println(tahun);
  delay(100);
}

void DisplaySetMonth() {
  // Setting the month
  p = 0;
  lcd_show(0, 0, "Set Month:", 0, 1);
  bulan = GetNum(2, 12, 2);
  if (bulan < 0 or bulan > 12) {
    menu = 4;
  }
  //Serial.println(bulan);
  delay(100);
}

void DisplaySetDay() {
  // Setting the day
  p = 0;
  lcd_show(0, 0, "Set Day:", 0, 1);
  tanggal = GetNum(2, 12, 2);
  if (tanggal < 0 or tanggal > 31) {
    menu = 3;
  };
  //Serial.println(tanggal);
  delay(100);
}
void StoreAgg() {

  // Variable saving
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SAVING IN");
  lcd.setCursor(8, 1);
  lcd.print("PROGRESS");
  seconds = 0;
  day = 0;
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.write(decToBcd(seconds));
  Wire.write(decToBcd(menit));
  Wire.write(decToBcd(jam));
  Wire.write(decToBcd(day));
  Wire.write(decToBcd(tanggal));
  Wire.write(decToBcd(bulan));
  Wire.write(decToBcd(tahun));
  Wire.endTransmission();
  delay(1000);
  lcd.clear();
  delay(200);
}
byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}

void NumToChar(unsigned long Num, char* Buffer, unsigned char Digit) {
  char i;
  for (i = (Digit - 1); i >= 0; i--) {
    Buffer[i] = (Num % 10) + '0';
    Num = Num / 10;
  }
  for (i = 0; i < Digit; i++) {
    if (Buffer[i] == '0') {
      Buffer[i] = ' ';
    } else {
      i = 100;
    }
  }
}
char BufNum[8];
void SlideNum(void) {
  BufNum[6] = BufNum[5];
  BufNum[5] = BufNum[4];
  BufNum[4] = BufNum[3];
  BufNum[3] = BufNum[2];
  BufNum[2] = BufNum[1];
  BufNum[1] = BufNum[0];
}
unsigned long GetNum(int Count, int X, int Y) {
  char Key, i, N;
  int Sum;
  lcd.blink();
  lcd.setCursor(X + (Count - 2), Y - 1);
  N = 0;
  for (i = 0; i < Count; i++) {
    BufNum[i] = ' ';
  }
  i = 0;
  Key = customKeypad.getKey();
  while (p != 1) {

    Key = NO_KEY;
    while (Key == NO_KEY) {
      Key = customKeypad.getKey();
    }

    if (Key == 'D') {
      menu = menu + 1;
      p = 1;
    }
    if ((N < Count) && (Key >= '0') && (Key <= '9')) {
      SlideNum();
      BufNum[0] = Key;
      N++;
    }
    lcd.setCursor(X - 1, Y - 1);
    for (i = 0; i < Count; i++) {
      lcd.print(BufNum[Count - (i + 1)]);
    }
    lcd.setCursor(X + (Count - 2), Y - 1);
  }
  Sum = 0;
  if (Count != 4) {
    for (i = 0; i < Count; i++) {
      if (BufNum[Count - (i + 1)] == 0x20) {
        BufNum[Count - (i + 1)] = '0';
      }
      Sum = (Sum * 10) + (BufNum[Count - (i + 1)] - '0');
    }
  }
  if (Count == 4) {
    Count = 2;

    for (i = 0; i < Count; i++) {
      if (BufNum[Count - (i + 1)] == 0x20) {
        BufNum[Count - (i + 1)] = '0';
      }
      Sum = (Sum * 10) + (BufNum[Count - (i + 1)] - '0');
    }
  }
  //Serial.println(hours);
  lcd.noCursor();
  lcd.noBlink();

  return (Sum);
}
void lcd_show(int b, int c, char* txt0, int d, int e) {
  lcd.clear();
  lcd.setCursor(b, c);
  lcd.print(txt0);
  lcd.setCursor(d, e);
}