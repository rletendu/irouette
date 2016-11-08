#include "config.h"
#include "board.h"
#include "protocol.h"
#include "my_sensors.h"
#include "rtc.h"
#include "radio.h"
#include "LowPower.h"
#include "prescaler.h"
#include <SoftwareSerial.h>
#include <avr/wdt.h>

struct SensorValues sensors_val;
enum   ChargeState charge_status;
enum   OperatingMode mode = DAY;
struct ParamValues param;
struct ts rtc_time;
bool   rtc_wake = false;
char   buff[BUFF_MAX];
uint16_t time_minute;
uint8_t led_fsm = 0;

void debug_sensors(void);
void debug_param(void);
void radio_task(void);
uint8_t setup_tx_frame(void);
bool parse_rx_frame(void);
void led_task(void);

#ifdef SERIAL_DEBUG
SoftwareSerial DEBUG_PRINTER(0, DBG_TX_PIN); // RX, TX
#endif

ISR(BADISR_vect)
{
  led_tail(true);
  while (1) {
    wdt_enable(WDTO_15MS);
  }
}
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void wdt_init(void)
{
  MCUSR = 0;
  wdt_disable();
  return;
}

void setup() {

  board_init();

#ifdef SERIAL_DEBUG
  DEBUG_PRINTER.begin(9600);
#endif

  DEBUG_PRINTLN(F("\n******* I_ROUETTE BOOT *******\n"));
  DEBUG_PRINT( F("Compiled: "));
  DEBUG_PRINT( F(__DATE__));
  DEBUG_PRINT( F(", "));
  DEBUG_PRINT( F(__TIME__));

  param.valid_sig = VALID_SIG;
  param.night_mode_lum = DEFAULT_NIGHT_MODE_LUM;
  param.sleep_period = DEFAULT_SLEEP_PERIOD;
  param.sunset_time = DEFAULT_SUNSET_TIME;
  param.sunrise_time = DEFAULT_SUNRISE_TIME;
  param.rpm_measure_time = DEFAULT_RPM_MEASURE_TIME;
  param.vcc_light_min = DEFAULT_VBAT_LIGHT_MIN;
  param.vcc_radio_min = DEFAULT_VBAT_RADIO_MIN;
  param.rpm_2_ms = DEFAULT_RPM_2_MS;
  debug_param();

#ifdef  DEBUG_RADIO_CONNECT_ONLY
  rtc_time.year = 2016;
  rtc_time.mon = 8;
  rtc_time.mday = 20;
  rtc_time.hour = 20;
  rtc_time.min = 10;
  rtc_time.sec = 0;
 
  sensors_val.vbat = get_vbat();
  sensors_val.lum = 815;
  sensors_val.rain = 127;
  sensors_val.temp_int = 12.8;
  sensors_val.pressure = 1015;
  sensors_val.humidity = 60.3;
  sensors_val.temp_ext = 12.6;
  sensors_val.wind_dir =  180;
  sensors_val.wind_speed = 7.1;

  DEBUG_PRINTLN(F("DEBUG_RADIO_CONNECT_ONLY !!! "));
  while (1) {
    vcc_sensor_enable(true);
    //rtc_get_time(&rtc_time);
    DEBUG_PRINTLN(F("Sensor Job"));
    sensors_update(&sensors_val, param.rpm_measure_time, param.rpm_2_ms);
    radio_task();
    vcc_sensor_enable(false);
    delay(100);
  }
#endif
#ifdef DEBUG_SENSOR_ONLY
  sensor_debug_loop();
#endif

  vcc_sensor_enable(true);
  rtc_init();
  rtc_get_time(&rtc_time);
  if ( rtc_time.year < 2016) {
    DEBUG_PRINTLN(F("RTC Time Error"));
    beep(10, true);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  }
  if (0) {
    DEBUG_PRINTLN(F("Time not setup!"));
    rtc_time.year = 2016;
    rtc_time.mon = 8;
    rtc_time.mday = 20;
    rtc_time.hour = 16;
    rtc_time.min = 41;
    rtc_time.sec = 0;
    rtc_set_time(&rtc_time);
    rtc_get_time(&rtc_time);
  }
  vcc_sensor_enable(false);
  rtc_wake = true; // Force rtc wake for first loop
}

