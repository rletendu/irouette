#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <ArduinoOTA.h>



const char* ssid = "domo";
const char* password = "pulsar est content";



// 39bits codes
#define BAR_ON_OFF    0xCA31DA45
#define BAR_VOL_PLUS  0xCF2F9DAB
#define BAR_VOL_MINUS 0xB2BBAC69
#define BAR_VOL_MUTE  0x123CD34B
#define BAR_AUX       0x2973A32A


//20bits
#define TV_AMBILIGHT      0x1008F
#define TV_AMBILIGHT_ALT  0x0008F

#define TC_ON_OFF     0x1000C
#define TV_VOL_PLUS   0x10010
#define TV_VOL_MINUS  0x10011

// e.g. D5 on a NodeMCU board.14
#define IR_RX_PIN D5
#define RELAY_PIN     D1

#define GND_TX_PIN    D2
#define IR_TX_PIN     D3

bool light_state = false;


// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(IR_RX_PIN, 256, 15, true);
IRsend irsend(IR_TX_PIN);
WiFiServer server(80);

decode_results results;  // Somewhere to store the results

unsigned long last_cmd, current;

void client_task(void);


void write_relay(bool state) {
  digitalWrite(RELAY_PIN, state);
}



void toggle_ambilight(void)
{
  light_state = !light_state;
  write_relay(light_state);
}

void ambilight(bool state)
{
  light_state = state;
  write_relay(light_state);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.printf("TV IR devcoder \n");
  irrecv.enableIRIn();  // Start the receiver
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GND_TX_PIN, OUTPUT);
  pinMode(IR_TX_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }



  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned
  int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  server.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  client_task();
  if (irrecv.decode(&results)) {
    if (results.value == TV_AMBILIGHT_ALT || results.value == TV_AMBILIGHT) {
      current = millis();
      if (abs(last_cmd - current) > 1000) {
        last_cmd = current;
        Serial.printf("TOGGLE!! \n");
        toggle_ambilight();
      }
    }
    else if (results.value == 0xCA31DA45) {
      Serial.printf("SOUND!! \n");
    }
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
  if (req.indexOf("/ambilight_on") != -1)
    ambilight(true);
  else if (req.indexOf("/ambilight_off") != -1)
    ambilight(false);
  else if (req.indexOf("/reset") != -1)
    ESP.restart();
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }
  client.flush();
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nAmbilight Access OK ";
  s += "</html>\n";
  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
}
