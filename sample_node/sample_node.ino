#include <ESP8266WiFi.h>
#include "config.h"
#include "domoticz/domoticz.h"

#ifdef DS18B20_PIN
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(DS18B20_PIN);
DallasTemperature DS18B20(&oneWire);
#endif

#ifdef DHT_PIN
#include <DHT.h>
DHT dht(DHT_PIN, DHTTYPE);
#endif


Domoticz domo = Domoticz();

#if ( SERVER_PORT > 0)
WiFiServer server(80);
void server_task(void);
#endif



char buff[100];
char name_buff[20];
char sw_status[20];

#ifdef DHT_PIN
bool update_dht_temperature(float *temp, float *hum)
{
  dht.begin();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("Failed reading DHT sensor!");
    return false;
  }
  DEBUG_PRINT("DHT Temperature: "); DEBUG_PRINTLN(t, 2);
  DEBUG_PRINT("DHT Humidity: "); DEBUG_PRINTLN(h, 2);
  *temp = t;
  *hum = h;
  return true;
}
#endif

#ifdef DS18B20_PIN
bool update_ds18b20_temperature(float *temp)
{
  float t;
  DS18B20.begin();
  DS18B20.requestTemperatures();
  t = DS18B20.getTempCByIndex(0);
  pinMode(DS18B20_PIN, OUTPUT);
  digitalWrite(DS18B20_PIN, LOW);
  if (isnan(t)) {
    DEBUG_PRINTLN("Failed reading DS18B20!");
    return false;
  }
  DEBUG_PRINT("DS18B20 Temperature: "); DEBUG_PRINTLN(t, 2);
  *temp = t;
}
#endif



void setup()
{
  float f;
  uint8_t h;
  uint16_t p;
  uint8_t i;

  Serial.begin(115200);
  DEBUG_PRINTLN("\nStarting");

  for (i = 0; i < 2; i++) {
    if (update_ds18b20_temperature(&f)) {
      DEBUG_PRINT("DS temperature: "); DEBUG_PRINTLN(f);
    }
  }

  if (domo.begin()) {
    DEBUG_PRINTLN("Connect OK");
  } else {
    DEBUG_PRINTLN("Connect Fail");
    ESP.restart();
  }

  if (domo.get_servertime(buff)) {
    DEBUG_PRINT("Server Time:"); DEBUG_PRINTLN(buff);
  }
  if (domo.get_temperature(IDX_GARAGE_TEMP, &f, name_buff)) {
    DEBUG_PRINT("Temperature Garage:"); DEBUG_PRINTLN(f);
  }
  if (domo.get_temperature(IDX_FREEZER, &f, name_buff)) {
    DEBUG_PRINT("Temperature Congelateur:"); DEBUG_PRINTLN(f);
  }
  if (domo.get_voltage(IDX_BATTERY, &f, name_buff)) {
    DEBUG_PRINT("Girouette Batterie:"); DEBUG_PRINTLN(f);
  }
  if (domo.get_temp_hum_baro(IDX_BARO, &f, &h, &p, name_buff)) {
    DEBUG_PRINT(name_buff); DEBUG_PRINT(" :" ); DEBUG_PRINT("T:"); DEBUG_PRINT(f); DEBUG_PRINT(" Hum:"); DEBUG_PRINT(h); DEBUG_PRINT(" Pression:")DEBUG_PRINTLN(p);
  }
  if (domo.get_switch_status(IDX_GARAGE_DOOR, sw_status, name_buff)) {
    DEBUG_PRINT(name_buff); DEBUG_PRINT(" :" ); DEBUG_PRINT("Status:"); DEBUG_PRINTLN(sw_status);
  }

  if (domo.update_temperature(IDX_DUMMY_TEMP, 13.5)) {
    DEBUG_PRINTLN("Temp Sensor Updated");
  }

  DEBUG_PRINT("Vbat (V): "); DEBUG_PRINTLN(domo.vbat() );
  DEBUG_PRINT("Vbat (%): "); DEBUG_PRINTLN(domo.vbat_percentage() );
  DEBUG_PRINT("RSSI (bBm): "); DEBUG_PRINTLN(domo.rssi() );
  DEBUG_PRINT("RSSI (12level): "); DEBUG_PRINTLN(domo.rssi_12level() );

#if ( SERVER_PORT > 0)
  server.begin();
  DEBUG_PRINTLN("Server started");
#endif
}




void loop() {




#if ( SERVER_PORT > 0)
  server_task();
#endif

#if (DEEP_SLEEP_TIME > 0 )
  ESP.deepSleep(DEEP_SLEEP_TIME * 60 * 1000000);
  delay(100);
#endif

}



#if ( SERVER_PORT > 0)
void server_task(void)
{
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  DEBUG_PRINTLN("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  DEBUG_PRINTLN(req);
  client.flush();

  // Match the request
  if (req.indexOf(" / relay") != -1) {
    DEBUG_PRINTLN("Relay Request");
  }
  else if (req.indexOf(" / reset") != -1) {
    DEBUG_PRINTLN("Reset Request");
    ESP.restart();
  }
  else {
    DEBUG_PRINTLN("invalid request");
    client.stop();
    return;
  }
  client.flush();
  String s = "HTTP / 1.1 200 OK\r\nContent - Type: text / html\r\n\r\n < !DOCTYPE HTML > \r\n<html>\r\nDoor Garage Activation OK ";
  s += " < / html > \n";
  // Send the response to the client
  client.print(s);
  delay(1);
  DEBUG_PRINTLN("Client disconnected");
}
#endif
