#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include "config.h"
#include "domoticz/domoticz.h"
#include "protocol.h"

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Setup debug printing macros.
#ifdef IROUETTE_ESP_DEBUG
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif


#define BUFF_MAX 200
char    buff[BUFF_MAX];

char nb;
String dataInput;

Domoticz domo = Domoticz();
bool send_data_frame(void);
bool build_param_frame(void);
void status_ok(void);
void status_ko(void);
void scan(void);
void setup() {
  Serial.end();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  while (Serial.available() > 0) {
    Serial.read();
  }
}

void loop() {

  int i;

  nb = Serial.readBytesUntil('\n', buff, BUFF_MAX);
  if (nb) {
    if (buff[0] != ':' ) {
      status_ko();
    } else {
      switch (buff[1]) {
        // Connect to Wifi command
        case 'C':
          if (domo.begin()) {
            status_ok();
          } else {
            status_ko();
          }
          break;

        // Send Data Frame to Domoticz server command
        case 'D':
          if (send_data_frame()) {
            status_ok();
          } else {
            status_ko();
          }
          break;

        // Receive Param Frame from Domoticz server command
        case 'P':
          if (build_param_frame()) {
            Serial.println(buff);
          } else {
            status_ko();
          }
          break;

        // Sleep command Request
        case 'S':
          status_ok();
          delay(100);
          Serial.end();
          ESP.deepSleep(1 * 60 * 1000000);
          delay(100);
          break;

        // Ping command (check correct communication with ESP serial)
        case 'p':
          status_ok();
          break;

        case 'n':
          scan();
          status_ok();
          break;

        // Unknown command
        default:
          status_ko();
          break;
      }
    }
  }
}

bool send_data_frame(void)
{
  uint8_t i, j, nb_sep;
  uint8_t tx_error_cnt = 0;
  uint8_t sep[10];
  String message;

  char *temp_ext;
  char *temp_int;
  char *pressure;
  char *lum;
  char *hum;
  char *vbat;
  char *wind_dir;
  char *wind_speed;

  nb_sep = 0;
  for (i = 0; i < BUFF_MAX; i++) {
    if (buff[i] == 0) {
      break;
    }
    if (buff[i] == '|') {
      sep[nb_sep++] = i;
      buff[i] = 0;
    }
  }
  if (nb_sep != (INDEX_DATA_MAX + 1)) {
    DEBUG_PRINT("-Incorrect Data Frame "); DEBUG_PRINT(nb_sep); DEBUG_PRINTLN(" Sep found!");
    return false;
  }

  temp_ext = &buff[sep[INDEX_TEMP_EXT] + 1];
  temp_int = &buff[sep[INDEX_TEMP_INT] + 1];
  pressure = &buff[sep[INDEX_PRESSURE] + 1];
  lum = &buff[sep[INDEX_LUM] + 1];
  hum = &buff[sep[INDEX_HUMIDITY] + 1];
  vbat = &buff[sep[INDEX_VBAT] + 1];
  wind_dir = &buff[sep[INDEX_WIND_DIR] + 1];
  wind_speed = &buff[sep[INDEX_WIND_SPEED] + 1];

  // :D|2016|08|16|11|26|58|23.1|27.2|1034|555|33|NA_RAIN|3.75|224|100|NA_MODE|NA_CHARGE
  DEBUG_PRINT("-YEAR:");   DEBUG_PRINTLN((char*)&buff[sep[INDEX_YEAR] + 1]);
  DEBUG_PRINT("-MONTH:");  DEBUG_PRINTLN((char*)&buff[sep[INDEX_MONTH] + 1]);
  DEBUG_PRINT("-DAY:");    DEBUG_PRINTLN((char*)&buff[sep[INDEX_DAY] + 1]);
  DEBUG_PRINT("-HOUR:");   DEBUG_PRINTLN((char*)&buff[sep[INDEX_HOUR] + 1]);
  DEBUG_PRINT("-MIN:");    DEBUG_PRINTLN((char*)&buff[sep[INDEX_MIN] + 1]);
  DEBUG_PRINT("-SECOND:"); DEBUG_PRINTLN((char*)&buff[sep[INDEX_SEC] + 1]);

  DEBUG_PRINT("-TEMP EXT:"); DEBUG_PRINTLN(temp_ext);
  DEBUG_PRINT("-TEMP_INT:"); DEBUG_PRINTLN(temp_int);

  DEBUG_PRINT("-PRESSURE:"); DEBUG_PRINTLN(pressure);
  DEBUG_PRINT("-LUM:"); DEBUG_PRINTLN(lum);
  DEBUG_PRINT("-HUM:"); DEBUG_PRINTLN(hum);
  DEBUG_PRINT("-RAIN:"); DEBUG_PRINTLN((char*)&buff[sep[INDEX_RAIN] + 1]);
  DEBUG_PRINT("-VBAT:"); DEBUG_PRINTLN(vbat);
  DEBUG_PRINT("-WINDIR:"); DEBUG_PRINTLN(wind_dir);
  DEBUG_PRINT("-WINSPEED:"); DEBUG_PRINTLN(wind_speed);
  DEBUG_PRINT("-MODE:"); DEBUG_PRINTLN((char*)&buff[sep[INDEX_MODE] + 1]);
  DEBUG_PRINT("-CHARGE:"); DEBUG_PRINTLN((char*)&buff[sep[INDEX_CHARGE] + 1]);

  if (domo.update_temperature(IDX_TEMP_INT, temp_int) == false) {
    tx_error_cnt++;
  }
  if (domo.update_wind(IDX_WIND, wind_dir, wind_speed) == false) {
    tx_error_cnt++;
  }
  if (domo.update_voltage(IDX_BATTERY, vbat) == false) {
    tx_error_cnt++;
  }
  if (domo.udpate_temp_hum_baro(IDX_BARO, temp_ext, hum, pressure) == false) {
    tx_error_cnt++;
  }
  if (domo.update_luminosity(IDX_LUX, lum) == false) {
    tx_error_cnt++;
  }
  message = "Irouette-Message";
  message += String(':') + String((char*)(&buff[sep[INDEX_YEAR] + 1]));
  message += String('_') + String(((char*)&buff[sep[INDEX_MONTH] + 1]));
  message += String('_') + String(((char*)&buff[sep[INDEX_DAY] + 1]));
  message += String('_') + String(((char*)&buff[sep[INDEX_HOUR] + 1]));
  message += String('_') + String(((char*)&buff[sep[INDEX_MIN] + 1]));
  message += String('_') + String(((char*)&buff[sep[INDEX_SEC] + 1]));
  message += String('_') + String(((char*)&buff[sep[INDEX_MODE] + 1]));
  message.toCharArray(buff, BUFF_MAX);
  DEBUG_PRINT("-Sending LOG:"); DEBUG_PRINTLN(buff);
  if (domo.send_log_message(buff) == false) {
    tx_error_cnt++;
  }
  if (tx_error_cnt) {
    return false;
  }
  return true;

}