void rtc_wakeUp()
{
  beep(1, true);
  detachInterrupt(RTC_INT);
  cpu_8MHZ(); // Back to full speed active
  rtc_wake = true;
}

void loop()
{
  uint8_t i;


  if (rtc_wake) {
    if (mode == NIGHT_LIGHT_ON) {
      all_led_off();
      for (i = 0; i < 10; i++) {
        led_tail(true);
        delay(100);
        led_tail(false);
        delay(100);
      }
      led_tail(true);
    }

    wdt_enable(WDTO_8S);
    DEBUG_PRINTLN(F(" - RTC Wakeup !"));
    rtc_wake = false;

    vcc_sensor_enable(true);

    if (get_vbat() > param.vcc_radio_min ) {
      // Update Sensors Values
      charge_status = get_charge_status();
      DEBUG_PRINTLN(F(" - Sensors Update"));
      rtc_get_time(&rtc_time);
      wdt_reset();
      sensors_update(&sensors_val, param.rpm_measure_time, param.rpm_2_ms);
      debug_sensors();

      // Send Sensor Values to server and get updated parameters from server
      DEBUG_PRINTLN(F(" - Radio Task"));
      wdt_disable();
      radio_task();
      wdt_reset();
      wdt_enable(WDTO_8S);
      wdt_reset();
      debug_param();
      wdt_reset();
    } else {
      DEBUG_PRINTLN(F(" - Skipping Sensor update and Radio Task"));
    }

    // Setup next wakeup by RTC
    DEBUG_PRINTLN(F(" - Setup next wakeup time"));
    wdt_reset();
    rtc_ack_alarm();
    rtc_next_alarm(param.sleep_period);
    vcc_sensor_enable(false);
    wdt_disable();

    // Compute operating mode
    time_minute = 60 * rtc_time.hour + rtc_time.min;
    // Check current time versus sunset / sunrise
    if (time_minute > param.sunset_time || time_minute < param.sunrise_time) {
      // Check Vbat level and Luminosity level to turn light On
      if (sensors_val.lum < param.night_mode_lum && sensors_val.vbat > param.vcc_light_min ) {
        mode = NIGHT_LIGHT_ON;
        DEBUG_PRINTLN(F(" - Mode NIGHT_LIGHT_ON"));
      } else {
        mode = NIGHT_LIGHT_OFF;
        DEBUG_PRINTLN(F(" - Mode NIGHT_LIGHT_OFF"));
      }
    } else {
      mode = DAY;
      DEBUG_PRINTLN(F(" - Mode DAY"));
    }
#ifdef FORCE_NIGHT_MODE
    mode = NIGHT_LIGHT_ON;
#endif
    attachInterrupt(RTC_INT, rtc_wakeUp, FALLING );
    beep(2, true);
  }

  switch (mode) {
    case DAY:
    case NIGHT_LIGHT_OFF:
      all_led_off();
      DEBUG_PRINTLN(F(" - Sleeping Day or Light off mode"));
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      break;

    case NIGHT_LIGHT_ON:
      led_task();
      DEBUG_PRINTLN(F(" - Sleeping Light On mode"));
      if (sensors_val.vbat > (param.vcc_light_min + 0.1) ) {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      } else {
        LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      }
      break;

    default:
      break;
  }
}

void radio_task(void)
{
  uint8_t i;
  if ( radio_sync() == false) {
    led_error_code(ERROR_SYNC);
    DEBUG_PRINTLN(F(" - Radio Sync Error"));
    return;
  }
  DEBUG_PRINTLN(F(" - Radio Sync"));

  if (radio_connect() == false) {
    led_error_code(ERROR_CONNECT);
    DEBUG_PRINTLN(F(" - Radio Not Connected"));
    return;
  }
  DEBUG_PRINTLN(F(" - Radio Connected"));

  setup_tx_frame();
  if (radio_send(buff) == false) {
    led_error_code(ERROR_TX);
    DEBUG_PRINTLN(F(" - Radio Tx Fail"));
  }
  DEBUG_PRINTLN(F(" - Radio Tx OK"));

  for (i = 0; i < BUFF_MAX; i++) {
    buff[i] = 0;
  }
  if (radio_get_param(buff) == false) {
    led_error_code(ERROR_RX);
    DEBUG_PRINTLN(F(" - Radio Rx Param Fail"));
  } else {
    DEBUG_PRINTLN(F(" - Radio Rx Param OK"));
    DEBUG_PRINTLN(buff);
    parse_rx_frame();
  }
  radio_disconnect();
}

