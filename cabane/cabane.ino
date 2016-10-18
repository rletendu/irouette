#include <ESP8266WiFi.h>
#include <DHT.h>
#include "config.h"
#include "domoticz/domoticz.h"



DHT dht(DHT_PIN, DHTTYPE);
float humidity, temperature;
Domoticz domo = Domoticz();





bool update_temperature(float *temp, float *hum);


bool update_temperature(float *temp, float *hum)
{
  dht.begin();
  float h = dht.readHumidity();
  float t = dht.readTemperature();


  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }

  Serial.print("Temperature: "); Serial.println(t, 2);
  Serial.print("Humidity: "); Serial.println(h, 2);
  *temp = t;
  *hum = h;
  return true;
}



void setup()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.println("\nStarting");



  if (domo.begin()) {
    Serial.println("Connect OK");
  } else {
    Serial.println("Connect Fail");
    ESP.restart();
  }

  dht.begin();
  if (update_temperature(&temperature, &humidity)) {
    domo.udpate_temp_hum(IDX_CABANE_TH, temperature, humidity);
  }
  delay(100);
  Serial.println("ESP8266 in sleep mode");
  ESP.deepSleep(UPDATE_TEMPERATURE_DELAY * 60 * 1000000);
  delay(100);
}



void loop()
{

}




