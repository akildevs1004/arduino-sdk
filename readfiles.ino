
String readConfig(String filename) {
  String path = "/" + filename;
  if (!LittleFS.exists(path)) {
    Serial.println("Config file not found: " + path);
    return "";
  }

  File file = LittleFS.open(path, "r");
  String content = file.readString();
  Serial.println("Config file   found Success: ");

  file.close();

  deserializeJson(config, content);
  //update Values from Device Config file to Program variables// for device.ino

  if (config.containsKey("max_doorcontact")) {
    countdownDuration = config["max_doorcontact"].as<long>();
  }

  if (config.containsKey("max_siren_pause")) {
    resetDuration = config["max_siren_pause"].as<long>();
  }

  if (config.containsKey("heartbeat")) {
    heartbeatInterval = config["heartbeat"].as<long>();
  }

  if (config.containsKey("max_temperature")) {
    TEMPERATURE_THRESHOLD = config["max_temperature"].as<double>();
  }
  if (config.containsKey("server_url")) {
    serverURL = config["server_url"].as<String>();
  }








  return content;
}

// Serve static files from LittleFS
String readFile(String path) {
  File file = LittleFS.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading: " + path);
    return "";
  }
  String content = file.readString();
  file.close();
  return content;
}

// Save data to config file
void saveConfig(String filename, String data) {
  File file = LittleFS.open("/" + filename, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing: " + filename);
    return;
  }
  file.print(data);
  file.close();
  Serial.println(data);
  Serial.println("Data saved to " + filename);
}



// Function to read, update, and write back JSON data using String for filenames
void updateJsonConfig(String filename, String param, String value) {


  // Open the file for reading
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file for reading");
    return;
  }

  // Allocate a buffer for the file content
  StaticJsonDocument<512> jsonDoc;

  // Deserialize the JSON data
  DeserializationError error = deserializeJson(jsonDoc, configFile);
  if (error) {
    Serial.print("Failed to parse JSON file: ");
    Serial.println(error.c_str());
    jsonDoc.clear();  // Initialize as an empty JSON object if parsing fails
  }
  configFile.close();  // Always close after reading

  // Add or update the parameter
  // jsonDoc[param] = value;


  if (value == "true") {
    jsonDoc[param] = true;
  } else if (value == "false") {
    jsonDoc[param] = false;
  } else {
    jsonDoc[param] = value;  // Assign as is if it's neither "true" nor "false"
  }




  // Open the file for writing (truncate the file)
  configFile = LittleFS.open("/config.json", FILE_WRITE);
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  // Serialize JSON to the file
  if (serializeJson(jsonDoc, configFile) == 0) {
    Serial.println("Failed to write to file");
  }

  // Close the file
  configFile.close();

  Serial.println("Configuration updated successfully.");

  Serial.println(param);
}