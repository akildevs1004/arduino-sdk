#include <WiFi.h>
#include <ETH.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <DHT22.h>
#include <HTTPClient.h>

WiFiManager wifiManager;


DynamicJsonDocument config(1024);  // Allocate 1024 bytes for JSON storage
String sessionToken = "";

bool loginStatus = false;
bool USE_ETHERNET = true;
WiFiClient client;  // Create a client object
WebServer server(80);



void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS is Not available");

    delay(1000);
  } else {
    String savedData = readConfig("config.json");
    Serial.println(savedData);
    if (savedData != "") {
      deserializeJson(config, savedData);
      if (config["wifi_or_ethernet"].as<String>() == "1")
        USE_ETHERNET = false;
    }
  } 

  startNetworkProcessStep1();  //load config and start Device web server

  routes();  // Define Routes and handlers
  server.begin();
  Serial.println("HTTP Server started");
  if (WiFi.status() == WL_CONNECTED) {

    
    socketConnectServer();
    handleHeartbeat();
    devicePinDefination();
  }
}

void loop() {

  server.handleClient();

  if (WiFi.status() == WL_CONNECTED) {

     
    handleHeartbeat();

    //////////////////////////deviceReadSensorsLoop();
    delay(100);  // Non-blocking delay
  }
}
