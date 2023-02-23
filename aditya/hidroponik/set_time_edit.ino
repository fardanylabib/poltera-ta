#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h> //for keypad
#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

DS3231  rtc(SDA, SCL); //real time clock connect
#define DS3231_I2C_ADDRESS 104
byte seconds, minutes, hours, day, date, month, year;

LiquidCrystal_I2C lcd(0x27,16,2);

const byte ROWS = 4; //four rows for keypad
const byte COLS = 4; //four columns for keypad
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {11, 10, 9, 8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad  buttonKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

int p = 0;
int A = 0;
char  data[16];
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


Time  t;

void setup()
{
  Wire.begin();  //begin I2C
  lcd.begin(4,20);  // initialize the lcd 
  lcd.backlight();
  rtc.begin();  //begin real time clock
  Serial.begin(9600);
  t = rtc.getTime();

  
}
void loop()
{
  int  Credit,Num;
  //menu = 0;
  customKey =  buttonKeypad.getKey();  //read form keypad
 if (customKey == 'D'){

    menu=menu+1;
    if(menu==2){p=1;}
    if(menu==1){p=0;}
  }
  if (menu==0)
  {
      // Print a message to the LCD.
      lcd.setCursor(3,0);
      lcd.print(rtc.getDateStr());   // Print date to LCD
      lcd.setCursor(0,1);
      lcd.print(rtc.getTimeStr());   // Print time to LCD
  


   }
 if (menu==1)
    {
    DisplaySetHour();    // set hour
    }
  if (menu==2)
    {
    DisplaySetMinute();  // set minute
    }
  if (menu==3)
    {
    DisplaySetDay();      // set day
    }
  if (menu==4)
    {
    DisplaySetMonth();   // set Month
    }
  if (menu==5)
    { 
    DisplaySetYear();    // set year   

    
    //check error if  min,hour,day,month,year  = 0  print error
    if(menit <= 0 && jam <= 0 && tanggal <= 0 && bulan <= 0 && tahun <= 0)
    { 
      menu = 0;         
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   ERROR   ");
    }    
  }
  if (menu==6)
  {
    StoreAgg();      //setup time date to DS3231
    delay(500);
    menu=0;    // return to main menu
  }
  delay(100);
}
void DisplaySetHour()
{
  int  Credit,Num;
// time setting
  p = 0;
  lcd_show(0,0,"Set Hour:",0,1);  
  jam = GetNum(2,12,2);   
  if(jam < 0 or jam > 23){menu = 1;}
  //Serial.println(jam);
  delay(100);
}

void DisplaySetMinute()
{
// Setting the minutes
  p = 0;
  lcd_show(0,0,"Set Minute:",0,1);
  menit = GetNum(2,12,2);
  if(menit < 0 or menit > 59) {menu = 2;}
  //Serial.println(menit);
  delay(100);
}
  
void DisplaySetYear()
{
  // setting the year
  p = 0;
  lcd_show(0,0,"Set Year:",0,1);
  tahun = GetNum(4,12,2);
  if(tahun < 0 or tahun > 99){menu = 5;}
  Serial.println(tahun);
  delay(100);
}

void DisplaySetMonth()
{
  // Setting the month
  p = 0;
  lcd_show(0,0,"Set Month:",0,1);
  bulan = GetNum(2,12,2);
  if(bulan < 0 or bulan > 12) {menu = 4 ;}
  //Serial.println(bulan);
  delay(100);
}

void DisplaySetDay()
{
  // Setting the day
  p = 0;
  lcd_show(0,0,"Set Day:",0,1);
  tanggal = GetNum(2,12,2);
  if(tanggal < 0 or tanggal > 31) {
    menu = 3 ; 
  };
  //Serial.println(tanggal);
  delay(100);
}
void StoreAgg()
{

  // Variable saving
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SAVING IN");
  lcd.setCursor(8,1);
  lcd.print("PROGRESS");
  seconds = 0 ; 
  day     = 0 ;
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
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

void NumToChar(unsigned long Num, char  *Buffer,unsigned char Digit)
{char i;
   for(i=(Digit-1);i>= 0;i--)
    { Buffer[i] =  (Num % 10) + '0';
      Num = Num / 10;
    }  
   for(i=0;i<Digit;i++)
    { if(Buffer[i] == '0'){Buffer[i] =  ' ';}else{i =100;}
                         
    }   
}
char BufNum[8];
void  SlideNum(void)
{
  BufNum[6]  = BufNum[5];
  BufNum[5]  = BufNum[4];
  BufNum[4]  = BufNum[3];
  BufNum[3]  = BufNum[2];
  BufNum[2]  = BufNum[1];
  BufNum[1]  = BufNum[0];
}
unsigned long GetNum(int Count,int X,int Y)
{ char Key,i,N;
  int Sum; 
  lcd.blink();
  lcd.setCursor(X+(Count-2),Y-1); 
  N = 0;
  for(i=0;i<Count;i++){BufNum[i] = ' ';}
  i = 0;
  Key =  buttonKeypad.getKey();
  while(p != 1)
  { 
         
    Key = NO_KEY;
    while(Key == NO_KEY){Key =  buttonKeypad.getKey();}
    
   if(Key == 'D'){menu = menu+1; p = 1; }             
   if((N < Count)&&(Key >= '0')&&(Key <= '9'))
   { SlideNum();
     BufNum[0] = Key;
     N++;
   }
   lcd.setCursor(X-1,Y-1);
   for(i=0;i<Count;i++){lcd.print(BufNum[Count-(i+1)]);}
   lcd.setCursor(X+(Count-2),Y-1);  
  }
Sum = 0;
if (Count != 4)
{
for(i=0;i<Count;i++)
 {
   if(BufNum[Count-(i+1)]==0x20){BufNum[Count-(i+1)] = '0';}
   Sum = (Sum*10) + (BufNum[Count-(i+1)]-'0');
 }
}
 if (Count == 4)
 {
  Count = 2;
 
 for(i=0;i<Count;i++)
 {
   if(BufNum[Count-(i+1)]==0x20){BufNum[Count-(i+1)] = '0';}
   Sum = (Sum*10) + (BufNum[Count-(i+1)]-'0');
 }
 }
 //Serial.println(hours);
lcd.noCursor();
lcd.noBlink();

return(Sum);  
}

void lcd_show(int b,int c,char* txt0,int d,int e)
{
  lcd.clear();
  lcd.setCursor(b,c);
  lcd.print(txt0);
  lcd.setCursor(d,e);
}
