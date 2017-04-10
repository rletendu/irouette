
#include "config.h"
#include "domoticz/domoticz.h"
#include <avr/wdt.h>

#define SD_PIN 4


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
#if SERVER_PORT !=0
EthernetServer server = EthernetServer(80);
#endif

char buff[20];
/*
  char name_buff[20];
  char sw_status[20];
*/
int  sleep_time;
int  idx;

void software_Reboot()
{
  wdt_enable(WDTO_15MS);
  while (1)
  {

  }
}


void setup()
{
  float f;
  uint8_t h;
  uint16_t p;
  uint8_t i;
  int port;
  float vbat_min;

  pinMode(SD_PIN, OUTPUT);
  digitalWrite(SD_PIN, 1);


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
  domo.send_log_message("Done from AVR ethernet Node!");
  float t = 10.0;



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
  DEBUG_PRINTLN(F("Server started"));
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
  EthernetClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  DEBUG_PRINTLN(F("new client"));
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  DEBUG_PRINTLN(req);
  client.flush();

  // Match the request
  if (req.indexOf(F("/relay")) != -1) {
    DEBUG_PRINTLN(F("Relay Request"));
  }
  else if (req.indexOf(F("/reset")) != -1) {
    DEBUG_PRINTLN(F("Reset Request"));
    software_Reboot();
    //ESP.restart();
  }
  else {
    DEBUG_PRINTLN(F("invalid request"));
    client.stop();
    return;
  }
  client.flush();
  //String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE HTML> \r\n<html>\r\nDoor Garage Activation OK</html>\n";

  // Send the response to the client
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE HTML> \r\n<html>\r\nDoor Garage Activation OK</html>\n"));
  delay(1);
  DEBUG_PRINTLN(F("Client disconnected"));
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

#if 0



const char *tests[] = {
  "", // no parameters
  "param1=test", // simple test
  "param1=test&param2=test2", // two parameters
  "param1=test&param2=test+2", // parameter with an encoded space
  "param1=test&param2=c%3A%5Cfoodir%5Cbarfile.fil", // percent encoding
  "p1=1&p2=2&p3=3&p4=4&p5=5&p6=6&p7=7&p8=8" // more params than our test will acommodate
};

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

}

void loop() {
  char buf[100];
  char *params[5][2];

  delay(5000);

  for (int i = 0; i < sizeof(tests) / sizeof(*tests); i++) {
    Serial.print("parsing \"");
    Serial.print(tests[i]);

    // copy test[i] into the buffer
    // because the parser overwrites what is i the string it is passed.
    strcpy(buf, tests[i]);

    // parse the buffer into params[][]
    int resultsCt = parseUrlParams(buf, params, 5, true);

    // print off the results;

    Serial.print("\" produced ");
    Serial.print(resultsCt);
    Serial.print(" parameters:");
    Serial.println();

    for (int i = 0; i < resultsCt; i++) {
      Serial.print("param ");
      Serial.print(i);
      Serial.print(" name \"");
      Serial.print( params[i][0]);
      Serial.print("\", param \"");
      Serial.print( params[i][1]);
      Serial.print("\".");
      Serial.println();
    }
    Serial.println();
  }
}

/**
   queryString: the string with is to be parsed.
   WARNING! This function overwrites the content of this string. Pass this function a copy
   if you need the value preserved.
   results: place to put the pairs of param name/value.
   resultsMaxCt: maximum number of results, = sizeof(results)/sizeof(*results)
   decodeUrl: if this is true, then url escapes will be decoded as per RFC 2616
*/

int parseUrlParams(char *queryString, char *results[][2], int resultsMaxCt, boolean decodeUrl) {
  int ct = 0;

  while (queryString && *queryString && ct < resultsMaxCt) {
    results[ct][0] = strsep(&queryString, "&");
    results[ct][1] = strchrnul(results[ct][0], '=');
    if (*results[ct][1]) *results[ct][1]++ = '\0';

    if (decodeUrl) {
      percentDecode(results[ct][0]);
      percentDecode(results[ct][1]);
    }

    ct++;
  }

  return ct;
}

/**
   Perform URL percent decoding.
   Decoding is done in-place and will modify the parameter.
*/

void percentDecode(char *src) {
  char *dst = src;

  while (*src) {
    if (*src == '+') {
      src++;
      *dst++ = ' ';
    }
    else if (*src == '%') {
      // handle percent escape

      *dst = '\0';
      src++;

      if (*src >= '0' && *src <= '9') {
        *dst = *src++ - '0';
      }
      else if (*src >= 'A' && *src <= 'F') {
        *dst = 10 + *src++ - 'A';
      }
      else if (*src >= 'a' && *src <= 'f') {
        *dst = 10 + *src++ - 'a';
      }

      // this will cause %4 to be decoded to ascii @, but %4 is invalid
      // and we can't be expected to decode it properly anyway

      *dst <<= 4;

      if (*src >= '0' && *src <= '9') {
        *dst |= *src++ - '0';
      }
      else if (*src >= 'A' && *src <= 'F') {
        *dst |= 10 + *src++ - 'A';
      }
      else if (*src >= 'a' && *src <= 'f') {
        *dst |= 10 + *src++ - 'a';
      }

      dst++;
    }
    else {
      *dst++ = *src++;
    }

  }
  *dst = '\0';
}





// Example GET line: GET /?foo=bar HTTP/1.1
void processGet (const char * data)
{
  // find where the parameters start
  const char * paramsPos = strchr (data, '?');
  if (paramsPos == NULL)
    return;  // no parameters
  // find the trailing space
  const char * spacePos = strchr (paramsPos, ' ');
  if (spacePos == NULL)
    return;  // no space found
  // work out how long the parameters are
  int paramLength = spacePos - paramsPos - 1;
  // see if too long
  if (paramLength >= MAX_PARAM)
    return;  // too long for us
  // copy parameters into a buffer
  char param [MAX_PARAM];
  memcpy (param, paramsPos + 1, paramLength);  // skip the "?"
  param [paramLength] = 0;  // null terminator

  // do things depending on argument (GET parameters)

  if (strcmp (param, "foo") == 0)
    Serial.println (F("Activating foo"));
  else if (strcmp (param, "bar") == 0)
    Serial.println (F("Activating bar"));

}  // end of processGet




#endif
