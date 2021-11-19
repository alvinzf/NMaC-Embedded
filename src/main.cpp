#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <HTTPClient.h>
#include <Wire.h>

#define WIFI_NETWORK "ANU"
#define WIFI_PASSWORD "gataulupa"
#define WIFI_TIMEOUT_MS 20000

const char *serverName = "http://192.168.137.1/esp32/esp-post-data.php";

String apiKeyValue = "tPmAT5Ab3j7F9";
String sensorName = "Table1";

#define SEALEVELPRESSURE_HPA (1013.25)

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;
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
  int dB = ((val + 83.2073) / 11.003) - 7;
  // db = 20. * (log10 (soundSensor));
  // (dB) = 16.801 x ln(sensorValue/1023) + 9.872

  // int dB = 20 * log10(val + 1);
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
  int dB = ((val + 83.2073) / 11.003) - 7;
  // int dB = 20 * log10(val + 1);

  delay(100);
  digitalWrite(ledIndicator, HIGH);
  Serial.print(val, DEC);
  Serial.print("\t");
  Serial.print(dB, DEC);
  Serial.println();

  if (dB > 60)
  {
    // if (millis() - prevMillis >= blinkInterval)
    // {
    //   prevMillis = millis();
    //   if (ledState == LOW)
    //   {
    //     ledState = HIGH;
    //   }
    //   else
    //   {
    //     ledState = LOW;
    //   }
    // }

    // digitalWrite(ledWarning, ledState);
    digitalWrite(ledWarning, HIGH);
    delay(100);
    digitalWrite(ledWarning, LOW);
  }

  if ((millis() - lastTime) > timerDelay)
  {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
      WiFiClient client;
      HTTPClient http;

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);

      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      // Prepare your HTTP POST request data
      String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName + "&value=" + String(dB) + "";
      Serial.print("httpRequestData: ");
      Serial.println(httpRequestData);

      // You can comment the httpRequestData variable above
      // then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
      //String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&location=Office&value1=24.75&value2=49.54&value3=1005.14";

      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      // If you need an HTTP request with a content type: text/plain
      //http.addHeader("Content-Type", "text/plain");
      //int httpResponseCode = http.POST("Hello, World!");

      // If you need an HTTP request with a content type: application/json, use the following:
      //http.addHeader("Content-Type", "application/json");
      //int httpResponseCode = http.POST("{\"value1\":\"19\",\"value2\":\"67\",\"value3\":\"78\"}");

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
