#include <Arduino.h>
#include "SoftwareSerialCustom.h"
#include "LowPower.h"


#define FRONT_DOOR_PIN 2
#define FRONT_DOOR_PC 2
#define BACK_DOOR_PIN 1
#define BACK_DOOR_PC 1
#define RX_PIN 3
#define TX_PIN 4
#define ESP_SERIAL_BAUD 9600
#define ESP_EN_PIN 0
#define DELAY_CONNECT 10000


#define RXTIMEOUT 5000
#define RX_ATTEMPT 1

#define ACK_MESSAGE_LEN 3
const char ack_message[ACK_MESSAGE_LEN + 1] = "ACK";
volatile bool mail_update_request = true;
volatile uint16_t count_mail = 0;
unsigned long time;
unsigned long time_last_mail_update = 0;
SoftwareSerialCustom ESP_Serial = SoftwareSerialCustom(RX_PIN, TX_PIN);

#define ESP_Enable() digitalWrite(ESP_EN_PIN, HIGH)
#define ESP_Disable() digitalWrite(ESP_EN_PIN, LOW)
#define Enable_Door_IT() PCMSK |= (1 << BACK_DOOR_PC) | (1 << FRONT_DOOR_PC)
#define Disable_Door_IT() PCMSK &= ~((1 << BACK_DOOR_PC) | (1 << FRONT_DOOR_PC))

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

bool receive_ack()
{
  uint8_t idx = 0;
  char rx_char;
  unsigned long timeout = millis() + RXTIMEOUT;

  while (idx < ACK_MESSAGE_LEN)
  {
    if (millis() > timeout)
    {
      return false;
      break;
    }
    if (ESP_Serial.available())
    {
      rx_char = ESP_Serial.read();
      if (rx_char == ack_message[idx])
      {
        idx++;
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

void send_mail_count_update()
{
  int vcc;
  uint8_t attempt = 0;
  vcc = getVCC();
  bool status = false;

  while (attempt < RX_ATTEMPT)
  {
    ESP_Enable();
    delay(DELAY_CONNECT);
    ESP_Serial.print("{MAIL:");
    ESP_Serial.print(count_mail);
    ESP_Serial.print(",BAT:");
    ESP_Serial.print(vcc);
    ESP_Serial.print("}");
    ESP_Serial.listen();
    status = receive_ack();
    ESP_Serial.stopListening();
    ESP_Disable();
    if (status)
    {
      status = true;
    }
    delay(100);
    attempt++;
  }
}

void UserPCINT0_vect()
{
  if (mail_update_request == 0)
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
}

void setup()
{
  // Init interupt pins detection
  // Back door mail box detection is micro swtich Normaly Open
  // Opening the door will close the switch
  pinMode(BACK_DOOR_PIN, INPUT_PULLUP);

  Enable_Door_IT();
  GIMSK |= (1 << PCIE);
  // Front door mail box detection is micro reelswtich Normaly Close
  // Opening the door will open the switch
  pinMode(FRONT_DOOR_PIN, INPUT); //! Caution no internal pull up, external one only !

  ESP_Enable();
  pinMode(ESP_EN_PIN, OUTPUT);
  delay(1000);
  ESP_Disable();
  ESP_Serial.begin(ESP_SERIAL_BAUD);
  ESP_Serial.stopListening();
  //ESP_Serial.println("Setup");
}

void loop()
{
  if (mail_update_request)
  {
    //ESP_Serial.println("Up");
    Disable_Door_IT();
    send_mail_count_update();
    mail_update_request = false;
    Enable_Door_IT();
  }
  ESP_Serial.println("Sleep");
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
