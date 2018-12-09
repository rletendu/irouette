#include "LowPower.h"

//#define OREGON_DEBUG
#define OREGON_SRC_INCLUDE
#include "oregon/oregon.hpp"



//#define DEBUG

#ifdef DEBUG
#ifndef DEBUG_PRINTER
#define DEBUG_PRINTER Serial
#endif
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif

#define TX_POWER 4
#define TX_PIN   3
#define DS18B20_PIN 0
#define WAKEUP_TIME 5*60/8

#include "DallasTemperaturePu.h"
DallasTemperature DS18B20(DS18B20_PIN);


bool update_ds18b20_temperature_raw(int *temp)
{
  DeviceAddress deviceAddress;
  uint16_t t;
  //oneWire.reset();
  DS18B20.begin();
  DS18B20.setWaitForConversion(true);
  DS18B20.requestTemperatures();
  if ( DS18B20.getDeviceCount() == 0) {
    DEBUG_PRINTLN("No Sensor Found");
  }
  DS18B20.getAddress(deviceAddress, 0);

  t = DS18B20.getTemp((uint8_t*)deviceAddress);
  pinMode(DS18B20_PIN, OUTPUT);
  digitalWrite(DS18B20_PIN, LOW);
  if (t == 0xFFFF) {
    DEBUG_PRINTLN("Failed reading DS18B20!");
    return false;
  }
  DEBUG_PRINT("DS18B20 Temperature (1/128C): "); DEBUG_PRINTLN(t);
  *temp = t;
}

Oregon rf_sender = Oregon();
bool wakeup = true;
uint8_t wakeup_count = 0;


void setup()
{
  digitalWrite( TX_POWER , LOW );
  pinMode( TX_POWER  , OUTPUT );

  digitalWrite( TX_POWER , HIGH );
  delay(100);
  digitalWrite( TX_POWER , LOW );
  delay(100);
  digitalWrite( TX_POWER , HIGH );
  delay(100);
  digitalWrite( TX_POWER , LOW );
  ;

}

void loop()
{

  int16_t temp;
  byte state;
  if (++wakeup_count >= WAKEUP_TIME) {
    wakeup = true;
  }
  if (wakeup) {
        digitalWrite( TX_PIN , HIGH );
    digitalWrite( TX_POWER , HIGH );
#if 1
    rf_sender.begin(TX_PIN);

    update_ds18b20_temperature_raw(&temp);
    rf_sender.send_temperature_from_ds18(0x20, 0xCB, temp, 1);
#else
    delay(1000);
#endif
    wakeup = false;
    wakeup_count = 0;

    digitalWrite( TX_POWER , LOW );
    digitalWrite( TX_PIN , LOW );
  }

  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF) ;


}


