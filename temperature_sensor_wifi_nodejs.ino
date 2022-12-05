// Include Wifi library
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

// Include Sensor library
#include <ArduinoOTA.h>

// Include the WebServer library
#include <ESP8266WebServer.h>   

// Include Sensor library
#include <LOLIN_HP303B.h>

//Create server object on port 80
ESP8266WebServer server(80); 


//Network Configuration, SSID and password
const char* ssid = "bambify";
const char* password = "2301198823071992";

// HP303B Opject
LOLIN_HP303B HP303BPressureSensor;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting 123");

  //Configure buildin led, this we use as heartbeat to see if board is still running
  pinMode(LED_BUILTIN, OUTPUT);

  // -- START WIFI setup -------------------- 

  // Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("WiFi connected\n");
 
  // Print the IP address
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print("IP address: end");
  
  Serial.print("\n");

  // -- END WIFI setup --------------------------------
  // -- START over the air updates -------------------- 

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  //
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  // -- END over the air updates -------------------- 

  // Barometric snesor init
  HP303BPressureSensor.begin();

  // Webserver init and handle requests to root
  Serial.println("Setting up webserver\n");
  server.on("/", handleRoot);      // Which routine to handle at root location
  server.begin();    

  //All done :) 
  Serial.println("");
  Serial.println("Init complete");
  
}


void handleRoot() {

  int32_t temperature;
  int16_t oversampling = 7;
  int16_t ret;
  Serial.println();

  ret = HP303BPressureSensor.measureTempOnce(temperature, oversampling);

  if (ret != 0)
  {
    //Something went wrong.
    //Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  }
  else
  {
    Serial.print("Temperature: ");
    Serial.print(temperature);
  }
  
  server.send(200, "text/plain", "{ \"temperature\": " + String(temperature) + "}");
}


void loop()
{  
  ArduinoOTA.handle();
  server.handleClient();          //Handle client requests  
}
