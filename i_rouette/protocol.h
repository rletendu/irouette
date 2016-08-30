#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

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
#define INDEX_RPM_2_MS    13
#define INDEX_PARAM_MAX   INDEX_RPM_2_MS


#endif // __PROTOCOL_H__
