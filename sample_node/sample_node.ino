#include <ESP8266WiFi.h>
#include "config.h"
#include "domoticz/domoticz.h"


Domoticz domo = Domoticz();
WiFiServer server(80);
void client_task(void);

ADC_MODE(ADC_VCC);
char buff[100];
char name_buff[20];

void setup()
{
  float f;
  uint8_t h;
  Serial.begin(9600);
  DEBUG_PRINTLN("Starting");
  DEBUG_PRINT("Esp Vcc: "); DEBUG_PRINTLN(ESP.getVcc());

  if (domo.begin()) {
    DEBUG_PRINTLN("Connect OK");
  } else {
    DEBUG_PRINTLN("Connect Fail");
    ESP.restart();
  }

  server.begin();
  DEBUG_PRINTLN("Server started");

  if (domo.get_servertime(buff)) {
    DEBUG_PRINT("Server Time:"); DEBUG_PRINTLN(buff);
  }
  if (domo.get_temperature(10, &f, name_buff)) {
    DEBUG_PRINT("Temperature Garage:"); DEBUG_PRINTLN(f);
  }
  if (domo.get_temperature(3, &f, name_buff)) {
    DEBUG_PRINT("Temperature Congelateur:"); DEBUG_PRINTLN(f);
  }
}

void loop() {

  client_task();
}




void client_task(void)
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
  if (req.indexOf("/relay") != -1) {
    DEBUG_PRINTLN("Relay Request");
  }
  else if (req.indexOf("/reset") != -1) {
    DEBUG_PRINTLN("Reset Request");
    ESP.restart();
  }
  else {
    DEBUG_PRINTLN("invalid request");
    client.stop();
    return;
  }
  client.flush();
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nDoor Garage Activation OK ";
  s += "</html>\n";
  // Send the response to the client
  client.print(s);
  delay(1);
  DEBUG_PRINTLN("Client disonnected");
}


/*

{
   "ActTime" : 1475089965,
   "ServerTime" : "2016-09-28 21:12:45",
   "Sunrise" : "08:00",
   "Sunset" : "19:48",
   "result" : [
      {
         "AddjMulti" : 1.0,
         "AddjMulti2" : 1.0,
         "AddjValue" : -7.0,
         "AddjValue2" : 0.0,
         "BatteryLevel" : 255,
         "CustomImage" : 0,
         "Data" : "21.9 C, 60 %",
         "Description" : "",
         "DewPoint" : "13.79",
         "Favorite" : 0,
         "HardwareID" : 3,
         "HardwareName" : "Virtual",
         "HardwareType" : "Dummy (Does nothing, use for virtual switches only)",
         "HardwareTypeVal" : 15,
         "HaveTimeout" : false,
         "Humidity" : 60,
         "HumidityStatus" : "Normal",
         "ID" : "14059",
         "LastUpdate" : "2016-09-28 21:07:20",
         "Name" : "Garage",
         "Notifications" : "false",
         "PlanID" : "0",
         "PlanIDs" : [ 0 ],
         "Protected" : false,
         "ShowNotifications" : true,
         "SignalLevel" : "-",
         "SubType" : "THGN122/123, THGN132, THGR122/228/238/268",
         "Temp" : 21.90,
         "Timers" : "false",
         "Type" : "Temp + Humidity",
         "TypeImg" : "temperature",
         "Unit" : 1,
         "Used" : 1,
         "XOffset" : "0",
         "YOffset" : "0",
         "idx" : "10"
      }
   ],
   "status" : "OK",
   "title" : "Devices"
}
*/

