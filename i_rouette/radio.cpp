#include "Arduino.h"
#include "config.h"
#include "board.h"
#include "radio.h"

extern char   buff[BUFF_MAX];

void radio_reset(void)
{
  vcc_sensor_enable(false);
  delay(1000);
  vcc_sensor_enable(true);
  while (Serial.available() > 0) {
    Serial.read();
  }
}

bool radio_connect(void )
{
  buff[0] = 0;
  uint8_t attempts = RADIO_CONNECT_ATTEMPT;

  while (attempts) {
    Serial.println(":C");
    Serial.flush();
    if (get_radio_status(TIMEOUT_CONNECT, buff) == RADIO_OK) {
      return true;
    }
    if (--attempts) {
      radio_reset();
    }
  }
  return false;
}

bool radio_sleep(void )
{
  Serial.println(":S");
  Serial.flush();
  if (get_radio_status(TIMEOUT_PING, buff) == RADIO_OK) {
    return true;
  }
  return false;
}

bool radio_ping(void)
{
  Serial.println(":p");
  Serial.flush();
  if (get_radio_status(TIMEOUT_PING, buff) == RADIO_OK) {
    return true;
  }
  return false;
}

bool radio_sync(void)
{
  uint8_t sync_attempts = RADIO_SYNC_ATTEMPT;

  while (sync_attempts) {
    if (radio_ping() == true ) {
      return true;
    }
    if (--sync_attempts) {
      radio_reset();
    }
  }
  return false;
}
bool radio_disconnect(void )
{

}

bool radio_send(char *data )
{
  Serial.println(data);
  Serial.flush();
  if (get_radio_status(TIMEOUT_TX, data) == RADIO_OK) {
    return true;
  }
  return false;
}

bool radio_get_param(char *data)
{
  Serial.println(":P");
  Serial.flush();
  if (get_radio_status(TIMEOUT_RX, data) == RADIO_OK ) {
    return true;
  }
  return false;
}

enum radio_status get_radio_status(long timeout, char *d)
{
  uint8_t nb;
  d[0] = 0;

  Serial.setTimeout(timeout);
  nb = Serial.readBytesUntil('\n', d, BUFF_MAX);
  if (nb > 0) {
    if (buff[0] == 'o') {
      return RADIO_OK;
    } else {
      return RADIO_FAIL;
    }
  }
  return RADIO_FAIL;
}

