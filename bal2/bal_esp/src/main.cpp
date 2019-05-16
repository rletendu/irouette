#include <Arduino.h>
#include <SoftwareSerialCustom.h>
#include <LowPower.h>

#define FRONT_DOOR_PIN 1
#define FRONT_DOOR_PC 1
#define BACK_DOOR_PIN 2
#define BACK_DOOR_PC 2
#define RX_PIN 3
#define TX_PIN 4
#define ESP_SERIAL_BAUD 9600
#define ESP_EN_PIN 5
#define DELAY_CONNECT 3000

volatile bool mail_update_request = true;
volatile uint16_t count_mail = 0;
unsigned long time;
unsigned long time_last_mail_update = 0;
SoftwareSerialCustom ESP_Serial = SoftwareSerialCustom(RX_PIN, TX_PIN);

#define ESP_Enable() digitalWrite(ESP_EN_PIN, HIGH)
#define ESP_Disable() digitalWrite(ESP_EN_PIN, LOW)

int getVCC()
{
  ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85
  delay(2);
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC))
    ;
  uint8_t low = ADCL;
  unsigned int val = (ADCH << 8) | low;
  //discard previous result
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC))
    ;
  low = ADCL;
  val = (ADCH << 8) | low;
  return ((long)1024 * 1100) / val;
}

bool send_mail_count_update()
{
  int vcc;

  ESP_Enable();
  vcc = getVCC();
  delay(DELAY_CONNECT);
  
  ESP_Serial.print("{MAIL:");
  ESP_Serial.print(count_mail);
  ESP_Serial.print(",BAT:");
  ESP_Serial.print(vcc);
  ESP_Serial.print("}");

  if (ESP_Serial.available())
  {
    ESP_Serial.read();
  }

  ESP_Disable();
}

void UserPCINT0_vect()
{
  if (digitalRead(BACK_DOOR_PIN) == 0)
  {
    count_mail = 0;
    mail_update_request = true;
    return;
  }
  if (digitalRead(FRONT_DOOR_PIN) == 1)
  {
    count_mail++;
    mail_update_request = true;
    return;
  }
}

void setup()
{
  // Init interupt pins detection
  // Back door mail box detection is micro swtich Normaly Open
  // Opening the door will close the switch
  pinMode(BACK_DOOR_PIN, INPUT_PULLUP);

  PCMSK |= (1 << BACK_DOOR_PC) | (1 << FRONT_DOOR_PC);
  GIMSK |= (1 << PCIE);
  // Front door mail box detection is micro reelswtich Normaly Close
  // Opening the door will open the switch
  pinMode(FRONT_DOOR_PIN, INPUT); //! Caution no internal pull up, external one only !

  ESP_Disable();
  pinMode(ESP_EN_PIN, OUTPUT);

  ESP_Serial.begin(ESP_SERIAL_BAUD);
}

void loop()
{
  if (mail_update_request)
  {
    send_mail_count_update();
    mail_update_request = false;
  }
  // put your main code here, to run repeatedly:
}