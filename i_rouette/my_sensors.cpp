#include "config.h"
#include "board.h"
#include <HMC5883L.h>
#include <SFE_BMP180.h>
#include "DHT.h"
#include "my_sensors.h"
#include <avr/wdt.h>


DHT dht(DHT_PIN, DHT22);
SFE_BMP180 pressure;
HMC5883L compass;


void sensors_init() {
  pressure.begin();
  dht.begin();
  compass = HMC5883L();
  compass.SetScale(1.3);
  compass.SetMeasurementMode(Measurement_Continuous);
}

void sensors_update(struct SensorValues *val, int speed_measure_time, float rpm_2_ms) {
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

  val->vbat = (2.5 * 1024) / analogRead(REF25_PIN);
  val->vbat = (2.5 * 1024) / analogRead(REF25_PIN);

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

  MagnetometerScaled scaled = compass.ReadScaledAxis(); //scaled values from compass.
  heading = atan2(scaled.YAxis, scaled.XAxis);

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
}

