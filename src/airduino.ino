#include <Adafruit_BMP085_U.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DS3232RTC.h>
#include <LiquidCrystal.h>
#include <Rotary.h>
#include <SD.h>
#include <SPI.h>
#include <Time.h>
#include <Wire.h>

LiquidCrystal lcd(2, 3, 4, 5, 6 , 7);
DHT dht(15, DHT22);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
Rotary rotation = Rotary(9, 10);

File sd;
boolean btnPressed;
boolean btnPressedLast;
const int pinSdCs = 14;
const int pinSw = 8;
double multiplier = 1000.00;
float qnh = 1013.25;
float temperature = 0.00;
int altitude;
int altitudeLast;
long duration = 0;

void setup() {
  pinMode(pinSw, INPUT);

  lcd.begin(16, 2);
  lcd.setCursor(0,0);

  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet) {
    lcd.print("Can't sync RTC  ");
    while(1);
  }

  if (!bmp.begin()) {
    lcd.print("No BMP085 found ");
    while(1);
  }

  if (!SD.begin(pinSdCs)) {
    lcd.print("Can't find SD   ");
    while(1);
  }

  dht.begin();
}

void loop() {
  // UI Events
  handleRotation();
  handleBtnPress();

  // Sensor events limited to 1/sec to give UI focus
  if (millis() >= duration + 1000) {
    duration = millis();
    printTime();
    getBmp();
    // getDht();
    // logToCard();
  }
}

void getBmp() {
  bmp.getTemperature(&temperature);

  clearChars(9, 15, 0);
  lcd.setCursor(9,0);
  lcd.print(temperature, 1);
  lcd.print(" C");

  sensors_event_t event;
  bmp.getEvent(&event);

  if (event.pressure) {
    altitude = round(round(
        bmp.pressureToAltitude(
        qnh,
        event.pressure,
        event.temperature) * 3.2808399) / 10) * 10;
    if (altitude != altitudeLast) {
      clearChars(0, 15, 0);
      lcd.setCursor(0,0);
      lcd.print(altitude);
      lcd.print("ft");
      altitudeLast = altitude;
    }
  }
}

void getDht() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  clearChars(0, 15, 0);
  lcd.setCursor(0, 0);
  lcd.print(h);
  lcd.print(" ");
  lcd.print(t);
}

void handleRotation() {
  unsigned char result = rotation.process();
  if (result) {
    if (result == DIR_CW) {
      qnh = qnh + multiplier;
    } else {
      qnh = qnh - multiplier;
    }
    clearChars(9, 15, 1);
    lcd.setCursor(9, 1);
    lcd.print(qnh);
  }
}

void handleBtnPress() {
  btnPressed = digitalRead(pinSw);
  if (btnPressed != btnPressedLast) {
    if (!btnPressed) {
      multiplier = multiplier / 10;
      if (multiplier < 0.01) {
        multiplier = 1000;
      }
    }
    btnPressedLast = btnPressed;
  }
}

void printTime() {
  lcd.setCursor(0,1);
  lcd.print(hour());
  printDigits(minute());
  printDigits(second());
}

void printDigits(int digits) {
  lcd.print(':');
  if (digits < 10) {
    lcd.print('0');
  }
  lcd.print(digits);
}

void clearChars(short fromX, short toX, short row) {
  for (short i = fromX; i < toX; i++) {
    lcd.setCursor(i, row);
    lcd.print(' ');
  }
}

void logToCard() {
  sd = SD.open("logfile.txt", FILE_WRITE);
  sd.println(millis());
  sd.close();
}
