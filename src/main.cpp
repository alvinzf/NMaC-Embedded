#include <Arduino.h>
#include "WiFi.h"

#define WIFI_NETWORK "ANU"
#define WIFI_PASSWORD "gataulupa"
#define WIFI_TIMEOUT_MS 20000

void connectToWiFi(){
  Serial.print("Connectting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS){
    Serial.print(",");
    delay(100);
  }
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Failed");
    // take action

  }
  else {
    Serial.print("Connected!");
    Serial.println(WiFi.localIP());
  }

}


int ledIndicator = 19;
int ledWarning = 21;
int sound = 33;


void setup(){
  Serial.begin(9600);
  connectToWiFi();


  pinMode(ledIndicator, OUTPUT);
  pinMode(ledWarning, OUTPUT);
  pinMode(sound, INPUT);  
}

void loop(){
  
  int val = analogRead(sound);
  int dB = (val+83.2073)/11.003;
  
  delay(100);
  digitalWrite(ledIndicator, HIGH);
  Serial.print(val,DEC);
  Serial.print("\t");
  Serial.print(dB,DEC);  
  Serial.println();

  if (dB > 60)
  {
    digitalWrite(ledWarning, HIGH);
    delay(1000);
    digitalWrite(ledWarning, LOW);
  }

}
