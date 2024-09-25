#include "EmonLib.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

EnergyMonitor emon;

// Module Details
int VoltageSensor = 34; // GPIO 35
int CurrentSensor = 35; // GPIO 34

#define vCalibration 150
#define currCalibration 90

#define phaseShift 1.7

// Wifi details

const char *ssid = "Kadam01";
const char *password = "8983151622";


void setup()
{
  Serial.begin(115200);

  delay(1000);

  // WiFi Connection
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Module Init
  emon.voltage(VoltageSensor, vCalibration, phaseShift); // Voltage: input pin, calibration, phase_shift
  emon.current(CurrentSensor, currCalibration); // Current: input pin, calibration.
}

void loop()
{
  emon.calcVI(20, 2000);
  Serial.println(String("Current: ") + emon.Irms + (" Volts: ") + emon.Vrms + (" Watts: ") + emon.apparentPower);

  if (WiFi.status() == WL_CONNECTED) {
     
    HTTPClient http;   
     
    http.begin("https://theft-api-2.tiluckdave.repl.co/send");  
    http.addHeader("Content-Type", "application/json");         
     
    StaticJsonDocument<200> doc;
    // Add values in the document
  
    doc["voltage"] = emon.Vrms;
    doc["current"] = emon.Irms;
    doc["apparentPower"] = emon.apparentPower;
   
    String requestBody;
    serializeJson(doc, requestBody);
     
    int httpResponseCode = http.POST(requestBody);
 
    if(httpResponseCode>0){
       
      String response = http.getString();                       
       
      Serial.println(httpResponseCode);   
      Serial.println(response);
     
    }
    else {
      Serial.println("Error occurred while sending HTTP POST");
    }
  }
  
}

