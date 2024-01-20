#include <DHT.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h> 
#include <virtuabotixRTC.h>

//RTC DS1302 
//VCC 5V
//GND
#define RTC_CLK 6
#define RTC_DAT 7
#define RTC_RST 8
virtuabotixRTC myRTC(RTC_CLK, RTC_DAT, RTC_RST);

//72XX Panel
//VCC 5V
//GND 
#define MTX_CIS 10
#define MTX_DIN 11
#define MTX_CLK 13
 
Max72xxPanel matrix = Max72xxPanel(MTX_CIS, 8, 1);
 
//DHT11
//VCC 5V
//GND 
#define DHTPIN 2
#define DHTTYPE DHT11
 
DHT dht(DHTPIN, DHTTYPE);

//BUTTONS
#define BTNYEL 5
#define BTNRED 3

//---------------------------------
const int wait = 50;
const int spacer = 1;
const int width = 5 + spacer;

int counter;
int counterLast;
int counterRed;
int counterYel;
const unsigned long period = 1000; // only refresh screen every sec
unsigned long previousMillis = 0;

char time[12]; 
char date[12]; 
char temp[20];
char humi[20]; 

int buttonYellow = 0;
int buttonRed = 0;

bool programmingMode = false;
int programmingStep = 0;
const int MAX_PROG_STEPS = 6;
//---------------------------------

void setup(){ 
  //brightness
  matrix.setIntensity(0);

  //position and order
  matrix.setPosition(0, 0, 0);
  matrix.setPosition(1, 1, 0);
  matrix.setPosition(2, 2, 0);
  matrix.setPosition(3, 3, 0);
  matrix.setPosition(4, 4, 0);
  matrix.setPosition(5, 5, 0);
  matrix.setPosition(6, 6, 0);
  matrix.setPosition(7, 7, 0);
  matrix.setPosition(8, 8, 0);

  //rotation 90deg clockwise
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
  matrix.setRotation(4, 1);
  matrix.setRotation(5, 1);
  matrix.setRotation(6, 1);
  matrix.setRotation(7, 1);
  matrix.setRotation(8, 1);

  //turn off matrix
  matrix.fillScreen(LOW);

  //buttons
  pinMode(BTNYEL, INPUT);
  pinMode(BTNRED, INPUT); 

  //set time first time
  //myRTC.setDS1302Time(0, 47, 15, 7, 17, 12, 2023);

  //init temp and humidity sensor
  dht.begin();    

  //init counter
  counter = 0;
}


void loop() { 
  unsigned long currentMillis = millis();
 
  int y = digitalRead(BTNYEL);
  int r = digitalRead(BTNRED);  

  if (currentMillis - previousMillis >= period)
  {
    previousMillis = currentMillis;

    if (r == HIGH) {
      counterRed++;
    }  
    if (y == HIGH) {
      counterYel++;
      if (programmingMode) {
        programmingStep++;
        if (programmingStep > MAX_PROG_STEPS) {
          programmingStep = 0;
        }
      }
    }  

    // refresh time from RTC
    myRTC.updateTime();
  
    sprintf(time,"%02d:%02d", myRTC.hours, myRTC.minutes); 
    sprintf(date,"%02d.%02d", myRTC.dayofmonth, myRTC.month); 
    sprintf(temp,"%4d", (int)dht.readTemperature());  
    sprintf(humi,"%4d", (int)dht.readHumidity());  
  
    if (programmingMode) {      
      // during programming mode, we flash the display
      counter++;
      matrix_print("");

      if (counter % 2) {
        switch (programmingStep) {
          case 1:
            programStep("Ho", myRTC.hours, 0, 23);
            break;
          case 2:
            programStep("Mi", myRTC.minutes, 0, 59);
            break;
          case 3:
            programStep("Da", myRTC.dayofmonth, 1, 31);
            break;
          case 4:
            programStep("Mo", myRTC.month, 1, 12);
            break;
          case 5:
            programStep("Dy", myRTC.dayofweek, 1, 7); 
            break;
          case 6:
            programStep("Yr", myRTC.dayofweek, 2000, 2999);
            break;
        }
        // matrix.drawLine(0, 7, 32, 7, HIGH);
      }
    } 
    else 
    {
      switch (counter) {
        case 10 ...16:
          matrix_print(date, true); 
          break;
        case 20 ...22:
          matrix_print((String)"T" + temp);
          break;
        case 40 ...42:
          matrix_print((String)"H" + humi);
          break;
        default:
          matrix_print(time, counter % 2);
      } 
    
      if (counter >= 59) {
        counter = 0;
      } else {
        counter++;
      } 
    }

    if (counterRed >= 3) {
      programmingMode = !programmingMode;
      if (programmingMode) {
        counter = counterLast;
        programmingStep = 1;
      } else {
        counterLast = counter;
        counter = 0;
        programmingStep = 0;
      }
      counterRed = 0;
    }

    matrix.write();
  }
} 

void programStep(String letter, int value, int min, int max) {
  char temp[12]; 
  int y = digitalRead(BTNYEL);

  if (y == HIGH) {
    value++;
    if (value > max) {
      value = min;
    }
    
    switch (programmingStep) {
      case 1:
        myRTC.setDS1302Time(myRTC.seconds, myRTC.minutes, value, myRTC.dayofweek, myRTC.dayofmonth, myRTC.month, myRTC.year);
        break;
      case 2:
        myRTC.setDS1302Time(myRTC.seconds, value, myRTC.hours, myRTC.dayofweek, myRTC.dayofmonth, myRTC.month, myRTC.year);
        break;
      case 3:
        myRTC.setDS1302Time(myRTC.seconds, myRTC.minutes, myRTC.hours, myRTC.dayofweek, value, myRTC.month, myRTC.year);
        break;
      case 4:
        myRTC.setDS1302Time(myRTC.seconds, myRTC.minutes, myRTC.hours, myRTC.dayofweek, myRTC.dayofmonth, value, myRTC.year);
        break;
      case 5:
        myRTC.setDS1302Time(myRTC.seconds, myRTC.minutes, myRTC.hours, myRTC.dayofweek, myRTC.dayofmonth, myRTC.month, value);
        break;
      case 6:
        myRTC.setDS1302Time(myRTC.seconds, myRTC.minutes, myRTC.hours, value, myRTC.dayofmonth, myRTC.month, myRTC.year);
        break;
    }
  }

  myRTC.updateTime();
  
  sprintf(temp, "%02d", value); 
  matrix_print((String)letter + " " + temp);  
} 

void matrix_print(String message) { 
  matrix.fillScreen(LOW);

  int c[5] = {2, 8, 14, 20, 26};

  for ( int i = 0 ; i < message.length(); i++ ) {
    matrix.drawChar(c[i], 0, message[i], HIGH, LOW, 1);
  }
}

void matrix_print(char m[], bool f) { 
  matrix.fillScreen(LOW);
  
  matrix.drawChar(2,0, m[0], HIGH,LOW,1);
  matrix.drawChar(8,0, m[1], HIGH,LOW,1);
  if (f) {
    matrix.drawChar(14,0, m[2], HIGH,LOW,1);
  }
  matrix.drawChar(20,0, m[3], HIGH,LOW,1);
  matrix.drawChar(26,0, m[4], HIGH,LOW,1);
}

void display_message(String message){
   matrix.fillScreen(LOW); 
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2;
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= width;
    }
    matrix.write();
    delay(wait/2);
  }   
}
