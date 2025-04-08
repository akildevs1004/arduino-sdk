//http://192.168.3.192:8000/api/get_device_company_info_arduino?serial_number=XT123456


void getDeviceAccoutnDetails() {
  String serverDeviceURL = "http://192.168.3.192:8000/api/get_device_company_info_arduino?serial_number=XT123456";

  Serial.println("Reading Account Details...");

  http.begin(serverDeviceURL);                         // Initialize HTTP client with URL
  http.addHeader("Content-Type", "application/json");  // Optional for GET

  int httpCode = http.GET();  // Send the request

  if (httpCode > 0) {
    Serial.println("HTTP Response Code: " + String(httpCode));

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Response: " + payload);

      StaticJsonDocument<256> accountDoc;

      DeserializationError error = deserializeJson(accountDoc, payload);

      if (error) {
        Serial.print("JSON Parsing Failed: ");
        Serial.println(error.c_str());
        return;
      }

      String company = accountDoc["company"]["name"];
      String accountExpiry = accountDoc["company"]["expiry"];



      // Expiry date (format: YYYY/MM/DD)
      // String expiryDate = "2025/10/13";  // Example expiry date

      // Sync time using NTP
      configTime(0, 0, "pool.ntp.org");
      delay(2000);  // Wait for NTP sync

      // Get today's date
      String todayDate111 = getCurrentDate();
      Serial.println("Today's Date: " + todayDate111);

      // // Calculate remaining days
      cloudAccountActiveDaysRemaining = calculateRemainingDays(accountExpiry);
      Serial.println("Remaining Days to Expiry: " + String(cloudAccountActiveDaysRemaining));

      updateJsonConfig("config.json", "cloud_company_name", company);
      updateJsonConfig("config.json", "cloud_account_expire", accountExpiry);
      updateJsonConfig("config.json", "cloudAccountActiveDaysRemaining", String(cloudAccountActiveDaysRemaining));


      Serial.println("Parsed Values:");
      Serial.println("Company: " + company);
      Serial.println("Status: " + accountExpiry);
      Serial.println("Days Remaining: " + String(cloudAccountActiveDaysRemaining));

      Serial.println("---------------------------------------------------------------------------");

      // // // Sync time using NTP
      // // configTime(0, 0, "pool.ntp.org");
      // // delay(2000);  // Wait for NTP sync

      // // // Get today's date
      // // String todayDate = getCurrentDate();
      // Serial.println("Today's Date: " + todayDate);

      // // Calculate remaining days
      // int remainingDays = calculateRemainingDays(expiryDate);
      // Serial.println("-------------------------Remaining Days to Expiry: " + String(remainingDays));
    }
  } else {
    Serial.println("HTTP Error: " + String(http.errorToString(httpCode)));

    cloudAccountActiveDaysRemaining = 40;

    updateJsonConfig("config.json", "cloud_company_name", "DEMO COMPANY");
    updateJsonConfig("config.json", "cloud_account_expire", "Not Set");
    updateJsonConfig("config.json", "cloudAccountActiveDaysRemaining", String(cloudAccountActiveDaysRemaining));
  }

  http.end();  // Close connection
}

// String expiryDate = "2025/10/13";  // Example expiry date
// // Function to get the current date in YYYY/MM/DD format
String getCurrentDate() {



  struct tm timeInfo;
  int retry = 0;
  const int retryCount = 10;

  while (!getLocalTime(&timeInfo) && retry < retryCount) {
    Serial.println("⏳ Waiting for NTP time...");
    delay(1000);
    retry++;
  }



  char dateStr[11];                                           // 10 characters + null terminator
  strftime(dateStr, sizeof(dateStr), "%Y/%m/%d", &timeInfo);  // Format: YYYY/MM/DD

  return String(dateStr);
}

// Function to convert a date string (YYYY/MM/DD) into epoch time (seconds)
time_t getEpochFromDate(String dateStr) {
  struct tm tm;
  memset(&tm, 0, sizeof(struct tm));

  int year = dateStr.substring(0, 4).toInt();
  int month = dateStr.substring(5, 7).toInt();
  int day = dateStr.substring(8, 10).toInt();

  tm.tm_year = year - 1900;  // Years since 1900
  tm.tm_mon = month - 1;     // Month range: 0-11
  tm.tm_mday = day;

  return mktime(&tm);
}

// Function to calculate the remaining days from today to the expiry date
int calculateRemainingDays(String expiryDateStr) {
  time_t now;
  time(&now);  // Get current time (epoch time)

  time_t expiry = getEpochFromDate(expiryDateStr);  // Get expiry date as epoch time

  // Calculate the difference in seconds
  double secondsRemaining = difftime(expiry, now);
  int daysRemaining = secondsRemaining / 86400;  // Convert seconds to days

  return daysRemaining;
}