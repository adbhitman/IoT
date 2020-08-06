/*
  Connect ESP32 to AskSensors
 * Description:  This sketch connects to a website (https://asksensors.com) using an ESP32 Wifi module.
 *  Author: https://asksensors.com, 2018
 *  github: https://github.com/asksensors
 */
 
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

//These are for BME280 sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
#define LIGHTSENSORPIN 34 //Valosensori pinnissä 34
byte movementSensorPin = 26;

WiFiMulti WiFiMulti;
HTTPClient ask;
// TODO: user config
const char* ssid     = ""; //Wifi SSID
const char* password = ""; //Wifi Password
const char* apiKeyIn = "";      // API KEY IN
const char* apiKeyIn2 = "";     // second api key for sensor2
const unsigned int writeInterval = (24*60*60*1000)/50;   // write interval (in ms) 50 times in a day

// ASKSENSORS API host config
const char* host = "api.asksensors.com";  // API host name
const int httpPort = 80;      // port

// function to called when interruption is happened
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");

  // Some reason this code stops at this point and just throws error:
  // Guru Meditation Error: Core  1 panic'ed (Interrupt wdt timeout on CPU1)
  
  sendData(makeUrl2(apiKeyIn2));
  
  boolean interrupt = true;
  static uint32_t now = millis();
  unsigned int waitingTime = 60000; // time to wait until we are sure that there is not any movement

  while(interrupt){
    if(digitalRead(movementSensorPin) == 1){
      now = millis(); // resets time if motion detected during waiting period
    }
    if (millis() - now >= waitingTime) {    
      interrupt = false;
      sendData(makeUrl2(apiKeyIn2));
      detachInterrupt(movementSensorPin);
      Serial.println("Interrupt Detached!");
    }
  }
}
  
void setup(){
  Serial.println(F("BME280 test"));
  if (!bme.begin(0x76)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while (1) delay(10);
  }

  pinMode(LIGHTSENSORPIN, INPUT);
  pinMode(movementSensorPin, INPUT_PULLUP);

  // Making interrupt function
  attachInterrupt(digitalPinToInterrupt(movementSensorPin), detectsMovement, RISING);
  
  // open serial
  Serial.begin(115200);
  Serial.println("*****************************************************");
  Serial.println("********** Program Start : Connect ESP32 to AskSensors.");
  Serial.println("Wait for WiFi... ");

  // connecting to the WiFi network
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  // connected
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop(){
  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }else {

    // Create a URLs for different sensors with right modules
    String url = makeUrl1(apiKeyIn);
    String url2 = makeUrl2(apiKeyIn2);

    //Checking 

    // Sending data to server
    sendData(url);
    sendData(url2);

  }

  client.stop();  // stop client

  delay(writeInterval);    // delay
}

// Sensor1/first apiKeyIn creation
String makeUrl1(String apiKeyIn) {
  String url = "http://api.asksensors.com/write/";
  url += apiKeyIn;
  url += "?module1=";
  url += bme.readTemperature();
  url += "&module2=";
  url += bme.readTemperature();
  url += "&module3=";
  url += bme.readHumidity();
  url += "&module4=";
  url += bme.readPressure()/100.0;
  return url;
}

// Sensor2/second apiKeyIn2 creation
String makeUrl2(String apiKeyIn) {
  String url = "http://api.asksensors.com/write/";
  url += apiKeyIn;
  url += "?module1=";
  url += analogRead(LIGHTSENSORPIN);
  url += "&module2=";
  url += analogRead(LIGHTSENSORPIN);
  url += "&module3=";
  url += digitalRead(movementSensorPin);
  url += "&module4=";
  url += digitalRead(movementSensorPin);
  return url;
}

// sending data in url to server
void sendData(String url){
  Serial.print("********** requesting URL: ");
  Serial.println(url);
   // send data 
   ask.begin(url); //Specify the URL
  
    //Check for the returning code
    int httpCode = ask.GET();          
 
    if (httpCode > 0) { 
 
        String payload = ask.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      } else {
      Serial.println("Error on HTTP request");
    }
 
    ask.end(); //End 
    Serial.println("********** End ");
    Serial.println("*****************************************************");
}
