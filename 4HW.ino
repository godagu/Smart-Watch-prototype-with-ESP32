// Author: Goda Gutparakyte
// Assignment: Robotics HW4
// Name: Smart watch prototype
// Last edited: 2025 11 24

#include <WiFi.h>
#include <TFT_eSPI.h>
#include "time.h" 
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <HTTPClient.h>

// I2C pins (shared)
#define I2C_SDA 32
#define I2C_SCL 33
// Display screen pins defined in User_Setup
#define BUTTON_PIN 26

struct ThemePalette{
  uint16_t backgroundColor;
  uint16_t dateColor;
  uint16_t timeColor;
  uint16_t heartColor;
  uint16_t stepsColor;
  uint16_t infoColor;
};


ThemePalette themes[] = {
  { TFT_BLACK, TFT_WHITE, TFT_SILVER, TFT_RED, TFT_GREEN, TFT_CYAN },
  { TFT_WHITE, TFT_BLACK, TFT_DARKGREY, TFT_RED, TFT_DARKGREEN, TFT_BLUE },
  { 0xFC18, TFT_WHITE, TFT_WHITE, TFT_MAGENTA, TFT_PURPLE, TFT_WHITE } 
};

// BUTTON
volatile bool buttonPressedFlag = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 250;


// THEMES
int currentThemeIndex = 0;
const int totalThemes = 3;
bool themeChanged = true;


// OBJECT
MAX30105 particleSensor;
Adafruit_MPU6050 mpu;
TFT_eSPI tft = TFT_eSPI();

// WIFI
const char* ssid = "";
const char* password = "";

// TIME
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;       
const int   daylightOffset_sec = 3600;  

// LOCATION and WEATHER
float myLat = 0.0;
float myLon = 0.0;
String myCity = "Locating...";
float outdoorTemp = 0.0;
unsigned long lastWeatherTime = 0;
bool locationFound = false;

// HEART SENSOR
const byte RATE_SIZE = 12;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg = 0;

// STEP
int stepCount = 0;
unsigned long lastStepTime = 0;
unsigned long lastAccelRead = 0;
const float STEP_THRESHOLD = 12.0; 
const int MIN_STEP_DELAY = 300; 

// TIMER
unsigned long lastScreenUpdate = 0;

const unsigned char heart_icon[128] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x03, 0xC0, 
  0x07, 0xE0, 0x07, 0xE0, 0x0F, 0xF0, 0x0F, 0xF0, 0x1F, 0xF8, 0x1F, 0xF8, 
  0x3F, 0xFC, 0x3F, 0xFC, 0x3F, 0xFE, 0x7F, 0xFC, 0x7F, 0xFF, 0xFF, 0xFE, 
  0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFE, 
  0x3F, 0xFF, 0xFF, 0xFC, 0x3F, 0xFF, 0xFF, 0xFC, 0x1F, 0xFF, 0xFF, 0xF8, 
  0x0F, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xE0, 0x03, 0xFF, 0xFF, 0xC0, 
  0x01, 0xFF, 0xFF, 0x80, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0xFE, 0x00, 
  0x00, 0x3F, 0xFC, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x0F, 0xF0, 0x00, 
  0x00, 0x07, 0xE0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

const unsigned char foot_icon[128] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x07, 0xF0, 0x00, 0x00, 
  0x0F, 0xF8, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 
  0x1F, 0xFC, 0x00, 0x00, 0x1F, 0xFC, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 
  0x0F, 0xF0, 0x07, 0xC0, 0x07, 0xE0, 0x0F, 0xE0, 0x00, 0x00, 0x1F, 0xF0, 
  0x07, 0xE0, 0x3F, 0xF8, 0x0F, 0xF0, 0x3F, 0xF8, 0x1F, 0xF8, 0x3F, 0xF8, 
  0x1F, 0xF8, 0x3F, 0xF8, 0x1F, 0xF8, 0x3F, 0xF0, 0x0F, 0xF0, 0x1F, 0xE0, 
  0x00, 0x00, 0x0F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xC0, 
  0x00, 0x00, 0x1F, 0xE0, 0x00, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x3F, 0xF0, 
  0x00, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x1F, 0xE0, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// button interrupt function
