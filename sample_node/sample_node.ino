#include <ESP8266WiFi.h>
#include "config.h"
#include "domoticz/domoticz.h"
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

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

ESP8266WebServer webserver ( 80 );
WiFiServer server(80);

#if ( SERVER_PORT > 0)

void server_task(void);
#endif



char buff[100];
char name_buff[20];
char sw_status[20];
char ssid[EEPROM_STR_SIZE];
char passwd[EEPROM_STR_SIZE];
char domo_server[EEPROM_STR_SIZE];
char domo_port[EEPROM_STR_SIZE];


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
  float t, t2;
  oneWire.reset();
  DS18B20.begin();
  DS18B20.setWaitForConversion(true);
  DS18B20.requestTemperatures();
  if (DS18B20.getDeviceCount() > 1 ) {
    t = DS18B20.getTempCByIndex(0);
    t2 = DS18B20.getTempCByIndex(1);
    DEBUG_PRINT("DS18B20 Temperature 2: "); DEBUG_PRINTLN(t2, 2);
  } else {
    t = DS18B20.getTempCByIndex(0);
  }
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

void read_eeprom_string(char *str, uint16_t offset)
{
  uint8_t i;
  for (i = 0; i < EEPROM_STR_SIZE; i++) {
    str[i] =  char(EEPROM.read(i + offset));
  }
}

void write_eeprom_string(char *str, uint16_t offset)
{
  uint8_t i;
  for (i = 0; i < EEPROM_STR_SIZE; i++) {
    EEPROM.write(i + offset, str[i]);
  }
  EEPROM.commit();
}

int read_eeprom_int( uint16_t offset)
{
  char tmp[EEPROM_STR_SIZE];
  read_eeprom_string(tmp, offset);
  if (strlen(tmp)) {
    return atoi(tmp);
  } else {
    return 0;
  }
}

void write_eeprom_int( uint16_t offset, int value)
{
  char tmp[EEPROM_STR_SIZE];
  snprintf(tmp, EEPROM_STR_SIZE, "%d", value);
  write_eeprom_string(tmp, offset);
}

float read_eeprom_float( uint16_t offset)
{
  char tmp[EEPROM_STR_SIZE];
  read_eeprom_string(tmp, offset);
  if (strlen(tmp)) {
    return atof(tmp);
  } else {
    return 0;
  }
}

void write_eeprom_float( uint16_t offset, float value)
{
  char tmp[EEPROM_STR_SIZE];
  dtostrf(value, 3, 1, tmp);
  write_eeprom_string(tmp, offset);
}



