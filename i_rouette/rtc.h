#ifndef __RTC_H__
#define __RTC_H__

void rtc_init(void);
void rtc_next_alarm(uint8_t minutes_delay);
void rtc_ack_alarm(void);
void rtc_get_time(struct ts *rtc_time);
void rtc_set_time(struct ts *rtc_time);


#endif //__RTC_H__
