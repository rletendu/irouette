#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include "ds3231.h"


#define SERIAL_DEBUG
//#define DEBUG_SENSOR_ONLY
//#define FORCE_NIGHT_MODE
//#define FORCE_FREEZE


#define RADIO_BAUD               9600

#define BUFF_MAX                 200
#define RTC_MAX_DEVIATION        5       // The maximum time deviation in minutes before adjusting RTC time versus server

#define PARAM_EE_ADDR            0
#define VALID_SIG                0x55AA
#define DEFAULT_NIGHT_MODE_LUM   523
#define DEFAULT_SLEEP_PERIOD     2
#define DEFAULT_SUNSET_TIME      22*60
#define DEFAULT_SUNRISE_TIME     8*60
#define DEFAULT_RPM_MEASURE_TIME 5
#define DEFAULT_VBAT_LIGHT_MIN   3.5    // The minimal Vcc Level to keep light active
#define DEFAULT_VBAT_RADIO_MIN   3.3 
#define DEFAULT_RPM_2_MS         1.25

#define TIMEOUT_CONNECT          20*1000
#define TIMEOUT_TX               20*1000
#define TIMEOUT_RX               20*1000


#define HEADING_MECHANICAL_CORRECTION (float)((-62)*PI/180)

enum OperatingMode {
  DAY = 0,
  NIGHT_LIGHT_ON = 1,
  NIGHT_LIGHT_OFF = 2,
};

enum ChargeState {
  NA = 0,
  CHARGING = 1,
  CHARGED = 2,
  NOT_CHARGING = 3,
};

struct SensorValues {
  float    temp_ext;
  float    temp_int;
  float    pressure;
  uint16_t lum;
  float    humidity;
  uint16_t rain;
  float    vbat;
  uint16_t wind_dir;
  uint16_t wind_speed;
};

struct ParamValues {
  uint16_t  valid_sig;
  uint16_t   night_mode_lum;
  uint8_t   sleep_period;
  uint16_t  sunset_time;
  uint16_t  sunrise_time;
  uint8_t   rpm_measure_time;
  float     vcc_light_min;
  float     vcc_radio_min;
  float     rpm_2_ms;
};



// Define where debug output will be printed.
#define DEBUG_PRINTER SoftSerialDebug

// Setup debug printing macros.
#ifdef SERIAL_DEBUG
#include <SoftwareSerial.h>
extern SoftwareSerial SoftSerialDebug;
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif

#endif
