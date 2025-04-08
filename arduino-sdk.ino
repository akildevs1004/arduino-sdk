#include <WiFi.h>
#include <ETH.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <DHT22.h>
#include <HTTPClient.h>
#include "esp_ping.h"
#include "esp_log.h"
#include "lwip/inet.h"


#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>


#include <WiFiClient.h>

#include <Update.h>
#include "FS.h"
// #include "SPIFFS.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <time.h>


String firmWareVersion = "2.0";

WiFiManager wifiManager;


DynamicJsonDocument config(1024);  // Allocate 1024 bytes for JSON storage
String sessionToken = "";

bool loginStatus = false;
bool USE_ETHERNET = true;
WiFiClient client;  // Create a client object
WebServer server(80);

String deviceConfigContent;
String DeviceIPNumber;

String loginErrorMessage;

String GlobalWebsiteResponseMessage;
String GlobalWebsiteErrorMessage;


HTTPClient http;
int cloudAccountActiveDaysRemaining = 0;
unsigned long lastRun = 0;
const unsigned long interval = 24UL * 60UL * 60UL * 1000UL;  // 24 hours in milliseconds
// const unsigned long interval = 10UL * 1000UL;  // 24 hours in milliseconds


String todayDate;


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

  lastRun = millis();  // Initial timestamp

  routes();  // Define Routes and handlers
  server.begin();
  Serial.println("HTTP Server started");



  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print("Wifi Internet checking...........");
  // }
  getDeviceAccoutnDetails();
  if (WiFi.status() == WL_CONNECTED) {

    delay(1000);
    updateJsonConfig("config.json", "ipaddress", DeviceIPNumber);
    updateJsonConfig("config.json", "firmWareVersion", firmWareVersion);

    getDeviceAccoutnDetails();


    socketConnectServer();
    handleHeartbeat();
    devicePinDefination();
    updateFirmWaresetup();
    uploadHTMLsetup();
  }

  if (cloudAccountActiveDaysRemaining <= 0) {
    Serial.println("❌ XXXXXXXXXXXXXXXXXXXXXXXXXXXXX----Account is expired----XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
  }
}

void loop() {

  server.handleClient();

  if (WiFi.status() == WL_CONNECTED) {

    if (cloudAccountActiveDaysRemaining > 0) {

      handleHeartbeat();
      updateFirmWareLoop();

      unsigned long currentMillis = millis();
      // Serial.print(currentMillis - lastRun);
      // Serial.print("----");

      // Serial.println(interval);

      if (currentMillis - lastRun >= interval) {
        lastRun = currentMillis;
        getDeviceAccoutnDetails();
      }
    } else

    {
      //Serial.println("❌ XXXXXXXXXXXXXXXXXXXXXXXXXXXXX----Account is expired----XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }



    //////////////////////////deviceReadSensorsLoop();
    delay(100);  // Non-blocking delay
  }
}

String replaceHeaderContent(String html) {
  html.replace("{firmWareVersion}", firmWareVersion);
  html.replace("{ipAddress}", DeviceIPNumber);
  html.replace("{loginErrorMessage}", loginErrorMessage);
  html.replace("{GlobalWebsiteResponseMessage}", GlobalWebsiteResponseMessage);
  html.replace("{GlobalWebsiteErrorMessage}", GlobalWebsiteErrorMessage);

  html.replace("{cloud_company_name}", config["cloud_company_name"].as<String>());
  html.replace("{cloud_account_expire}", config["cloud_account_expire"].as<String>());
  html.replace("{cloudAccountActiveDaysRemaining}", String(cloudAccountActiveDaysRemaining));



  return html;
}
