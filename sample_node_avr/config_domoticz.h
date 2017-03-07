#ifndef __CONFIG_DOMOTICZ_H__
#define __CONFIG_DOMOTICZ_H__


//#define DOMOTICZ_INTERFACE  DOMOTICZ_WIFI
#define DOMOTICZ_INTERFACE  DOMOTICZ_ETHERNET

// #define DOMOTICZ_DEBUG

//#define DOMOTICZ_SEND_VBAT
#define DOMOTICZ_VBAT_MIN 2000
#define DOMOTICZ_VBAT_MAX 3600

//#define DOMOTICZ_SEND_RSSI

#if 0
#define DOMOTICZ_SERVER   "192.168.2.201"
#define DOMOTICZ_PORT     "8080"
#else
#define DOMOTICZ_SERVER   "10.171.140.103"
#define DOMOTICZ_PORT     "8081"
#endif

#define DOMOTICZ_USER     "domoticz_username"
#define DOMOTICZ_PASSWD   "domoticz_password"

#define MYSSID "domo"
#define PASSWD "pulsar est content"

#define DOMO_BUFF_MAX 250


#endif //
