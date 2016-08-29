#include "Arduino.h"
#include "config.h"
#include "board.h"

extern char   buff[BUFF_MAX];


bool radio_connect(void )
{
  uint8_t trial;
  char nb;
  buff[0] = 0;

  Serial.setTimeout(TIMEOUT_CONNECT);
  while (Serial.available() > 0)
  {
    Serial.read();
  }
  Serial.flush();
  Serial.println(":C");

  nb = Serial.readBytesUntil('\n', buff, BUFF_MAX);
  if (nb > 0) {
    if (buff[0] == 'o') {
      return true;
    }
  }
  return false;
}

bool radio_sleep(void )
{

  char nb;
  buff[0] = 0;
  Serial.println(":S");
  Serial.flush();
  Serial.setTimeout(TIMEOUT_CONNECT);
  nb = Serial.readBytesUntil('\n', buff, BUFF_MAX);
  if (nb > 0) {
    if (buff[0] == 'o') {
      return true;
    }
  }

  return false;
}

bool radio_disconnect(void )
{

}

bool radio_send(char *data )
{

  char nb;

  Serial.setTimeout(TIMEOUT_TX);
  Serial.println(data);
  Serial.flush();
  data[0] = 0;
  nb = Serial.readBytesUntil('\n', data, BUFF_MAX);
  if (nb > 0) {
    if (data[0] == 'o') {
      return true;
    }
  }
  return false;
}

bool radio_get_param(char *data)
{

  char nb;
  data[0] = 0;

  Serial.setTimeout(TIMEOUT_RX);
  Serial.println(":P");
  Serial.flush();
  nb = Serial.readBytesUntil('\n', data, BUFF_MAX);
  if (nb > 0) {
    if (data[0] == 'o') {
      return true;
    }
  }
}

