#include <ESP8266WiFi.h>
#include <DHT.h>
#include "config.h"
#include "domoticz/domoticz.h"


volatile bool isr = false;
DHT dht(DHT_PIN, DHTTYPE);
float humidity, temperature;
Domoticz domo = Domoticz();
WiFiServer server(80);
unsigned long update_temperature_time;

bool update_door_status(void);
void client_task(void);
bool update_temperature(float *temp, float *hum);


bool update_temperature(float *temp, float *hum)
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }

  Serial.print("Temperature: "); Serial.println(t, 2);
  Serial.print("Humidity: "); Serial.println(h, 2);
  *temp = t;
  *hum = h;
  return true;
}

void relay_pulse(void)
{
  digitalWrite(RELAY_PIN, 1);
  delay(RELAY_PULSE_TIME);
  digitalWrite(RELAY_PIN, 0);
}

void door_changed(void)
{
  isr = true;
}


void setup()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  Serial.println("Starting");

  pinMode(SENSE_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FORCE_PIN, OUTPUT);
  digitalWrite(FORCE_PIN, 0);
  attachInterrupt(digitalPinToInterrupt(SENSE_PIN), door_changed, CHANGE);

  if (domo.begin()) {
    Serial.println("Connect OK");
  } else {
    Serial.println("Connect Fail");
    ESP.restart();
  }

  if (update_door_status() == false) {
    ESP.restart();
  }

  server.begin();
  Serial.println("Server started");

  dht.begin();

  update_temperature(&temperature, &humidity);
  domo.udpate_temp_hum(IDX_GARAGE_TEMP, temperature, humidity);
  update_temperature_time = millis() + 1000 * 60 * UPDATE_TEMPERATURE_DELAY;
}

void loop() {
  if (millis() > update_temperature_time) {
    update_temperature(&temperature, &humidity);
    domo.udpate_temp_hum(IDX_GARAGE_TEMP, temperature, humidity);
    update_temperature_time = millis() + 1000 * 60 * UPDATE_TEMPERATURE_DELAY;
  }
  if (isr) {
    delay(2000);
    if (update_door_status() == false) {
      ESP.restart();
    }
    isr = false;
  }
  client_task();
}


bool update_door_status(void)
{
  if (digitalRead(SENSE_PIN) ) {
    Serial.println("Door Openned");
    if (domo.update_switch(IDX_GARAGE_DOOR, false)) {
      Serial.println("Update Open OK ...");;
    } else {
      Serial.println("Update Open KO ...");;
    }

  } else {
    Serial.println("Door Closed");
    domo.update_switch(IDX_GARAGE_DOOR, true);
  }
}

void client_task(void)
{
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

