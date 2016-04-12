#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <Rotary.h>

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
Rotary rotation = Rotary(24, 25);

boolean btnPressed;
boolean btnPressedLast;
const int pinSw = 22;
double multiplier = 1000.00;
float qnh = 1013.25;
int altitude;
int altitudeLast;
long duration = 0;

void setup() {
  pinMode(pinSw, INPUT);
  Serial.begin(9600);

  if (!bmp.begin()) {
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
} 

void loop() {
  // UI Events
  handleRotation();
  handleBtnPress();

  // Sensor events limited to 1/sec to give UI focus
  if (millis() >= duration + 1000) {
    duration = millis();
    setBmp();
  }
} 

void setBmp() {
  sensors_event_t event;
  bmp.getEvent(&event);

  if (event.pressure) {
    altitude = round(round(
        bmp.pressureToAltitude(
        qnh,
        event.pressure,
        event.temperature) * 3.2808399) / 10) * 10;
    if (altitude != altitudeLast) {
      Serial.print("Altitude: ");
      Serial.print(altitude);
      Serial.println("ft");
      altitudeLast = altitude;
    }
  }
}

void handleRotation() {
  unsigned char result = rotation.process();
  if (result) {
    if (result == DIR_CW) {
      qnh = qnh + multiplier;
      Serial.println(qnh);
    } else {
      qnh = qnh - multiplier;
      Serial.println(qnh);
    }
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


