#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SERIAL_DEBUG

#define DHTTYPE       DHT22
// #define DHT_PIN       D4 // Pin where DHT sensor is connected

#define DS18B20_PIN   D1  // Pin where DS18B20 is connected
#define SETUP_PIN     D1


#define DEEP_SLEEP_TIME   1  // Time in minutes where ESP8266 will sleep before restarting, if 0 : no Sleep
#define SERVER_PORT      0  // Server listern port, if 0 no server enable on the node

// Domoticz Server IDx configuration
#define IDX_EDF           1
#define IDX_CABANE_T_H    2
#define IDX_FREEZER       3
#define IDX_WIND          4
#define IDX_BARO          5
#define IDX_BATTERY       6
#define IDX_TEMP_INT      7
#define IDX_LUX           8
#define IDX_GARAGE_DOOR   9
#define IDX_GARAGE_TEMP   10
#define IDX_DUMMY_TEMP    11


#define IDX_VAR_RPM_TIME   8
#define IDX_VAR_SLEEP_TIME 9
#define IDX_VAR_NIGHT_LUM  10
#define IDX_VAR_VCC_LIGHT  11
#define IDX_VAR_VCC_RADIO  12
#define IDX_VAR_RPM_2MS    13

#define DEBUG_PRINTER Serial

#ifdef SERIAL_DEBUG
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif

#define EEPROM_SIZE                  512
#define EEPROM_STR_SIZE              32
#define EEPROM_SSID_OFFSET           0*EEPROM_STR_SIZE
#define EEPROM_PASSWD_OFFSET         1*EEPROM_STR_SIZE
#define EEPROM_SERVER_OFFSET         2*EEPROM_STR_SIZE
#define EEPROM_PORT_OFFSET           3*EEPROM_STR_SIZE
#define EEPROM_SLEEP_TIME_OFFSET     4*EEPROM_STR_SIZE
#define EEPROM_VBAT_MIN_OFFSET       5*EEPROM_STR_SIZE
#define EEPROM_VBAT_MAX_OFFSET       6*EEPROM_STR_SIZE
#define EEPROM_IDX1_OFFSET           7*EEPROM_STR_SIZE
#define EEPROM_IDX2_OFFSET           8*EEPROM_STR_SIZE


#define SETUP_SSID        "SETUP"

#endif





