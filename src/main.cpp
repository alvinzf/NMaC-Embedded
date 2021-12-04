#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <HTTPClient.h>
#include <Wire.h>

#define WIFI_NETWORK "ANU"
#define WIFI_PASSWORD "gataulupa"
#define WIFI_TIMEOUT_MS 20000

#define RXp2 16
#define TXp2 17

const char *serverName = "http://192.168.137.1/esp32/esp-post-data.php";

String apiKeyValue = "tPmAT5Ab3j7F9";
String sensorName = "Table1";

#define SEALEVELPRESSURE_HPA (1013.25)

unsigned long lastTime = 0;
unsigned long timerDelay = 500;
unsigned long blinkInterval = 100;
unsigned long prevMillis = 0;

AsyncWebServer server(80);
int ledIndicator = 19;
int ledWarning = 21;
int sound = 33;
int ledState = LOW;

void connectToWiFi()
{
  Serial.print("Connectting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS)
  {
    delay(100);
    Serial.print(",");
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Failed");
  }
  else
  {
    Serial.print("Connected! ");
    Serial.println(WiFi.localIP());
  }
}

String readSensor()
{
  int val = analogRead(sound);
  // int dB = ((val + 83.2073) / 11.003) - 7;
  int dB = map(val, 0, 1500, 30, 115);
  return String(dB);
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2);
  connectToWiFi();

  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html"); });
  server.on("/noise", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readSensor().c_str()); });
  server.begin();

  pinMode(sound, INPUT);
}

void loop()
{

  if ((millis() - lastTime) > timerDelay)
  {
    int val = analogRead(sound);
    // int dB = ((val + 83.2073) / 11.003) - 7;
    // int dB = 20 * log10(val + 1);
    int dB = map(val, 0, 1500, 30, 115);
    String classification = "";
    if (Serial2.available() > 0)
    {

      classification = Serial2.readString();
    }
    Serial.print(val, DEC);
    Serial.print("\t");
    Serial.print(dB, DEC);
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
      WiFiClient client;
      HTTPClient http;

      http.begin(client, serverName);

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName + "&value=" + String(dB) + "&classification=" + classification + "";
      Serial.print("httpRequestData: ");
      Serial.println(httpRequestData);

      int httpResponseCode = http.POST(httpRequestData);

      if (httpResponseCode > 0)
      {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else
      {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
