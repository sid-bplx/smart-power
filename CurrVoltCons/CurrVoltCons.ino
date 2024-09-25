#include "EmonLib.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

EnergyMonitor emon;

// Module Details

int relay = 13;          //GPIO 13
int VoltageSensor = 35;  // GPIO 35
int CurrentSensor = 34;  // GPIO 34

#define vCalibration 160
#define currCalibration 120

#define phaseShift 1.7

// Wifi details

const char *ssid = "Kadam01";
const char *password = "8983151622";

int flag = 0;

String response;


void setup() {
  Serial.begin(115200);

  delay(1000);

  // WiFi Connection
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Module Init
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);

  emon.voltage(VoltageSensor, vCalibration, phaseShift);  // Voltage: input pin, calibration, phase_shift
  emon.current(CurrentSensor, currCalibration);           // Current: input pin, calibration.
}

void loop() {
  emon.calcVI(20, 2000);

  Serial.println(String("Current: ") + emon.Irms + (" Volts: ") + emon.Vrms + (" Watts: ") + emon.apparentPower + (" Relay: ") + digitalRead(relay));

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin("https://theft-api-2.tiluckdave.repl.co/fetch");

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {

      response = http.getString();

      Serial.println(httpResponseCode);
      Serial.println(String("Distributor Power: ") + response);
    } else {
      Serial.println("Error occurred while sending HTTP POST");
    }
  }

  float PD = response.toFloat() - emon.apparentPower;
  Serial.println(PD);
  if (flag > 2) {
    if ((PD >= 180) || (PD <= -180)) {
      digitalWrite(relay, 1);

      Serial.println("Theft Detected! Turning off the power supply!");
      if (WiFi.status() == WL_CONNECTED) {

        HTTPClient http;

        http.begin("https://theft-api-2.tiluckdave.repl.co/setTheft");
        http.addHeader("Content-Type", "application/json");

        StaticJsonDocument<200> doc;
        // Add values in the document

        doc["theft"] = true;

        String requestBody;
        serializeJson(doc, requestBody);

        int httpResponseCode = http.POST(requestBody);

        if (httpResponseCode > 0) {

          String response = http.getString();
          Serial.println(response);

        } else {
          Serial.println("Error occurred while sending HTTP POST");
        }
      }
      flag = 0;
      delay(15000);
    }
  } else {
    digitalWrite(relay, 0);
    if (WiFi.status() == WL_CONNECTED) {

      HTTPClient http;

      http.begin("https://theft-api-2.tiluckdave.repl.co/setTheft");
      http.addHeader("Content-Type", "application/json");

      StaticJsonDocument<200> doc;
      // Add values in the document

      doc["theft"] = false;

      String requestBody;
      serializeJson(doc, requestBody);

      int httpResponseCode = http.POST(requestBody);

      if (httpResponseCode > 0) {

        String response = http.getString();
        Serial.println(response);

      } else {
        Serial.println("Error occurred while sending HTTP POST");
      }
    }
    flag++;
    delay(100);
  }
}
