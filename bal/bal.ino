#include "LowPower.h"

#ifdef USE_ENABLE_INTERRUPT
#include <EnableInterrupt.h>
#endif
#include "DallasTemperaturePu.h"
#include <EEPROM.h>


//#define OREGON_DEBUG
#define OREGON_SRC_INCLUDE
#include "oregon/oregon.hpp"

#define X10
#ifdef X10
#define X10_SRC_INCLUDE
#include "x10rf/x10rf.h"
#endif
#if defined __AVR_ATtiny85__
#undef DEBUG
#else
#define DEBUG
#endif
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
#define MAIL_HOUSE_CODE            'B'
#define MAIL_UNIT_CODE             1
#define TEMPERATURE_CHANNEL        0x01
#define TEMPERATURE_ID             0xBA

// Pins allocation
#define TX_POWER_PIN               4
#define TX_DATA_PIN                3
#if defined __AVR_ATtiny85__
#define DS18B20_PIN                0
#define FRONT_DOOR_PIN             1
#define FRONT_DOOR_PC              1
#else
#define DS18B20_PIN                5
#define FRONT_DOOR_PIN             6
#endif
#define BACK_DOOR_PIN              2
#define BACK_DOOR_PC               2

#define TEMPERATURE_WAKEUP_TIME    20 // Temperature update period in minutes
#define MIN_TIME_MAIL_REFRESH_MS   100
#define MIN_VCC_OK_MV              2800

#define EE_ADR_MAIL_METER_ID              0x00
#define EE_ADR_TEMPERATURE_CHANNEL        0x01
#define EE_ADR_TEMPERATURE_ID             0x02

#define WAKEUP_COUNT_TIME                 TEMPERATURE_WAKEUP_TIME*60/8


DallasTemperature DS18B20(DS18B20_PIN);
x10rf myx10 = x10rf(TX_DATA_PIN, 0, 5);
Oregon rf_sender = Oregon();
bool mail_update_request = true;
uint16_t wakeup_count = WAKEUP_COUNT_TIME;
uint16_t count_mail = 0;
unsigned long time;
unsigned long time_last_mail_update = 0;
uint8_t mail_meter_id = MAIL_METER_ID;
char mail_house_code = MAIL_HOUSE_CODE;
char mail_unit_code = MAIL_UNIT_CODE;

uint8_t temperature_channel = TEMPERATURE_CHANNEL;
uint8_t temperature_id = TEMPERATURE_ID;

#define NOTIFY_BACK 3
#define NOTIFY_FRONT 2


void notify(uint8_t cnt)
{
  uint8_t i;
  for (i = 0; i < cnt; i++) {
    radio_power_on();
    delay(500);
     radio_power_off();
    delay(500);
  }
}

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
    return false;
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
  return true;
}

void send_mail_count_update(void )
{
  DEBUG_PRINT(F("Mail update : "));
  DEBUG_PRINTLN(count_mail);
  radio_power_on();
  // meter update
  myx10.send_meter(mail_meter_id, count_mail);
  // switch update
  if (count_mail > 0) {
    myx10.send_switch(mail_house_code, mail_unit_code, ON);
  } else {
    myx10.send_switch(mail_house_code, mail_unit_code, OFF);
  }
  radio_power_off();
  time_last_mail_update = millis();
}

void front_door_interupt() {
  DEBUG_PRINTLN(F("Front Door ISR"));
  time = millis();
  notify(NOTIFY_FRONT);
  //if ( abs(time - time_last_mail_update) > MIN_TIME_MAIL_REFRESH_MS ) {
  {
    count_mail++;
    mail_update_request = true;
  }
#ifdef USE_ENABLE_INTERRUPT
  disableInterrupt(BACK_DOOR_PIN);
#else

#endif
}
void back_door_interupt() {
  DEBUG_PRINTLN(F("Back Door ISR"));
  time = millis();
 
  // if ( abs(time - time_last_mail_update) > MIN_TIME_MAIL_REFRESH_MS ) {
  {
    count_mail = 0;
    mail_update_request = true;
  }
#ifdef USE_ENABLE_INTERRUPT
  disableInterrupt(BACK_DOOR_PIN);
#else

#endif

}

ISR (PCINT0_vect)
{
  delay(100);
  if (digitalRead(BACK_DOOR_PIN) == 0) {
    notify(NOTIFY_BACK);
    count_mail = 0;
    mail_update_request = true;
    return;
  }
  if (digitalRead(FRONT_DOOR_PIN) == 1) {
     notify(NOTIFY_FRONT);
    count_mail++;
    mail_update_request = true;
    return;
  }
}

void setup()
{
  uint8_t i;
#ifdef DEBUG
  DEBUG_PRINTER.begin(9600);
  DEBUG_PRINTLN(F("Starting"));
#endif
  i = EEPROM.read(EE_ADR_MAIL_METER_ID);
  if (i != 0xFF) {
    mail_meter_id = i;
  }
  i = EEPROM.read(EE_ADR_TEMPERATURE_CHANNEL);
  if (i != 0xFF) {
    temperature_channel = i;
  }
  i = EEPROM.read(EE_ADR_TEMPERATURE_ID);
  if (i != 0xFF) {
    temperature_id = i;
  }

  // Init interupt pins detection
  // Back door mail box detection is micro swtich Normaly Open
  // Opening the door will close the switch
  pinMode( BACK_DOOR_PIN  , INPUT_PULLUP );
#ifdef USE_ENABLE_INTERRUPT
  enableInterrupt(BACK_DOOR_PIN, back_door_interupt, FALLING);
#else
  PCMSK = (1 << BACK_DOOR_PC) | (1 << FRONT_DOOR_PC);
  GIMSK = (1 << PCIE);
#endif
  // Front door mail box detection is micro reelswtich Normaly Close
  // Opening the door will open the switch
  pinMode( FRONT_DOOR_PIN  , INPUT ); //! Caution no internal pull up, external one only !
#ifdef USE_ENABLE_INTERRUPT
  enableInterrupt(FRONT_DOOR_PIN, front_door_interupt, RISING);
#endif
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
  int16_t raw_temperature;
  byte state;
  bool battery_status;

  if (mail_update_request) {
    send_mail_count_update();
    mail_update_request = false;
    //delay(2 * MIN_TIME_MAIL_REFRESH_MS);
#ifdef USE_ENABLE_INTERRUPT

    enableInterrupt(BACK_DOOR_PIN, back_door_interupt, FALLING);
    enableInterrupt(FRONT_DOOR_PIN, front_door_interupt, RISING);


#else

#endif
  }
  if (++wakeup_count >= WAKEUP_COUNT_TIME) {
    if (getVCC() >= MIN_VCC_OK_MV) {
      battery_status = true;
    } else {
      battery_status = false;
    }
    radio_power_on();
    rf_sender.begin(TX_DATA_PIN);
    if ( update_ds18b20_temperature_raw(&raw_temperature) ) {
      rf_sender.send_temperature_from_ds18(temperature_channel, temperature_id, raw_temperature, battery_status);
    }
    wakeup_count = 0;
    radio_power_off();
  }

  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF) ;
}



int getVCC() {
#if defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85
#elif defined(__AVR_ATmega1284P__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega1284
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
#endif
  delay(2);
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  uint8_t low = ADCL;
  unsigned int val = (ADCH << 8) | low;
  //discard previous result
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  low = ADCL;
  val = (ADCH << 8) | low;
  return ((long)1024 * 1100) / val;
}

/*

     myx10.send_switch('B', 4, ON);
  delay(100);
  DEBUG_PRINTLN("Tx Meter info");
  myx10.send_meter(12, 75123);
*/


