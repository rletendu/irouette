
#include "config.h"
#include "domoticz/domoticz.h"


#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

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

char buff[20];
char name_buff[20];
char sw_status[20];

int  sleep_time;
int  idx;




void setup()
{
  float f;
  uint8_t h;
  uint16_t p;
  uint8_t i;
  int port;
  float vbat_min;

  Serial.begin(115200);
  DEBUG_PRINTLN(F("\nStarting"));


/*
  DEBUG_PRINTLN("Reading EEprom Parameters");
  eep.read_string(EEPROM_SSID_OFFSET, ssid);
  DEBUG_PRINT("EEPROM_SSID: "); DEBUG_PRINTLN(ssid);
  eep.read_string(EEPROM_PASSWD_OFFSET, passwd);
  DEBUG_PRINT("EEPROM_PASSWD: "); DEBUG_PRINTLN(passwd);
  eep.read_string(EEPROM_SERVER_OFFSET, domo_server);
  DEBUG_PRINT("EEPROM_SERVER: "); DEBUG_PRINTLN(domo_server);
  eep.read_string(EEPROM_PORT_OFFSET, domo_port);
  DEBUG_PRINT("EEPROM_PORT: "); DEBUG_PRINTLN(domo_port);
  port = atoi(domo_port);
  DEBUG_PRINT("PORT: "); DEBUG_PRINTLN(port);
  eep.read_int(EEPROM_PORT_OFFSET, &port);
  DEBUG_PRINT("PORT directly from EE: "); DEBUG_PRINTLN(port);
  eep.read_int(EEPROM_IDX1_OFFSET, &idx);
  DEBUG_PRINT("IDx: "); DEBUG_PRINTLN(idx);
  eep.read_int(EEPROM_PORT_OFFSET, &sleep_time);
  DEBUG_PRINT("Sleep Time "); DEBUG_PRINTLN(sleep_time);
*/
/*
  pinMode(SETUP_PIN, INPUT_PULLUP);
  if (digitalRead(SETUP_PIN) == 0) {
    DEBUG_PRINTLN("Starting in Setup Mode");
    start_setup_server();
  } else  {
    DEBUG_PRINTLN("Starting in Normal Mode");
  }

#ifdef DS18B20_PIN
  for (i = 0; i < 2; i++) {
    if (update_ds18b20_temperature(&f)) {
      DEBUG_PRINT("DS temperature: "); DEBUG_PRINTLN(f);
    }
  }
#endif
*/
  if (domo.begin()) {
    DEBUG_PRINTLN(F("Connect OK"));
  } else {
    DEBUG_PRINTLN(F("Connect Fail"));
    
  }
  delay(100);
  if (domo.get_servertime(buff)) {
    DEBUG_PRINT(F("Server Time:")); DEBUG_PRINTLN(buff);
  }
  delay(100);  
  if (domo.update_temperature(7, 23.5)) {
    DEBUG_PRINTLN(F("Temp Sensor Updated"));
  }
  delay(100);
  if (domo.update_temperature(7, 13.5)) {
    DEBUG_PRINTLN(F("Temp Sensor Updated"));
  }

  while(1);

/*
  
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

  if (domo.update_temperature(7, 13.5)) {
    DEBUG_PRINTLN("Temp Sensor Updated");
  }

  DEBUG_PRINT("Vbat (V): "); DEBUG_PRINTLN(domo.vbat() );
  DEBUG_PRINT("Vbat (%): "); DEBUG_PRINTLN(domo.vbat_percentage() );
  DEBUG_PRINT("RSSI (bBm): "); DEBUG_PRINTLN(domo.rssi() );
  DEBUG_PRINT("RSSI (12level): "); DEBUG_PRINTLN(domo.rssi_12level() );
*/
#if ( SERVER_PORT > 0)
  server.begin();
  DEBUG_PRINTLN("Server started");
#endif
}




void loop() {

#if ( SERVER_PORT > 0)
  server_task();
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


/*
void start_setup_server()
{
  DEBUG_PRINT("Starting Setup Server on ");DEBUG_PRINTLN(SETUP_SSID);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SETUP_SSID);
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINTLN(WiFi.softAPIP());

  if (!MDNS.begin("esp8266")) {
    DEBUG_PRINTLN("Error MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  webserver.on ( "/", handleRoot );
  webserver.onNotFound ( handleNotFound );
  webserver.on ( "/reboot", []() {
    webserver.send ( 200, "text/plain", "<html><body><meta http-equiv='refresh' content='5;url=/' />Rebooting in 5 seconds</body></html>" );
    ESP.restart();
  } );
  webserver.begin();
  DEBUG_PRINTLN("TCP server started");
  MDNS.addService("http", "tcp", 80);
  while (1) {
    webserver.handleClient();
  }
}
*/
