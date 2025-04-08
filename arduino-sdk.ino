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


HTTPClient http;
int cloudAccountActiveDaysRemaining = 0;


String todayDate;


// const char* ssid = "akil";
// const char* password = "Akil1234";

// // Expiry date (format: YYYY/MM/DD)
// String expiryDate = "2025/10/13";  // Example expiry date

// // Function to get the current date in YYYY/MM/DD format
// String getCurrentDate() {
//   struct tm timeInfo;
//   if (!getLocalTime(&timeInfo)) {
//     Serial.println("Failed to obtain time");
//     return "";
//   }

//   char dateStr[11];                                           // 10 characters + null terminator
//   strftime(dateStr, sizeof(dateStr), "%Y/%m/%d", &timeInfo);  // Format: YYYY/MM/DD

//   return String(dateStr);
// }

// // Function to convert a date string (YYYY/MM/DD) into epoch time (seconds)
// time_t getEpochFromDate(String dateStr) {
//   struct tm tm;
//   memset(&tm, 0, sizeof(struct tm));

//   int year = dateStr.substring(0, 4).toInt();
//   int month = dateStr.substring(5, 7).toInt();
//   int day = dateStr.substring(8, 10).toInt();

//   tm.tm_year = year - 1900;  // Years since 1900
//   tm.tm_mon = month - 1;     // Month range: 0-11
//   tm.tm_mday = day;

//   return mktime(&tm);
// }

// // Function to calculate the remaining days from today to the expiry date
// int calculateRemainingDays(String expiryDateStr) {
//   time_t now;
//   time(&now);  // Get current time (epoch time)

//   time_t expiry = getEpochFromDate(expiryDateStr);  // Get expiry date as epoch time

//   // Calculate the difference in seconds
//   double secondsRemaining = difftime(expiry, now);
//   int daysRemaining = secondsRemaining / 86400;  // Convert seconds to days

//   return daysRemaining;
// }

// void setupAccount() {

//   // Serial.begin(115200);

//   // // Connect to Wi-Fi
//   // WiFi.begin(ssid, password);
//   // Serial.print("Connecting to WiFi...");
//   // while (WiFi.status() != WL_CONNECTED) {
//   //   delay(500);
//   //   Serial.print(".");
//   // }
//   // Serial.println("Connected!");

//   // Sync time using NTP
//   configTime(0, 0, "pool.ntp.org");
//   delay(2000);  // Wait for NTP sync

//   // Get today's date
//   String todayDate = getCurrentDate();
//   Serial.println("Today's Date: " + todayDate);

//   // Calculate remaining days
//   int remainingDays = calculateRemainingDays(expiryDate);
//   Serial.println("Remaining Days to Expiry: " + String(remainingDays));
//   cloudAccountActiveDaysRemaining = remainingDays;
// }
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
