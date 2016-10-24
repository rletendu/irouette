#include "Arduino.h"
#include "config.h"
#include <Wire.h>
#include "ds3231.h"

void rtc_init(void)
{
  Wire.begin();
  DS3231_init(DS3231_INTCN);
}

void rtc_next_alarm(uint8_t minutes_delay)
{
  struct ts rtc_time;
  unsigned char wakeup_min;
  DS3231_get(&rtc_time);
  // calculate the minute when the next alarm will be triggered
  wakeup_min = (rtc_time.min / minutes_delay + 1) * minutes_delay;
  if (wakeup_min > 59) {
    wakeup_min -= 60;
  }
  // flags define what calendar component to be checked against the current time in order
  // to trigger the alarm
  // A2M2 (minutes) (0 to enable, 1 to disable)
  // A2M3 (hour)    (0 to enable, 1 to disable)
  // A2M4 (day)     (0 to enable, 1 to disable)
  // DY/DT          (dayofweek == 1/dayofmonth == 0)
  uint8_t flags[4] = { 0, 1, 1, 1 };

  // set Alarm2. only the minute is set since we ignore the hour and day component
  DS3231_set_a2(wakeup_min, 0, 0, flags);
  // activate Alarm2
  DS3231_set_creg(DS3231_INTCN | DS3231_A2IE );
}

void rtc_get_time(struct ts *rtc_time)
{
#ifdef DEBUG_USE_VIRTUAL_SENSORS
  rtc_time->min += 10;
  if (rtc_time->min > 59) {
    rtc_time->min -= 60;
    rtc_time->hour += 1;
    if ( rtc_time->hour > 23) {
      rtc_time->hour -= 24;
    }
  }
#else
  DS3231_get(rtc_time);
#endif
}

void rtc_set_time(struct ts *rtc_time)
{
#ifdef DEBUG_USE_VIRTUAL_SENSORS

#else
  struct ts uptime;
  uptime.year = rtc_time->year;
  uptime.mon = rtc_time->mon;
  uptime.mday = rtc_time->mday;
  uptime.hour = rtc_time->hour;
  uptime.min = rtc_time->min;
  uptime.sec = rtc_time->sec;
  DS3231_set(uptime);
#endif
}

void rtc_ack_alarm(void)
{
  DS3231_clear_a2f();
}