uint8_t setup_tx_frame(void)
{
  char index = 0;
  char remain = BUFF_MAX;
  char str_temp[6];

  snprintf(&buff[index], remain, ":D|%d|%02d|%02d|%02d|%02d|%02d",
           rtc_time.year, rtc_time.mon, rtc_time.mday, rtc_time.hour, rtc_time.min, rtc_time.sec);

  index = strlen(buff); remain = BUFF_MAX - index;
  dtostrf(sensors_val.temp_ext, 3, 1, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain = BUFF_MAX - index;
  dtostrf(sensors_val.temp_int, 3, 1, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain = BUFF_MAX - index;
  dtostrf(sensors_val.pressure, 3, 1, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain = BUFF_MAX - index;
  snprintf(&buff[index], remain, "|%d", sensors_val.lum);

  index = strlen(buff); remain = BUFF_MAX - index;
  dtostrf(sensors_val.humidity, 3, 1, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain = BUFF_MAX - index;
  snprintf(&buff[index], remain, "|%d", sensors_val.rain);

  index = strlen(buff); remain = BUFF_MAX - index;
  dtostrf(sensors_val.vbat, 3, 2, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain = BUFF_MAX - index;
  snprintf(&buff[index], remain, "|%d|%d|%d|%d", sensors_val.wind_dir, sensors_val.wind_speed, mode, charge_status);
  DEBUG_PRINT("Tx Frame "); DEBUG_PRINTLN(buff);

  return strlen(buff);
}


bool parse_rx_frame(void)
{
  uint8_t i, j, nb_sep;
  uint8_t sep[INDEX_PARAM_MAX + 1];
  uint16_t server_time_minute;
  struct ts server_time;
  if (buff[0] != 'o') {
    return false;
  }

  DEBUG_PRINT(F("Rx Frame ")); DEBUG_PRINTLN(buff);
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
  if (nb_sep != (INDEX_PARAM_MAX + 1)) {
    DEBUG_PRINTLN(F("Err Param number"));
    return false;
  }

  server_time.year = atoi(&buff[sep[INDEX_YEAR] + 1]);
  server_time.mon = atoi(&buff[sep[INDEX_MONTH] + 1]);
  server_time.mday = atoi(&buff[sep[INDEX_DAY] + 1]);
  server_time.hour = atoi(&buff[sep[INDEX_HOUR] + 1]);
  server_time.min = atoi(&buff[sep[INDEX_MIN] + 1]);
  server_time.sec = atoi(&buff[sep[INDEX_SEC] + 1]);
  server_time_minute = server_time.min + 60 * server_time.hour;

  rtc_get_time(&rtc_time);
  time_minute = 60 * rtc_time.hour + rtc_time.min;
  if ( abs(time_minute - server_time_minute) > RTC_MAX_DEVIATION) {
    DEBUG_PRINTLN(F(" - RTC Time Update !"));
    rtc_set_time(&server_time);
  }

  param.night_mode_lum = atoi(&buff[sep[INDEX_NIGHT_LUM] + 1]);
  param.sleep_period = atoi(&buff[sep[INDEX_SLEEP_TIME] + 1]);
  param.sunset_time = atoi(&buff[sep[INDEX_SUNSET] + 1]);
  param.sunrise_time = atoi(&buff[sep[INDEX_SUNRISE] + 1]);
  param.rpm_measure_time = atoi(&buff[sep[INDEX_RPM_TIME] + 1]);
  param.vcc_light_min = atof(&buff[sep[INDEX_VCC_LIGHT] + 1]);
  if (param.vcc_light_min < 3.0 ) param.vcc_light_min = DEFAULT_VBAT_LIGHT_MIN;

  param.vcc_radio_min = atof(&buff[sep[INDEX_VCC_RADIO] + 1]);
  if (param.vcc_radio_min < 3.0) param.vcc_radio_min = DEFAULT_VBAT_RADIO_MIN;
  param.rpm_2_ms = atof(&buff[sep[INDEX_RPM_2_MS] + 1]);
  DEBUG_PRINTLN(F("Get Param OK"));
  return true;
}



void debug_sensors(void)
{
  DEBUG_PRINTLN();
  // display current time
  DEBUG_PRINTLN(F("-------- Sensors --------"));
  snprintf(buff, BUFF_MAX, "\tRTC: % d. % 02d. % 02d % 02d: % 02d: % 02d", rtc_time.year,
           rtc_time.mon, rtc_time.mday, rtc_time.hour, rtc_time.min, rtc_time.sec);
  DEBUG_PRINTLN(buff);
  DEBUG_PRINT(F("\tCharge State: "));
  DEBUG_PRINTLN((int)charge_status);
  DEBUG_PRINT(F("\tVBAT: "));
  DEBUG_PRINTLN(sensors_val.vbat, 2);
  DEBUG_PRINT(F("\tWin dir: "));
  DEBUG_PRINTLN(sensors_val.wind_dir);
  DEBUG_PRINT(F("\tWind Speed: "));
  DEBUG_PRINTLN(sensors_val.wind_speed);
  DEBUG_PRINT(F("\tTemp int: "));
  DEBUG_PRINTLN(sensors_val.temp_int, 2);
  DEBUG_PRINT(F("\tTemp ext: "));
  DEBUG_PRINTLN(sensors_val.temp_ext, 2);
  DEBUG_PRINT(F("\tPressure: "));
  DEBUG_PRINTLN(sensors_val.pressure, 2);
  DEBUG_PRINT(F("\tLuminosity: "));
  DEBUG_PRINTLN(sensors_val.lum);
  DEBUG_PRINT(F("\tHumidity: "));
  DEBUG_PRINTLN(sensors_val.humidity, 2);
}

void debug_param(void)
{
  DEBUG_PRINTLN();
  DEBUG_PRINTLN(F("-------- Parameters --------"));
  DEBUG_PRINT(F("\tnight_mode_lum: ")); DEBUG_PRINTLN(param.night_mode_lum);
  DEBUG_PRINT(F("\tsleep_period: ")); DEBUG_PRINTLN(param.sleep_period);
  DEBUG_PRINT(F("\tsunset_time: ")); DEBUG_PRINTLN(param.sunset_time);
  DEBUG_PRINT(F("\tsunrise_time: ")); DEBUG_PRINTLN(param.sunrise_time);
  DEBUG_PRINT(F("\trpm_measure_time: ")); DEBUG_PRINTLN(param.rpm_measure_time);
  DEBUG_PRINT(F("\tvcc_light_min: ")); DEBUG_PRINTLN(param.vcc_light_min, 2);
  DEBUG_PRINT(F("\tvcc_radio_min: ")); DEBUG_PRINTLN(param.vcc_radio_min, 2);
  DEBUG_PRINT(F("\trpm_to_ms: ")); DEBUG_PRINTLN(param.rpm_2_ms, 2);
}

void sensor_debug_loop(void) {
  while (1) {
    vcc_sensor_enable(true);
    rtc_get_time(&rtc_time);
    sensors_update(&sensors_val, 2, param.rpm_2_ms);
    debug_sensors();
    vcc_sensor_enable(false);
    delay(1000);
  }
}

void led_task(void)
{
  uint8_t i;
  all_led_off();
#ifdef FORCE_FREEZE
  {
#else
  if (sensors_val.temp_ext <= 0) {
#endif

    for (i = 0; i < 10; i++) {
      all_led_on();
      delay(100);
      all_led_off();
      delay(100);
    }
    led_blink(true);
  }

  led_tail(true);

  if (sensors_val.vbat > (param.vcc_light_min + 0.1) ) {
    switch (led_fsm) {
      case 0:
        led_blue_head(true);
        led_white(true);
        led_fsm++;
        break;

      case 1:
        led_green_head(true);
        led_white(true);
        led_fsm++;
        break;

      case 2:
        led_green_head(true);
        led_green_right(true);
        led_green_left(true);
        led_fsm++;
        break;

      case 3:
        led_blue_head(true);
        led_red_right(true);
        led_red_left(true);
        led_fsm++;
        break;

      default:
        all_led_on();
        led_fsm = 0;
        break;

    }
  } else {
    switch (led_fsm) {
      case 0:
        led_blue_head(true);
        led_fsm++;
        break;

      case 1:
        led_green_head(true);
        led_fsm++;
        break;

      default:
        led_green_head(true);
        led_green_right(true);
        led_green_left(true);
        led_fsm = 0;
        break;
    }
  }
}

