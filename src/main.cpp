#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

HardwareSerial GPSSerial(1);

// The TinyGPS++ object
TinyGPSPlus gps;

void setup(){
  Serial.begin(9600);
  GPSSerial.begin(9600, SERIAL_8N1, 16, 17); // The serial connection to the GPS device
  Serial.print("tes");
}

void loop(){
  // This sketch displays information every time a new sentence is correctly encoded.
  while (GPSSerial.available() > 0){
    gps.encode(GPSSerial.read());
    if (gps.location.isUpdated()){
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
    }
    else {
      Serial.println("INVALID");
    }
    delay(500);
  }
}