#include <ESP8266WiFi.h>
#include <DHT.h>


#include "config.h"
#include "domoticz.h"

#define DHTTYPE DHT22
#define RELAY_PIN D1
#define DHT_PIN D4
#define FORCE_PIN  D2
#define SENSE_PIN  D3

volatile bool isr = false;
volatile uint8_t isr_count = 0;
DHT dht(DHT_PIN, DHTTYPE); // 11 works fine for ESP8266

float humidity, temp_f;  // Values read from sensor

Domoticz domo = Domoticz();
WiFiServer server(80);


void update_temperature() {

  humidity = dht.readHumidity();          // Read humidity (percent)
  temp_f = dht.readTemperature();     // Read temperature as Fahrenheit
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temp_f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temperature: "); Serial.println(temp_f, 2);
  Serial.print("Humidity: "); Serial.println(humidity, 2);

}

void relay_pulse(void)
{
  digitalWrite(RELAY_PIN, 1);
  delay(1000);
  digitalWrite(RELAY_PIN, 0);
}

void door_opened(void)
{
  isr_count++;
  isr = true;
  Serial.print("ISR "); Serial.println(isr_count);
}


void setup() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  Serial.println("Starting");
  // put your setup code here, to run once:
  pinMode(SENSE_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FORCE_PIN, OUTPUT);
  digitalWrite(FORCE_PIN, 0);
  attachInterrupt(digitalPinToInterrupt(SENSE_PIN), door_opened, CHANGE);

  if (digitalRead(SENSE_PIN) ) {
    Serial.println("Door Openned");

  } else {
    Serial.println("Door Closed");
  }
  if (domo.begin()) {
    Serial.println("Connect OK");
  } else {
    Serial.println("Connect Fail");
  }
  server.begin();
  Serial.println("Server started");

  dht.begin(); 
  update_temperature();
}

void loop() {
  /*
  if (WiFi.status() != WL_CONNECTED) {
    ESP.restart();
  }*/
  //update_temperature();
  if (isr) {
    delay(2000);
    if (digitalRead(SENSE_PIN) ) {
      Serial.println("Door Openned");
      Serial.println("Closing...");
      if (domo.update_switch(IDX_GARAGE_DOOR, false)) {
        Serial.println("Update Open OK ...");;
      } else {
        Serial.println("Update Open KO ...");;
      }

    } else {
      Serial.println("Door Closed");
      domo.update_switch(IDX_GARAGE_DOOR, true);
    }
    isr = false;
  }


  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Match the request
  int val;
  if (req.indexOf("/relay") != -1)
    relay_pulse();
  else if (req.indexOf("/reset") != -1)
    ESP.restart();
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }


  client.flush();
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nDoor Garage Activation OK ";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");


}
