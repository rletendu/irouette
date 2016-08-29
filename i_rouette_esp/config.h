#ifndef __CONFIG_H__
#define __CONFIG_H__

// #define DOMOTICZ_DEBUG
// #define IROUETTE_ESP_DEBUG


#define DOMO_CONNECT 1

#if (DOMO_CONNECT==1)
  #define DOMOTICZ_SERVER "192.168.2.201"
  #define DOMOTICZ_PORT 8080
  #define MYSSID "domo"
  #define PASSWD "pulsar est content"
#elif (DOMO_CONNECT==2)
  #define DOMOTICZ_SERVER "rletendu.no-ip.org"
  #define DOMOTICZ_PORT 8080
  #define MYSSID "honor6"
  #define PASSWD "pulsar44"
#elif (DOMO_CONNECT==3)
  #define DOMOTICZ_SERVER "rletendu.no-ip.org"
  #define DOMOTICZ_PORT 8080
  #define MYSSID "AVRGUEST"
  #define PASSWD "MicroController"
#endif

// Domoticz Server IDx configuration
#define IDX_WIND          4
#define IDX_BARO          5
#define IDX_BATTERY       6
#define IDX_TEMP_INT      7
#define IDX_LUX           8


#define IDX_VAR_RPM_TIME   8
#define IDX_VAR_SLEEP_TIME 9
#define IDX_VAR_NIGHT_LUM  10
#define IDX_VAR_VCC_LIGHT  11
#define IDX_VAR_VCC_RADIO  12


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

#endif





