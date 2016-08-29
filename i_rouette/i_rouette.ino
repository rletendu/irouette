#include "config.h"
#include "board.h"
#include "my_sensors.h"
#include "rtc.h"
#include "radio.h"
#include "LowPower.h"
#include "prescaler.h"
#include <SoftwareSerial.h>

struct SensorValues sensors_val;
enum   ChargeState charge_status;
enum   OperatingMode mode = DAY;
struct ParamValues param;
struct ts rtc_time;
bool   rtc_wake = false;
char   buff[BUFF_MAX];
uint16_t time_minute;


// Define where debug output will be printed.
#define DEBUG_PRINTER SoftSerialDebug

// Setup debug printing macros.
#ifdef SERIAL_DEBUG

SoftwareSerial SoftSerialDebug(0, DBG_TX_PIN); // RX, TX
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif

void debug_sensors(void);
void radio_task(void);
uint8_t setup_tx_frame(void);
bool parse_rx_frame(void);

ISR(BADISR_vect)
{

    led_tail(true);
    while (1);
}

void setup() {

  board_init();

#ifdef SERIAL_DEBUG
  DEBUG_PRINTER.begin(9600);
#endif

  DEBUG_PRINTLN("\n*** I_ROUETTE BOOT ***\n");

  param.valid_sig = VALID_SIG;
  param.night_mode_lum = DEFAULT_NIGHT_MODE_LUM;
  param.sleep_period = DEFAULT_SLEEP_PERIOD;
  param.sunset_time = DEFAULT_SUNSET_TIME;
  param.sunrise_time = DEFAULT_SUNRISE_TIME;
  param.rpm_measure_time = DEFAULT_RPM_MEASURE_TIME;
  param.vcc_light_min = DEFAULT_VBAT_LIGHT_MIN;
  param.vcc_radio_min = DEFAULT_VBAT_RADIO_MIN;

  sensor_enable(true);
  rtc_init();
  rtc_get_time(&rtc_time);
  if ( rtc_time.year < 2016) {
    DEBUG_PRINTLN("RTC Time Error");
    beep(10, true);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  }
  if (0) {
    DEBUG_PRINTLN("Time not setup!");
    rtc_time.year = 2016;
    rtc_time.mon = 8;
    rtc_time.mday = 20;
    rtc_time.hour = 16;
    rtc_time.min = 41;
    rtc_time.sec = 0;
    rtc_set_time(&rtc_time);
    rtc_get_time(&rtc_time);
  }
  sensors_update(&sensors_val, param.rpm_measure_time);
  rtc_ack_alarm();
  rtc_next_alarm(param.sleep_period);
  sensor_enable(false);
  debug_sensors();

}

void rtc_wakeUp()
{
  beep(1, true);
  detachInterrupt(RTC_INT);
  // Back to full speed active
  cpu_8MHZ();
  rtc_wake = true;
}

void loop()
{
  time_minute = 60 * rtc_time.hour + rtc_time.min;
  // Compute operating mode
  if (time_minute > param.sunset_time || time_minute < param.sunrise_time) {
    if (sensors_val.lum < param.night_mode_lum && sensors_val.vbat > param.vcc_light_min ) {
      mode = NIGHT_LIGHT_ON;
      DEBUG_PRINTLN("- Mode NIGHT_LIGHT_ON")
    } else {
      mode = NIGHT_LIGHT_OFF;
      DEBUG_PRINTLN("- Mode NIGHT_LIGHT_OFF")
    }
  } else {
    mode = DAY;
    DEBUG_PRINTLN("- Mode DAY")
  }

  radio_task();

  switch (mode) {
    case DAY:
    case NIGHT_LIGHT_OFF:
      all_led_off();
      attachInterrupt(RTC_INT, rtc_wakeUp, FALLING );
      beep(2, true);
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      break;

    case NIGHT_LIGHT_ON:
      // TODO LED control !
      //cpu_250KHZ();
      led_tail(true);
      attachInterrupt(RTC_INT, rtc_wakeUp, FALLING );
      beep(2, true);
      //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_ON, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      break;

    default:
      break;
  }

  if (rtc_wake) {
    DEBUG_PRINTLN("- Wakeup !");
    
    rtc_wake = false;
    charge_status = get_charge_status();
    DEBUG_PRINTLN("-Sensors Update");
    sensor_enable(true);
    rtc_get_time(&rtc_time);
    sensors_update(&sensors_val, param.rpm_measure_time);
    rtc_ack_alarm();
    rtc_next_alarm(param.sleep_period);
    sensor_enable(false);
    debug_sensors();
  }
}

void radio_task(void)
{
  uint8_t i;

  radio_enable(true);
  if (radio_connect()) {
    DEBUG_PRINTLN("Radio Connected");
    setup_tx_frame();
    if (radio_send(buff)) {
      DEBUG_PRINTLN("Radio Tx OK");
      for (i = 0; i < BUFF_MAX; i++) {
        buff[i] = 0;
      }
      if (radio_get_param(buff)) {
        DEBUG_PRINTLN(buff);
        parse_rx_frame();
      } else {
        DEBUG_PRINTLN("Radio Rx Param Fail");
        goto radio_end;
      }
    } else {
      DEBUG_PRINTLN("Radio Tx Fail");
      goto radio_end;
    }
  }
  else {
    DEBUG_PRINTLN("Radio Not Connected");
  }
radio_end:
  radio_disconnect();
  radio_enable(false);

}


