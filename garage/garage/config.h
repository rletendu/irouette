#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DOMOTICZ_DEBUG
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
#endif

// Domoticz Server IDx configuration
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



#endif





