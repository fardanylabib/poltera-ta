
#include "GravityTDS.h"

#define TdsSensorPin A1
GravityTDS gravityTds;

float temperature = 25, tdsValue = 0;
int relay = 2;    //nutrisi A
int relay2 = 3;   //nutrisi B
int relay3 = 12;  //pompa air
int relay4 = 13;  //motor
void setup() {
  Serial.begin(115200);
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  pinMode(relay, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();  //sample and calculate
  tdsValue = gravityTds.getTdsValue();  // then get the value
  if (tdsValue >= 500 ) {
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);
    digitalWrite(relay, HIGH);
    digitalWrite(relay2, HIGH);
    Serial.print(tdsValue, 0);
    Serial.println("PPM");
    Serial.print("Pemberian Nutrisi");
    delay(800);
    digitalWrite(relay4, HIGH);
    digitalWrite(relay, LOW);
    digitalWrite(relay2, LOW);
    Serial.print("Proses Pengadukan");
    delay(2000);
  }
  else {
    Serial.print("Pompa Aktif");
    digitalWrite(relay3, HIGH);
    digitalWrite(relay, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay4, LOW);
  }
}
