#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "time.h"

HardwareSerial GPSSerial(1);

// The TinyGPS++ object
TinyGPSPlus gps;

// Setup Oled
#define i2c_Address 0x3C // I2C address of the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Setup WiFi and Firebase

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "hp1234"
#define WIFI_PASSWORD "12345678"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBccB0VHE3GATtMsU2aC7JZN03BHDfCX44"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "user123@gmail.com"
#define USER_PASSWORD "123456"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://kmipn-ayok-aja-gua-mah-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String latPath = "/latitude";
String lngPath = "/longitude";
String timePath = "/timestamp";

String latitudePath;
String longitudePath;

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

float lat;
float lng;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 5000;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 17);
    display.print("Connecting to WiFi");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
  display.clearDisplay();
  display.setCursor(10, 0);
  display.print("Connected");
  display.setTextSize(1);
  display.setCursor(25, 32);
  display.println(WiFi.localIP());
  display.display();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(9600);
  GPSSerial.begin(9600, SERIAL_8N1, 16, 17); // The serial connection to the GPS device
  Serial.print("GAMe : GPS Angkot Manajemen");
  // Begin Oled
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(30, 24);
  display.println("GAMe");
  display.display();

  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

// Write float values to the database
void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void dataAngkot(){
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
}

void loop(){
  // dataAngkot();

  // This sketch displays information every time a new sentence is correctly encoded.
  while (GPSSerial.available() > 0){
    gps.encode(GPSSerial.read());
    if (gps.location.isUpdated()){
      lat = gps.location.lat(), 6;
      lng = gps.location.lng(), 6;
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print("Longitude= "); 
      Serial.println(gps.location.lng(), 6);
    }
  }

  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    
    //Send Static Var
    latitudePath = "/UsersData/" + uid + "/latitude";
    longitudePath = "/UsersData/" + uid + "/longitude";

    sendFloat(latitudePath, lat);
    sendFloat(longitudePath, lng);

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    json.set(latPath.c_str(), String(lat, 6));
    json.set(lngPath.c_str(), String(lng, 6));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}

