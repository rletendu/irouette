#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SERIAL_DEBUG

#define DHTTYPE       DHT22
#define DHT_PIN       D4

#define DS18B20_PIN   D1


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


#endif





