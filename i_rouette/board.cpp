#include "Arduino.h"
#include "config.h"
#include "prescaler.h"
#include "board.h"

void sensor_enable(bool en)
{
  if (en) {
    digitalWrite(VCC_EN_PIN, 1);
    delay(2000);
  } else {
    digitalWrite(VCC_EN_PIN, 0);
  }
}

void radio_enable(bool en)
{
  if (en) {
    digitalWrite(VCC_EN_PIN, 0);
    delay(1000);
    Serial.end();
    Serial.begin(RADIO_BAUD);
    digitalWrite(VCC_EN_PIN, 1);

    delay(2000);
  } else {
    Serial.end();
    pinMode(TX_PIN, OUTPUT);
    digitalWrite(VCC_EN_PIN, 0);
  }
}

enum ChargeState get_charge_status(void) {
  int val = 0;
  if (digitalRead(CHARGING_PIN) == HIGH) val = 2;
  if (digitalRead(CHARGED_PIN) == HIGH) val += 1;
  return (enum ChargeState)(val);
}

void board_init(void)
{
  uint8_t i;
  cpu_8MHZ();
  pinMode(VCC_EN_PIN, OUTPUT);
  sensor_enable(false);
  radio_enable(false);
  pinMode(LED0_PWM_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED_TAIL, OUTPUT);
  pinMode(CHARGING_PIN, INPUT);
  pinMode(CHARGED_PIN, INPUT);
  pinMode(WAKEUP_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);


  // Timer 1 will be used for couting RPM wind speed, disabling its default arduino PWM usage
  TCCR1B = 0x0;
  TCNT1 = 0;

  // Timer 2 as PWM only when in idle
  TCCR2B = (1 << CS20);

  for (i = 0; i < 5; i++) {
    led_tail(true);
    delay(200);
    led_tail(false);
    delay(200);
  }
  beep(5, false);
}

int adc_band_gap_code(void)
{
  uint8_t low, high;
  uint8_t admux = ADMUX;

  ADMUX = (0 << 6) | (14 & 0x0F);
  // start the conversion
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  // start the conversion
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  low  = ADCL;
  high = ADCH;
  ADMUX = admux;
  return (high << 8) | low;
}
void cpu_8MHZ(void)
{
  setClockPrescaler(CLOCK_PRESCALER_1);
}

void cpu_250KHZ(void)
{
  setClockPrescaler(CLOCK_PRESCALER_32);
}

void beep(uint8_t nb_beep, bool len)
{
  uint8_t i;

  for (i = 0; i < nb_beep; i++) {
    digitalWrite(BUZZER_PIN, 1);
    if (len) {
      delay(100);
    } else {
      delay(20);
    }
    digitalWrite(BUZZER_PIN, 0);
    delay(100);
  }

}

void led_tail(bool en)
{
  if (en) {
    digitalWrite(LED_TAIL, 1);
  } else {
    digitalWrite(LED_TAIL, 0);
  }
}

void all_led_off(void)
{
  led_tail(false);
}

