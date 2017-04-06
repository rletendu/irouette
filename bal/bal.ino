#include "LowPower.h"

//#define OREGON_DEBUG
#define OREGON_SRC_INCLUDE
#include "oregon/oregon.hpp"


#include <EnableInterrupt.h>


#define X10
#ifdef X10
#define X10_SRC_INCLUDE
#include "x10rf/x10rf.h"
#endif

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
#define DOOR_PIN  1

#include "DallasTemperaturePu.h"
DallasTemperature DS18B20(DS18B20_PIN);


x10rf myx10 = x10rf(TX_PIN, 0, 5);
Oregon rf_sender = Oregon();
bool wakeup = true;
uint8_t wakeup_count = 0;
uint16_t count_mail = 0;



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

void interruptFunction() {
  digitalWrite( TX_POWER , HIGH );
   digitalWrite( TX_PIN , HIGH );

  count_mail++;
  myx10.send_meter(12, count_mail);
  digitalWrite( TX_POWER , LOW );
   digitalWrite( TX_PIN , LOW );
}


void setup()
{

  pinMode( DOOR_PIN  , INPUT_PULLUP );

  enableInterrupt(DOOR_PIN, interruptFunction, FALLING);

  digitalWrite( TX_POWER , LOW );
  pinMode( TX_POWER  , OUTPUT );

  digitalWrite( TX_POWER , HIGH );
  delay(100);
  digitalWrite( TX_POWER , LOW );
  delay(100);
  digitalWrite( TX_POWER , HIGH );
  delay(100);
  digitalWrite( TX_POWER , LOW );
  myx10.begin();

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

    rf_sender.begin(TX_PIN);
#if 1
    update_ds18b20_temperature_raw(&temp);
    rf_sender.send_temperature_from_ds18(0x20, 0xBA, 0, 1);
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

/*

     myx10.send_switch('B', 4, ON);
  delay(100);
  DEBUG_PRINTLN("Tx Meter info");
  myx10.send_meter(12, 75123);
*/


