#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#define WIFI_NETWORK "ANU"
#define WIFI_PASSWORD "gataulupa"
#define WIFI_TIMEOUT_MS 20000

AsyncWebServer server(80);
int ledIndicator = 19;
int ledWarning = 21;
int sound = 33;

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
    // take action
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
  // int dB = (val + 83.2073) / 11.003;
  // db = 20. * (log10 (soundSensor));
  // (dB) = 16.801 x ln(sensorValue/1023) + 9.872

  int dB = 20 * log10(val + 1);
  return String(dB);
}

void setup()
{
  Serial.begin(9600);
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
  pinMode(ledIndicator, OUTPUT);
  pinMode(ledWarning, OUTPUT);
  pinMode(sound, INPUT);
}

void loop()
{

  int val = analogRead(sound);
  // int dB = (val + 83.2073) / 11.003;
  int dB = 20 * log10(val + 1);

  delay(100);
  digitalWrite(ledIndicator, HIGH);
  Serial.print(val, DEC);
  Serial.print("\t");
  Serial.print(dB, DEC);
  Serial.println();

  if (dB > 60)
  {
    digitalWrite(ledWarning, HIGH);
    delay(1000);
    digitalWrite(ledWarning, LOW);
  }
}
