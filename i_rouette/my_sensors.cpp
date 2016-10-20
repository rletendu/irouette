#include "config.h"
#include "board.h"

#include <SFE_BMP180.h>
#include "DHT.h"
#include "my_sensors.h"
#include <avr/wdt.h>

#ifdef USE_ADAFRUIT_MAG_LIB
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
#else
#include <HMC5883L.h>
HMC5883L compass;
#endif

DHT dht(DHT_PIN, DHT22);
SFE_BMP180 pressure;

void sensors_init() {
  pressure.begin();
  dht.begin();
#ifdef USE_ADAFRUIT_MAG_LIB
  mag.begin();
#else
  compass = HMC5883L();
  compass.SetScale(1.3);
  compass.SetMeasurementMode(Measurement_Continuous);
#endif
}

float get_vbat(void) {
#ifdef DEBUG_USE_VIRTUAL_SENSORS
  return 3.72;
#else
  analogRead(REF25_PIN);
  return ((2.5 * 1024) / analogRead(REF25_PIN));
#endif
}

void sensors_update(struct SensorValues *val, int speed_measure_time, float rpm_2_ms) {
#ifdef DEBUG_USE_VIRTUAL_SENSORS
  val->vbat = get_vbat();
  val->lum = 815;
  val->rain = 127;
  val->temp_int = 17.8;
  val->pressure = 1015;
  val->humidity = 60.3;
  val->temp_ext = 7.9;
  val->wind_dir =  180;
  val->wind_speed = 7.1;
  delay(2);

#else
  char status;
  double int_temperature, int_pressure;
  float heading;
  float tmp;
  unsigned long start_time, end_time;
  start_time = millis();

  TCCR1B = 0x6;
  end_time = millis() + 1000 * speed_measure_time;
  wdt_reset();
  sensors_init();

  val->vbat = get_vbat();
  val->lum = analogRead(LDR_PIN);
  val->rain = analogRead(RAIN_PIN);

  status = pressure.startTemperature();
  if (status != 0)
  {
    wdt_reset();
    delay(status);
    status = pressure.getTemperature(int_temperature);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        wdt_reset();
        delay(status);
        status = pressure.getPressure(int_pressure, int_temperature);
        if (status != 0)
        {
          val->temp_int = int_temperature;
          val->pressure = int_pressure;
        }
      }
    }
  }
  tmp = dht.readHumidity();
  if ( !isnan(tmp) ) {
    val->humidity = tmp;
  }
  tmp = dht.readTemperature();
  if ( !isnan(tmp) ) {
    val->temp_ext = tmp;
  }
#ifdef USE_ADAFRUIT_MAG_LIB
  sensors_event_t event;
  mag.getEvent(&event);
  heading = atan2(event.magnetic.y, event.magnetic.x);
#else
  MagnetometerScaled scaled = compass.ReadScaledAxis(); //scaled values from compass.
  heading = atan2(scaled.YAxis, scaled.XAxis);
#endif
  heading += HEADING_MECHANICAL_CORRECTION;
  // Correct for when signs are reversed.
  if (heading < 0) heading += 2 * PI;
  if (heading > 2 * PI) heading -= 2 * PI;
  val->wind_dir =  int(heading * RAD_TO_DEG);

  while (millis() < end_time) {
    wdt_reset();
  }
  val->wind_speed = TCNT1 * (60 / speed_measure_time) * rpm_2_ms;
  TCCR1B = 0x0; TCNT1 = 0;
#endif
}

