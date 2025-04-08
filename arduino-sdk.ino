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




String firmWareVersion = "1.0.0";

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


    updateJsonConfig("config.json", "ipaddress", DeviceIPNumber);
    updateJsonConfig("config.json", "firmWareVersion", firmWareVersion);


    socketConnectServer();
    handleHeartbeat();
    devicePinDefination();
    updateFirmWaresetup();
    // updateFirmwareDatasetup();

    uploadHTMLsetup();
  }
}

void loop() {

  server.handleClient();

  if (WiFi.status() == WL_CONNECTED) {


     handleHeartbeat();
    updateFirmWareLoop();

    //////////////////////////deviceReadSensorsLoop();
    delay(100);  // Non-blocking delay
  }
}

String replaceHeaderContent(String html )
{
  html.replace("{firmWareVersion}", firmWareVersion);
    html.replace("{ipAddress}", DeviceIPNumber);
    html.replace("{loginErrorMessage}", loginErrorMessage);
    html.replace("{GlobalWebsiteResponseMessage}", GlobalWebsiteResponseMessage);
    html.replace("{GlobalWebsiteErrorMessage}", GlobalWebsiteErrorMessage);

    return html;
}
