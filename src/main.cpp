#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

HardwareSerial GPSSerial(1);

// The TinyGPS++ object
TinyGPSPlus gps;

// Setup Oled
#define i2c_Address 0x3C // I2C address of the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup(){
  Serial.begin(9600);
  GPSSerial.begin(9600, SERIAL_8N1, 16, 17); // The serial connection to the GPS device
  Serial.print("GAMe : GPS Angkot Manajemen");
  // Begin Oled
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("GAMe");
  display.display();
}

void loop(){
  // Show Data To Oled
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(40, 0);
  display.print("GAMe");
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print("Jenis : Suzuki Carry");
  display.setCursor(0, 40);
  display.print("Plat  : BA 2592 MY");
  display.setCursor(0, 55);
  display.print("Warna : Biru");
  display.display();

  // This sketch displays information every time a new sentence is correctly encoded.
  while (GPSSerial.available() > 0){
    gps.encode(GPSSerial.read());
    if (gps.location.isUpdated()){
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
    }
  }
}