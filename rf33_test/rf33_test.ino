#define OREGON_DEBUG
#include "oregon.hpp"

#define TX_PIN 1
Oregon rf_sender = Oregon();

void setup()
{
  Serial.begin(9600);
  rf_sender.begin(TX_PIN);

}

void loop()
{
  rf_sender.send_temperature(0x20, 0xCB, 11.2, 1);
  delay(3000);
  rf_sender.send_temperature_hum(0x20, 0xCB, 11.2, 52,1);
  delay(3000);
}
