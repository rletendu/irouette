#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include "ds3231.h"


#define SERIAL_DEBUG

// Pins Assignement
#define _A0_PIN             A0
#define RAIN_PIN            A1
#define _A2_PIN             A2
#define _A3_PIN             A3
#define SDA_PIN             A4
#define SCL_PIN             A5
#define REF25_PIN           A6
#define LDR_PIN             A7

#define RX_PIN              0
#define TX_PIN              1
#define WAKEUP_PIN          2
#define LED1_PIN            3
#define VCC_EN_PIN          4
#define _D5_PIN             5
#define DHT_PIN             6
#define CHARGING_PIN        7
#define CHARGED_PIN         8
#define BUZZER_PIN          9
#define LED_TAIL            10
#define LED0_PWM_PIN        11
#define DBG_TX_PIN          12
#define _D13_PIN            13

#define RTC_INT             0

#define RADIO_BAUD          9600

#define BUFF_MAX            200
#define MAX_PARAM_FIELDS    15
#define RTC_MAX_DEVIATION   5 // In minute

#define PARAM_EE_ADDR            0
#define VALID_SIG                0x55AA
#define DEFAULT_NIGHT_MODE_LUM   523
#define DEFAULT_SLEEP_PERIOD     2
#define DEFAULT_SUNSET_TIME      22*60
#define DEFAULT_SUNRISE_TIME     8*60
#define DEFAULT_RPM_MEASURE_TIME 5
#define DEFAULT_VBAT_LIGHT_MIN   3.5
#define DEFAULT_VBAT_RADIO_MIN   3.3

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
  uint8_t   night_mode_lum;
  uint8_t   sleep_period;
  uint16_t  sunset_time;
  uint16_t  sunrise_time;
  uint8_t   rpm_measure_time;
  float     vcc_light_min;
  float     vcc_radio_min;
};

#define TIMEOUT_CONNECT 20*1000
#define TIMEOUT_TX      20*1000
#define TIMEOUT_RX      20*1000

// Data Frame Indexes
#define INDEX_YEAR        0
#define INDEX_MONTH       1
#define INDEX_DAY         2
#define INDEX_HOUR        3
#define INDEX_MIN         4
#define INDEX_SEC         5
#define INDEX_TEMP_EXT    6
#define INDEX_TEMP_INT    7
#define INDEX_PRESSURE    8
#define INDEX_LUM         9
#define INDEX_HUMIDITY    10
#define INDEX_RAIN        11
#define INDEX_VBAT        12
#define INDEX_WIND_DIR    13
#define INDEX_WIND_SPEED  14
#define INDEX_MODE        15
#define INDEX_CHARGE      16
#define INDEX_DATA_MAX    INDEX_CHARGE

// Param Frame Indexes
#define INDEX_YEAR        0
#define INDEX_MONTH       1
#define INDEX_DAY         2
#define INDEX_HOUR        3
#define INDEX_MIN         4
#define INDEX_SEC         5
#define INDEX_NIGHT_LUM   6
#define INDEX_SLEEP_TIME  7
#define INDEX_SUNRISE     8
#define INDEX_SUNSET      9
#define INDEX_RPM_TIME    10
#define INDEX_VCC_LIGHT   11
#define INDEX_VCC_RADIO   12
#define INDEX_PARAM_MAX   INDEX_VCC_RADIO


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
