#ifndef __MY_SENSORS_H__
#define __MY_SENSORS_H__

void sensors_update(struct SensorValues *val, int speed_measure_time, float rpm_2_ms);
float get_vbat(void);

#endif