void setup()
{
  float f;
  uint8_t h;
  uint16_t p;
  uint8_t i;
  int port;
  float vbat_min;

  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  DEBUG_PRINTLN("\nStarting");
  DEBUG_PRINT("Rcause : "); DEBUG_PRINTLN(ESP.getResetReason());
  const rst_info * resetInfo = system_get_rst_info();
  DEBUG_PRINT("Rcause : "); DEBUG_PRINTLN(resetInfo->reason);

  DEBUG_PRINTLN("Reading EEprom Parameters");
  read_eeprom_string(ssid, EEPROM_SSID_OFFSET);
  DEBUG_PRINT("EEPROM_SSID: "); DEBUG_PRINTLN(ssid);
  read_eeprom_string(passwd, EEPROM_PASSWD_OFFSET);
  DEBUG_PRINT("EEPROM_PASSWD: "); DEBUG_PRINTLN(passwd);
  read_eeprom_string(domo_server, EEPROM_SERVER_OFFSET);
  DEBUG_PRINT("EEPROM_SERVER: "); DEBUG_PRINTLN(domo_server);
  read_eeprom_string(domo_port, EEPROM_PORT_OFFSET);
  DEBUG_PRINT("EEPROM_PORT: "); DEBUG_PRINTLN(domo_port);
  port = atoi(domo_port);
  DEBUG_PRINT("PORT: "); DEBUG_PRINTLN(port);
  DEBUG_PRINT("PORT directly from EE: "); DEBUG_PRINTLN(read_eeprom_int(EEPROM_PORT_OFFSET));
  DEBUG_PRINT("Vbat min: "); DEBUG_PRINTLN(read_eeprom_float(EEPROM_VBAT_MIN_OFFSET));

  DEBUG_PRINTLN("Test Writing EEprom Parameters");
  /*
    write_eeprom_string("TestSSID", EEPROM_SSID_OFFSET);
    write_eeprom_string("my_password", EEPROM_PASSWD_OFFSET);
    write_eeprom_string("192.168.1.200", EEPROM_SERVER_OFFSET);
    write_eeprom_int(EEPROM_PORT_OFFSET, read_eeprom_int(EEPROM_PORT_OFFSET) + 1);
  */
  write_eeprom_float(EEPROM_VBAT_MIN_OFFSET, read_eeprom_float(EEPROM_VBAT_MIN_OFFSET) + 1.1);
  //write_eeprom_string("8080", EEPROM_PORT_OFFSET);

#ifdef DS18B20_PIN
  for (i = 0; i < 2; i++) {
    if (update_ds18b20_temperature(&f)) {
      DEBUG_PRINT("DS temperature: "); DEBUG_PRINTLN(f);
    }
  }
#endif
  start_setup();

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
  DEBUG_PRINT("Sleeping for "); DEBUG_PRINT(DEEP_SLEEP_TIME); DEBUG_PRINTLN(" min");
  delay(1000);
  DEBUG_PRINTER.flush();
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

void handleRoot() {
  String message;
  char tmp[EEPROM_STR_SIZE];

  for ( uint8_t i = 0; i < webserver.args(); i++ ) {
    webserver.arg ( i ).toCharArray(tmp, EEPROM_STR_SIZE );
    if (webserver.argName (i) == String("ssid")) {
      DEBUG_PRINT("SSID : "); DEBUG_PRINTLN(tmp);
      write_eeprom_string(tmp, EEPROM_SSID_OFFSET);
    } else if (webserver.argName (i) == String("pass")) {
      DEBUG_PRINT("PASS : "); DEBUG_PRINTLN(tmp);
      write_eeprom_string(tmp, EEPROM_PASSWD_OFFSET);
    } else if (webserver.argName (i) == String("server")) {
      DEBUG_PRINT("Server : "); DEBUG_PRINTLN(tmp);
      write_eeprom_string(tmp, EEPROM_SERVER_OFFSET);
    } else if (webserver.argName (i) == String("port")) {
      DEBUG_PRINT("Port : "); DEBUG_PRINTLN(tmp);
      write_eeprom_string(tmp, EEPROM_PORT_OFFSET);
    }
  }
  
  message = "<html><body style='background-color:powderblue;'><h1>Domoticz Node Configurator</h1>";
  message += "<meta http-equiv='pragma' content='no-cache'/>";
  message += "<p>";
  message += String("MAC: ") + String(WiFi.macAddress().c_str());
  message += "<form method='get' action='/'>";
  read_eeprom_string(tmp, EEPROM_SSID_OFFSET);
  message += "<label>SSID: </label><input name='ssid' length=32 value='" + String(tmp) + "'><br>";
  read_eeprom_string(tmp, EEPROM_PASSWD_OFFSET);
  message += "<label>PASSWD: </label><input name='pass' length=32 value='" + String(tmp) + "'><br>";
  read_eeprom_string(tmp, EEPROM_SERVER_OFFSET);
  message += "<label>SERVER: </label><input name='server' length=32 value='" + String(tmp) + "'><br>";
  read_eeprom_string(tmp, EEPROM_PORT_OFFSET);
  message += "<label>PORT: </label> <input name ='port' length=32 value = '" + String(tmp) + "' ><br>";
  message += "<input type='submit'></form>";
  message += "</body></html>\r\n\r\n";
  webserver.send ( 200, "text/html", message );
}

void handleNotFound() {
  DEBUG_PRINTLN(webserver.uri());
  String message = "Not Found\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += ( webserver.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";
  for ( uint8_t i = 0; i < webserver.args(); i++ ) {
    message += " " + webserver.argName ( i ) + ": " + webserver.arg ( i ) + "\n";
  }
  webserver.send ( 404, "text/plain", message );
}

void handlessid() {
  DEBUG_PRINTLN(webserver.uri());
  String message = "SSID updated ! \n\n";
  for ( uint8_t i = 0; i < webserver.args(); i++ ) {
    message += " " + webserver.argName ( i ) + ": " + webserver.arg ( i ) + "\n";
    if (webserver.argName (i) == String("ssid")) {
      DEBUG_PRINT("SSID : "); DEBUG_PRINTLN(webserver.arg ( i ));
    } else if (webserver.argName (i) == String("pass")) {
      DEBUG_PRINT("PASS : "); DEBUG_PRINTLN(webserver.arg ( i ));
    }
  }
  webserver.send ( 200, "text/plain", message );
}

void start_setup()
{
  DEBUG_PRINT("Starting Setup Server on ")DEBUG_PRINTLN("SETUP_SS");
  WiFi.mode(WIFI_AP); // Update from original
  WiFi.softAP(SETUP_SSID);
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINTLN(WiFi.softAPIP());
  DEBUG_PRINTLN("TCP server started");
  // MDNS.addService("http", "tcp", 80);
  webserver.on ( "/", handleRoot );
  webserver.on ( "/ssid", handlessid );
  webserver.onNotFound ( handleNotFound );
  webserver.on ( "/reboot", []() {
    webserver.send ( 200, "text/plain", "Rebooting !" );
    ESP.restart();
  } );
  webserver.begin();
  while (1) {
    webserver.handleClient();
  }
}

