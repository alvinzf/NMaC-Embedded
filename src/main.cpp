#include <Arduino.h>
#include "WiFi.h"
#include <HTTPClient.h>

#define WIFI_NETWORK "ANU"
#define WIFI_PASSWORD "gataulupa"
#define WIFI_TIMEOUT_MS 20000

#define RXp2 16
#define TXp2 17

const char *serverName = "http://192.168.137.1/esp32/esp-post-data.php";

String apiKeyValue = "tPmAT5Ab3j7F9";
String sensorName = "Table1";
String classification = "";
String data;
String dB;

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
  }
  else
  {
    Serial.print("Connected! ");
    Serial.println(WiFi.localIP());
  }
}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2);
  connectToWiFi();

  pinMode(sound, INPUT);
}

void loop()
{

  if (Serial2.available() > 0)
  {
    classification = "";
    data = Serial2.readString();
    dB = getValue(data, '&', 0);
    classification = getValue(data, '&', 1);
    Serial.print("dB: ");
    Serial.println(dB);
    Serial.print("classification: ");
    Serial.println(classification);

    if (WiFi.status() == WL_CONNECTED)
    {
      WiFiClient client;
      HTTPClient http;

      http.begin(client, serverName);

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName + "&value=" + dB + "&classification=" + classification + "";
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
      // Serial.println("WiFi Disconnected");
    }
    // lastTime = millis();
  }
}