/*
   "ServerTime" : "2016-08-25 13:58:12",
   "Sunrise" : "07:15",
   "Sunset" : "20:56",
*/
bool build_param_frame(void)
{
  char tmp_buff[30];
  char var_night_lum[8];
  char var_sleep_time[8];
  char var_rpm_time[8];
  char var_vcc_light[8];
  char var_vcc_radio[8];
  char var_rpm_2_ms[8];

  uint8_t i;
  char c;
  uint16_t t_rise;
  uint16_t t_set;

  for (i = 0; i < sizeof(buff); i++) {
    buff[i] = 0;
  }
  buff[0] = 'o';
  buff[1] = '|';
  // "2016-08-25 13:58:12"
  if (domo.get_servertime(tmp_buff)) {
    for (i = 0; i < 19; i++) {
      c = tmp_buff[i];
      if (c == '-' || c == ' ' || c == ':' )  {
        c = '|';
      }
      buff[2 + i] = c;
    }
  } else return false;

  // "07:15",
  if (domo.get_sunrise(tmp_buff)) {
    tmp_buff[2] = 0;
    tmp_buff[5] = 0;
    t_rise = 60 * atoi(&tmp_buff[0]);
    t_rise += atoi(&tmp_buff[3]);
  } else return false;

  if (domo.get_sunset(tmp_buff)) {
    tmp_buff[2] = 0;
    tmp_buff[5] = 0;
    t_set = 60 * atoi(&tmp_buff[0]);
    t_set += atoi(&tmp_buff[3]);
  } else return false;

  for (i = 0; i < sizeof(var_night_lum); i++) {
    var_night_lum[i] = 0;
  }
  if (domo.get_variable(IDX_VAR_NIGHT_LUM, var_night_lum) == false) {
    return false;
  }

  for (i = 0; i < sizeof(var_sleep_time); i++) {
    var_sleep_time[i] = 0;
  }
  if (domo.get_variable(IDX_VAR_SLEEP_TIME, var_sleep_time) == false) {
    return false;
  }

  for (i = 0; i < sizeof(var_rpm_time); i++) {
    var_rpm_time[i] = 0;
  }
  if (domo.get_variable(IDX_VAR_RPM_TIME, var_rpm_time) == false) {
    return false;
  }

  for (i = 0; i < sizeof(var_vcc_light); i++) {
    var_vcc_light[i] = 0;
  }
  if (domo.get_variable(IDX_VAR_VCC_LIGHT, var_vcc_light) == false) {
    return false;
  }

  for (i = 0; i < sizeof(var_vcc_radio); i++) {
    var_vcc_radio[i] = 0;
  }
  if (domo.get_variable(IDX_VAR_VCC_RADIO, var_vcc_radio) == false) {
    return false;
  }

  for (i = 0; i < sizeof(var_rpm_2_ms); i++) {
    var_rpm_2_ms[i] = 0;
  }
  if (domo.get_variable(IDX_VAR_RPM_2MS, var_rpm_2_ms) == false) {
    return false;
  }

  snprintf(&buff[21], BUFF_MAX - 19, "|%s|%s|%d|%d|%s|%s|%s|%s", var_night_lum, var_sleep_time, t_rise, t_set, var_rpm_time, var_vcc_light, var_vcc_radio, var_rpm_2_ms);
  return true;
}

void status_ok(void)
{
  Serial.println("o");
}

void status_ko(void)
{
  Serial.println("f");
}

void scan(void)
{
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
}






