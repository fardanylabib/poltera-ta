#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <DS3231.h>
#include <LinearRegression.h>
int ph = analogRead(A0);
int tds = analogRead(A1);
int samples = 10;
float nilaiTDS = 0;
float mTDSVal[20];
float bTDSVal[20];
float mPHVal[3];
float bPHVal[3];
double nilaiKalibrasiTDS[2] = { 0, 0 };
double nilaiKalibrasipH[2] = { 0, 0 };
int relay = 2;    //nutrisi A
int relay2 = 3;   //nutrisi B
int relay3 = 12;  //pompa air
int relay4 = 13;  //motor
LiquidCrystal_I2C lcd(0x27, 20, 4);
const byte BARIS = 4;
const byte KOLOM = 4;
char buff[16];
DS3231 rtc(SDA, SCL);  //real time clock connect
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
Time time;
Time currentTime;
LinearRegression lr;
int measurings = 0;
long timeAcuan;
bool jalankanAlat() {
  lcd.clear();
  float bTDSVal;  //=> GANTI nama variabel jadi bTDSVal
  float mTDSVal;  //=> GANTI nama variabel jadi mTDSVal
  float bPHVal;
  float mPHVal;
  float eror;
  EEPROM.get(0, mTDSVal);  //=> mTDSVal
  EEPROM.get(5, bTDSVal);  //=> bTDSVal
  Serial.println(mTDSVal);
  Serial.println(bTDSVal);
  //=> Panggil EEPROM.get() untuk bPHVal dan mPHVal juga
  EEPROM.get(10, mPHVal);
  EEPROM.get(15, bPHVal);
  Serial.println(mPHVal);
  Serial.println(bPHVal);
  EEPROM.get(50, timeAcuan);
  EEPROM.get(55, eror);
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
    int tds = analogRead(A1);
    int nilaiTDS = (tds - bTDSVal) / mTDSVal;  //=> bTDSVal & mTDSVal
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
    float nilaiPH = (ph - bPHVal) / mPHVal;
    lcd.setCursor(0, 2);
    lcd.print("pH : ");
    lcd.print(nilaiPH);
    delay(2000);
    lcd.setCursor(0, 3);
    lcd.print("eror TDS: ");
    lcd.print(eror);
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
      Serial.println("masuk 5");
    } else if (lama > 3024000 && lama < 3628800) {
      EEPROM.get(45, dosis);
      Serial.println("masuk 6");
    }


    if (nilaiTDS <= dosis - 20) {
      Serial.println("semprot");
      digitalWrite(relay3, LOW);
      digitalWrite(relay4, LOW);
      digitalWrite(relay, HIGH);
      digitalWrite(relay2, LOW);
      delay(100);
      digitalWrite(relay4, HIGH);
      digitalWrite(relay, LOW);
      delay(5000);
      digitalWrite(relay4, LOW);
      digitalWrite(relay2, HIGH);
      delay(100);
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
  float sigma = 0;
  for (int i = 0; i < 20; i++) {
    int tds = analogRead(A1);
    lcd.setCursor(0, 3);
    delay(3000);
    lcd.print(tds);
    lcd.print("ppm");
    Serial.print(tds);
    lcd.setCursor(0, 0);
    lcd.print("nilai : ");
    lcd.print(i + 1);
    float value = getFloatFromKeypad(buff);
    lr.learn(value, tds);
    float erorI = value - tds;
    erorI = erorI * erorI;
    sigma = sigma + erorI;
    delay(500);
  }

  lr.getValues(nilaiKalibrasiTDS);
  float mTDSVal = (float)nilaiKalibrasiTDS[0];
  float bTDSVal = (float)nilaiKalibrasiTDS[1];

  Serial.println(mTDSVal);
  Serial.println(bTDSVal);
  EEPROM.put(0, mTDSVal);
  EEPROM.put(5, bTDSVal);
  return true;
}
bool kalibrasipH() {
  lcd.clear();
  for (int i = 0; i < 3; i++) {
    int ph = analogRead(A0);
    lcd.setCursor(0, 3);
    lcd.print("pH : ");
    lcd.print(ph);
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("nilai : ");
    lcd.print(i + 1);
    float valuepH = getFloatFromKeypad(buff);
    lr.learn(valuepH, ph);
  }
  lr.getValues(nilaiKalibrasipH);
  float mPHVal = (float)nilaiKalibrasipH[0];
  float bPHVal = (float)nilaiKalibrasipH[1];
  Serial.print(mPHVal);
  Serial.print(bPHVal);
  EEPROM.put(10, mPHVal);
  EEPROM.put(15, bPHVal);
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

bool resetWaktu() {
  lcd.clear();
  lcd.begin(4, 20);  // initialize the lcd
  lcd.backlight();
  currentTime = rtc.getTime();
  long timeNumber = rtc.getUnixTime(currentTime);
  EEPROM.put(50, timeNumber);
  Serial.println("p");
}
bool eror() {
  lcd.clear();
  float sigma = 0;
  for (int i = 0; i < 20; i++) {
    int tds = analogRead(A1);
    // int nilaiTDS = (tds - bTDSVal) / mTDSVal;  //=> bTDSVal & mTDSVal
    lcd.setCursor(0, 3);
    delay(3000);
    lcd.print(tds);
    lcd.print("ppm");
    Serial.print(nilaiTDS);
    lcd.setCursor(0, 0);
    lcd.print("nilai : ");
    lcd.print(i + 1);
    float nilai = getFloatFromKeypad(buff);
    float erorI = nilai - nilaiTDS;
    erorI = erorI * erorI;
    sigma = sigma + erorI;
    delay(500);
  }
  float sigmaPerN = sigma / 20;
  // sqrt(sigmaPerN);
  float eror = sqrt(sigmaPerN);
  EEPROM.put(55, eror);  
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
  pinMode(relay, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  lr = LinearRegression();
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1. jalankan Alat");
  lcd.setCursor(0, 1);
  lcd.print("2. Klbrs TDS / 3. pH");  //=> 2. Klbrs TDS / 3. pH  => GINI SAJA
  lcd.setCursor(0, 2);
  lcd.print("4. Atur Dosis");
  lcd.setCursor(0, 3);
  lcd.print("5. Reset Waktu");
  char menuTerpilih = getCharFromKeypad();
  switch (menuTerpilih) {
    case '1':
      jalankanAlat();
      break;
    case '2':
      kalibrasiTds();
      break;
    case '3':
      kalibrasipH();
      break;
    case '4':
      pengaturanDosis();
      break;
    case '5':
      resetWaktu();
      break;
    case '6':
      eror();
      break;
  }
}