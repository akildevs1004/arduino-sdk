
#define ETH_PHY_ADDR 0
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18
#define ETH_POWER_PIN -1
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_TYPE ETH_PHY_LAN8720


//---------------------------NETWORK SETTINGS START------------------------------------------

// Static IP Configuration
IPAddress local_IP = IPAddress();
IPAddress gateway = IPAddress();
IPAddress subnet = IPAddress();
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);


// Simple credentials (replace with secure method for production)
const char* USERNAME = "admin";
const char* PASSWORD = "password";



//wifi
// WiFi credentials
String WIFI_SSID = "";
String WIFI_PASSWORD = "";

// Static IP Configuration for WiFi
IPAddress wifi_local_IP(192, 168, 1, 20);  // Using .100 as requested
IPAddress wifi_gateway(192, 168, 1, 1);
IPAddress wifi_subnet(255, 255, 255, 0);
IPAddress wifi_primaryDNS(8, 8, 8, 8);
IPAddress wifi_secondaryDNS(8, 8, 4, 4);

bool wifiConnected = false;
 





//---------------------------NETWORK SETTINGS END------------------------------------------
void startNetworkProcessStep1()
{
  //wifi
  //IPAddress local_IP=IPAddress();;
  
  Serial.print("USE_ETHERNET----------------------");
  Serial.println(USE_ETHERNET);

  configureWifiEtherNetServer();//server start   
 
}


void configureWifiEtherNetServer(){
  // Apply configuration
  if (USE_ETHERNET) {

    local_IP.fromString(config["eth_ip"].as<String>());
    gateway.fromString(config["eth_gateway"].as<String>());
    subnet.fromString(config["eth_subnet"].as<String>());

    DeviceIPNumber = config["eth_ip"].as<String>();

    ETH.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);


    // Your existing Ethernet setup code...
    if (!ETH.begin(ETH_TYPE, ETH_PHY_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE)) {
      Serial.println("Ethernet Failed to Start");
    }
    //wifi

    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
      Serial.println("An error occurred while mounting LittleFS");
      return;
    }

    WiFi.onEvent(WiFiEvent);

    if (!ETH.begin(ETH_TYPE, ETH_PHY_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE)) {
      Serial.println("Ethernet Failed to Start");
      return;
    }

    //   if ( ETH.linkStatus()==ETH_LINK_OFF) {
    //   delay(1000);
    // }

    // Apply static IP configuration
    if (!ETH.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("Failed to configure Ethernet with static IP");
    } else {
      Serial.println("Static IP: ");
      Serial.println(ETH.localIP());
      DeviceIPNumber = ETH.localIP().toString();
    }
  } else {
    Serial.print("Wifi SSID");
    Serial.println(config["wifi_ssid"].as<String>());
    {
      WiFi.begin(config["wifi_ssid"].as<String>(), config["wifi_password"].as<String>());


       while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Wifi Internet checking.");
  }

      IPAddress gateway_c = WiFi.gatewayIP();
      IPAddress subnet_c = WiFi.subnetMask();
      IPAddress wifi_id_c = IPAddress();
      wifi_id_c.fromString(config["wifi_ip"].as<String>());
      // WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
      WiFi.config(wifi_id_c, gateway_c, subnet_c);
      Serial.println("Wifi Static IP: ");
      Serial.println(WiFi.localIP());

      DeviceIPNumber = WiFi.localIP().toString();
    }
  }
 
  

}


void connectWiFiWithStaticIP() { 

  // WIFI_SSID = "akil";
  //  WIFI_PASSWORD = "Akil1234";
  Serial.println(WIFI_SSID);
  Serial.println("Saved Wifi Details");



  // Configure static IP
  if (!WiFi.config(wifi_local_IP, wifi_gateway, wifi_subnet, wifi_primaryDNS, wifi_secondaryDNS)) {
    Serial.println("Failed to configure WiFi with static IP");
    return;
  }

  // Connect to WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to WiFi");
  } else {
    Serial.println("\nWiFi connected");
    Serial.print("Static IP address: ");
    Serial.println(WiFi.localIP());
  }
}

// Your existing WiFi event handler
void WiFiEvent(WiFiEvent_t event) {
  Serial.println("WiFi Event Occurred");
  // Add specific event handling if needed
}


String getWiFiStatus() {
  switch (WiFi.status()) {
    case WL_CONNECTED: return "Connected (IP: " + WiFi.localIP().toString() + ")";
    case WL_NO_SSID_AVAIL: return "Network not available";
    case WL_CONNECT_FAILED: return "Connection failed";
    case WL_IDLE_STATUS: return "Idle";
    case WL_DISCONNECTED: return "Disconnected";
    default: return "Unknown status";
  }
}

String getEthernetStatus() {
  return ETH.linkUp() ? "Connected (IP: " + ETH.localIP().toString() + ")" : "Disconnected";
}

bool checkInternet() {
  //WiFiClient client;
  return client.connect("www.google.com", 80);
}

String getInternetStatus() {
  return checkInternet() ? "Online" : "Offline";
}