void IRAM_ATTR handleButtonInterrupt(){
  buttonPressedFlag = true;
}

void setup(){
    Serial.begin(115200);
    
    // init screen
    tft.init();
    tft.setRotation(0);
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // WiFi connect
    tft.drawString("Connecting WiFi...", 120, 100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED){ delay(100); }
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK); 
    tft.drawString("Connected!", 120, 80);

    delay(1000);

    // location
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Finding Location...", 120, 120);
    detectLocation();

    delay(1000);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(myCity, 120, 120); // Show detected city
    delay(2000);

    // get time for this time zone
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    Wire.begin(I2C_SDA, I2C_SCL);
    if(!particleSensor.begin(Wire, I2C_SPEED_FAST)){
      while(1);
    }
    if (!mpu.begin()) {
      while(1);
    }
    
    // accelerometer settings
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    // heart rate sensor settings
    byte ledBrightness = 0x1F; 
    byte sampleAverage = 4; 
    byte ledMode = 2; 
    int sampleRate = 400; 
    int pulseWidth = 411; 
    int adcRange = 4096;
    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
    particleSensor.setPulseAmplitudeRed(ledBrightness);
    particleSensor.setPulseAmplitudeIR(ledBrightness);
    particleSensor.setPulseAmplitudeGreen(0);
    
    // initial weather fetch
    if(locationFound) getWeather();

    tft.fillScreen(TFT_BLACK);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
}

void loop(){

  // first check interrupt
  if(buttonPressedFlag){
    buttonPressedFlag = false;

    // account for debounce and chanfe theme
    if(millis() - lastDebounceTime > debounceDelay){
      currentThemeIndex++;
      if(currentThemeIndex >= totalThemes){
        currentThemeIndex = 0;
      }

      themeChanged = true;
      lastDebounceTime = millis();

      updateDisplay(beatAvg);
    }
  }


  // get IR and calculate hear rate
  long irValue = particleSensor.getIR();
  if (irValue < 50000) {
     beatsPerMinute = 0; 
     beatAvg = 0; 
     rateSpot = 0; 
     lastBeat = millis();
  } else {
    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);
      // normal range, remove outliers
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute; 
        rateSpot %= RATE_SIZE; 
        beatAvg = 0;

        // get average of last readings
        for (byte x = 0 ; x < RATE_SIZE ; x++) {
          beatAvg += rates[x];
        }

        beatAvg /= RATE_SIZE;
      }
    }
  }

  // detect steps every 50ms
  if (millis() - lastAccelRead > 50) {
     detectStep();
     lastAccelRead = millis();
  }

  // update weather every 15 min
  if (millis() - lastWeatherTime > 900000 && locationFound) {
     getWeather();
     lastWeatherTime = millis();
  }

  // update screen every second
  if (millis() - lastScreenUpdate > 1000) {
    updateDisplay(beatAvg);
    lastScreenUpdate = millis();
  }
}

// function for detecting steps (future improvement - check for rhytmic movement)
void detectStep() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float magnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                         a.acceleration.y * a.acceleration.y + 
                         a.acceleration.z * a.acceleration.z);
  if (magnitude > STEP_THRESHOLD) {
    if (millis() - lastStepTime > MIN_STEP_DELAY) {
      stepCount++;
      lastStepTime = millis();
    }
  }
}

// get location 
void detectLocation() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://ip-api.com/json/");
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      
      // extract ciry
      int cityStart = payload.indexOf("\"city\":\"") + 8;
      int cityEnd = payload.indexOf("\"", cityStart);
      if (cityStart > 8 && cityEnd > 0) {
        myCity = payload.substring(cityStart, cityEnd);
      }

      // extract latitude
      int latStart = payload.indexOf("\"lat\":") + 6;
      int latEnd = payload.indexOf(",", latStart);
      if (latStart > 6 && latEnd > 0) {
        myLat = payload.substring(latStart, latEnd).toFloat();
      }

      // extract longtitude
      int lonStart = payload.indexOf("\"lon\":") + 6;
      int lonEnd = payload.indexOf(",", lonStart);


      if (lonStart > 6 && lonEnd > 0) {
        myLon = payload.substring(lonStart, lonEnd).toFloat();
        locationFound = true;
      }
    }
    http.end();
  }
}