uint8_t setup_tx_frame(void)
{
  char index = 0;
  char remain = BUFF_MAX;
  char str_temp[6];

  snprintf(&buff[index], remain, ":D|%d|%02d|%02d|%02d|%02d|%02d",
           rtc_time.year, rtc_time.mon, rtc_time.mday, rtc_time.hour, rtc_time.min, rtc_time.sec);

  index = strlen(buff); remain -= index;
  dtostrf(sensors_val.temp_ext, 4, 1, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain -= index;
  dtostrf(sensors_val.temp_int, 4, 1, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain -= index;
  dtostrf(sensors_val.pressure, 4, 1, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain -= index;
  snprintf(&buff[index], remain, "|%d", sensors_val.lum);

  index = strlen(buff); remain -= index;
  dtostrf(sensors_val.humidity, 4, 1, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain -= index;
  snprintf(&buff[index], remain, "|%d", sensors_val.rain);

  index = strlen(buff); remain -= index;
  dtostrf(sensors_val.vbat, 3, 2, str_temp);
  snprintf(&buff[index], remain, "|%s", str_temp);

  index = strlen(buff); remain -= index;
  snprintf(&buff[index], remain, "|%d|%d|%d|%d", sensors_val.wind_dir, sensors_val.wind_speed, mode, charge_status);
  DEBUG_PRINT("Tx Frame "); DEBUG_PRINTLN(buff);

  return strlen(buff);
}


bool parse_rx_frame(void)
{
  uint8_t i, j, nb_sep;
  uint8_t sep[MAX_PARAM_FIELDS];
  uint16_t server_time_minute;
  struct ts server_time;
  if (buff[0] != 'o') {
    return false;
  }

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
    DEBUG_PRINTLN("Err Param number");
    return false;
  }

  server_time.year = atoi(&buff[sep[INDEX_YEAR] + 1]);
  server_time.mon = atoi(&buff[sep[INDEX_MONTH] + 1]);
  server_time.mday = atoi(&buff[sep[INDEX_DAY] + 1]);
  server_time.hour = atoi(&buff[sep[INDEX_HOUR] + 1]);
  server_time.min = atoi(&buff[sep[INDEX_MIN] + 1]);
  server_time.sec = atoi(&buff[sep[INDEX_SEC] + 1]);
  server_time_minute = server_time.min + 60 * server_time.hour;

  // sensor_enable(true);
  rtc_get_time(&rtc_time);
  time_minute = 60 * rtc_time.hour + rtc_time.min;
  if ( abs(time_minute - server_time_minute) > 5) {
    DEBUG_PRINTLN("RTC Time Update")
    rtc_set_time(&server_time);
    // When RTC time is updated, we need to be sure to re-setup next wakeup alarm !
    rtc_ack_alarm();
    rtc_next_alarm(param.sleep_period);
  }
  // sensor_enable(false);
  param.night_mode_lum = atoi(&buff[sep[INDEX_NIGHT_LUM] + 1]);
  DEBUG_PRINT("night_mode_lum:"); DEBUG_PRINTLN(param.night_mode_lum);
  param.sleep_period = atoi(&buff[sep[INDEX_SLEEP_TIME] + 1]);
  DEBUG_PRINT("sleep_period:"); DEBUG_PRINTLN(param.sleep_period);
  param.sunset_time = atoi(&buff[sep[INDEX_SUNSET] + 1]);
  DEBUG_PRINT("sunset_time:"); DEBUG_PRINTLN(param.sunset_time);
  param.sunrise_time = atoi(&buff[sep[INDEX_SUNRISE] + 1]);
  DEBUG_PRINT("sunrise_time:"); DEBUG_PRINTLN(param.sunrise_time);
  param.rpm_measure_time = atoi(&buff[sep[INDEX_RPM_TIME] + 1]);
  DEBUG_PRINT("rpm_measure_time:"); DEBUG_PRINTLN(param.rpm_measure_time);
  param.vcc_light_min = atof(&buff[sep[INDEX_VCC_LIGHT] + 1]);
  DEBUG_PRINT("vcc_light_min:"); DEBUG_PRINTLN(param.vcc_light_min);
  param.vcc_radio_min = atof(&buff[sep[INDEX_VCC_RADIO] + 1]);
  DEBUG_PRINT("vcc_radio_min:"); DEBUG_PRINTLN(param.vcc_radio_min);
  DEBUG_PRINTLN("Param OK");
  return true;
}



void debug_sensors(void)
{
  DEBUG_PRINTLN();
  // display current time
  snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", rtc_time.year,
           rtc_time.mon, rtc_time.mday, rtc_time.hour, rtc_time.min, rtc_time.sec);
  DEBUG_PRINTLN(buff);
  DEBUG_PRINT("Charge Status:");
  DEBUG_PRINTLN((int)charge_status);
  DEBUG_PRINT("VBAT:");
  DEBUG_PRINTLN(sensors_val.vbat, 2);
  DEBUG_PRINT("Win dir:");
  DEBUG_PRINTLN(sensors_val.wind_dir);
  DEBUG_PRINT("Temp internal:");
  DEBUG_PRINTLN(sensors_val.temp_int, 2);
  DEBUG_PRINT("Temp external:");
  DEBUG_PRINTLN(sensors_val.temp_ext, 2);
  DEBUG_PRINT("Pressure:");
  DEBUG_PRINTLN(sensors_val.pressure, 2);
  DEBUG_PRINT("Lum:");
  DEBUG_PRINTLN(sensors_val.lum);
  DEBUG_PRINT("Humidity:");
  DEBUG_PRINTLN(sensors_val.humidity, 2);
  DEBUG_PRINT("Wind Speed (rpm):");
  DEBUG_PRINTLN(sensors_val.wind_speed);
}
