#include "LowPower.h"
#include <EnableInterrupt.h>
#include "DallasTemperaturePu.h"


//#define OREGON_DEBUG
#define OREGON_SRC_INCLUDE
#include "oregon/oregon.hpp"

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

#define MAIL_METER_ID              12
#define TEMPERATURE_CHANNEL        0x01
#define TEMPERATURE_ID             0xBA
                                
#define TX_POWER_PIN               4
#define TX_DATA_PIN                3
#define DS18B20_PIN                0
#define TEMPERATURE_WAKEUP_TIME    5
#define FRONT_DOOR_PIN             1
#define BACK_DOOR_PIN              2
#define MIN_TIME_MAIL_REFRESH_MS   100

#define WAKEUP_COUNT_TIME          TEMPERATURE_WAKEUP_TIME*60/8

DallasTemperature DS18B20(DS18B20_PIN);
x10rf myx10 = x10rf(TX_DATA_PIN, 0, 5);
Oregon rf_sender = Oregon();
bool wakeup = true;
uint8_t wakeup_count = 0;
uint16_t count_mail = 0;
unsigned long time;
unsigned long time_last_mail_update = 0;

void radio_power_on(void)
{
  digitalWrite( TX_POWER_PIN , HIGH );
  digitalWrite( TX_DATA_PIN , HIGH );
}

void radio_power_off(void)
{
  digitalWrite( TX_POWER_PIN , LOW );
  digitalWrite( TX_DATA_PIN , LOW );
}

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
  DEBUG_PRINT(F("DS18B20 Temperature (1/128C): ")); DEBUG_PRINTLN(t);
  *temp = t;
}

void send_mail_count_update(void )
{
  radio_power_on();
  myx10.send_meter(MAIL_METER_ID, count_mail);
  radio_power_off();
  time_last_mail_update = millis();
}

void front_door_interupt() {
  time = millis();
  if ( abs(time-time_last_mail_update) > MIN_TIME_MAIL_REFRESH_MS ) { 
    count_mail++;
    send_mail_count_update();
  }
}

void back_door_interupt() {
  time = millis();
  if ( abs(time-time_last_mail_update) > MIN_TIME_MAIL_REFRESH_MS ) { 
    count_mail = 0;
    send_mail_count_update();
  }
}


void setup()
{
  uint8_t i;
  
  // Init interupt pins detection 
  // Back door letter box detection is micro swtich Normaly Open 
  // Opening the door will close the switch
  pinMode( BACK_DOOR_PIN  , INPUT_PULLUP );
  enableInterrupt(BACK_DOOR_PIN, back_door_interupt, FALLING);
  // Front door letter box detection is micro reelswtich Normaly Close
  // Opening the door will open the switch
  pinMode( FRONT_DOOR_PIN  , INPUT ); //! Caution no internal pull up, external one only !
  enableInterrupt(FRONT_DOOR_PIN, front_door_interupt, RISING);

  // Init Radio Vcc pin
  digitalWrite( TX_POWER_PIN , LOW );
  pinMode( TX_POWER_PIN  , OUTPUT );
  // Notify Startup
  for (i = 0; i < 10; i++) {
    radio_power_on();
    delay(100);
    radio_power_off();
    delay(100);
  }
  myx10.begin();
}

void loop()
{
  int16_t temp;
  byte state;

  if (++wakeup_count >= WAKEUP_COUNT_TIME) {
    wakeup = true;
  }

  if (wakeup) {
    radio_power_on();
    rf_sender.begin(TX_DATA_PIN);
#if 1
    update_ds18b20_temperature_raw(&temp);
    rf_sender.send_temperature_from_ds18(TEMPERATURE_CHANNEL, TEMPERATURE_ID, temp, 1);
#else
    delay(1000);
#endif
    wakeup = false;
    wakeup_count = 0;
    radio_power_off();
  }
  delay(MIN_TIME_MAIL_REFRESH_MS);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF) ;
}

/*

     myx10.send_switch('B', 4, ON);
  delay(100);
  DEBUG_PRINTLN("Tx Meter info");
  myx10.send_meter(12, 75123);
*/