/// fetch weather based on location
void getWeather() {
  if (WiFi.status() == WL_CONNECTED && locationFound) {
    HTTPClient http;
    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(myLat) + "&longitude=" + String(myLon) + "&current_weather=true";    
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();

      int dataSectionIndex = payload.indexOf("\"current_weather\"");

      int tempIndex = payload.indexOf("\"temperature\":", dataSectionIndex); 
      if (tempIndex > 0) {
         int commaIndex = payload.indexOf(",", tempIndex);
         String tempString = payload.substring(tempIndex + 14, commaIndex);
         outdoorTemp = tempString.toFloat();
      }
    }
    http.end();
  }
}


// function for updating display
void updateDisplay(int bpm){
  struct tm timeinfo;
  // 
  if(!getLocalTime(&timeinfo)){ return; }

  // get the current theme
  ThemePalette t = themes[currentThemeIndex];
  if(themeChanged){
    tft.fillScreen(t.backgroundColor);
    themeChanged = false;
  }

  // Date
  char dateString[20];
  tft.setTextDatum(MC_DATUM);
  strftime(dateString, 20, "%a, %d %b", &timeinfo);
  tft.setTextColor(t.dateColor, t.backgroundColor);
  tft.setTextSize(2); 
  tft.drawString(dateString, 120, 40);  

  // Time
  char timeString[6]; 
  strftime(timeString, 7, "%H:%M", &timeinfo);
  tft.setTextColor(t.timeColor, t.backgroundColor);
  tft.setTextSize(6); 
  tft.drawString(timeString, 120, 90); 

  // Heart rate and steps
  int yPos = 135; 
  int xLeft = 30; 
  int xRight = 130;
  
  // Heart rate
  if (bpm > 0) {
     tft.drawBitmap(xLeft, yPos, heart_icon, 32, 32, t.heartColor);
     tft.setTextDatum(ML_DATUM); 
     tft.setTextColor(t.heartColor, t.backgroundColor); 
     tft.setTextSize(3);
     tft.setTextPadding(80); 
     tft.drawNumber(bpm, xLeft + 40, yPos + 16); 
  } else {
     tft.drawBitmap(xLeft, yPos, heart_icon, 32, 32, TFT_DARKGREY);
     tft.setTextDatum(ML_DATUM);
     tft.setTextColor(TFT_DARKGREY, t.backgroundColor);
     tft.setTextSize(3);
     tft.setTextPadding(80);
     tft.drawString("--", xLeft + 40, yPos + 16);
  }

  // Steps
  tft.drawBitmap(xRight, yPos, foot_icon, 32, 32, t.stepsColor);
  tft.setTextDatum(ML_DATUM); 
  tft.setTextColor(t.stepsColor, t.backgroundColor); 
  tft.setTextSize(3);
  tft.setTextPadding(80);
  char stepStr[10];
  sprintf(stepStr, "%d", stepCount);
  tft.drawString(stepStr, xRight + 40, yPos + 16);

  // Weather and city
  tft.setTextDatum(MC_DATUM); 
  tft.setTextColor(t.infoColor, t.backgroundColor);
  tft.setTextSize(2);
  
  String weatherCityStr;
  String weatherNumStr;

  if (locationFound) {
      weatherCityStr = myCity;
      weatherNumStr = String(outdoorTemp, 1) + "C";
  } else {
      weatherCityStr = "Locating..."; 
      weatherNumStr = "";
  }
  
  tft.drawString(weatherCityStr, 120, 190);
  tft.drawString(weatherNumStr, 120, 215);
}


