#include "Arduino.h"
#include "eeprom_esp.h"
#include <EEPROM.h>

Eeprom_esp::Eeprom_esp(uint16_t size, uint8_t str_size)
{
  _str_size = str_size;
  _size = size;
  EEPROM.begin(size);
}


bool Eeprom_esp::clear(void)
{
  uint16_t i;
  for (i = 0; i < _size; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}


bool Eeprom_esp::read_string(uint16_t offset, char *str )
{
  uint8_t i;
  for (i = 0; i < _str_size; i++) {
    str[i] =  char(EEPROM.read(i + offset));
  }
}

bool Eeprom_esp::write_string(uint16_t offset, char *str)
{
  uint8_t i;
  for (i = 0; i < _str_size; i++) {
    EEPROM.write(i + offset, str[i]);
  }
  EEPROM.commit();
}

bool Eeprom_esp::read_int( uint16_t offset, int *val)
{
  char tmp[_str_size];
  read_string(offset, tmp);
  if (strlen(tmp)) {
    *val = atoi(tmp);
    return true;
  } else return false;
}

bool Eeprom_esp::write_int( uint16_t offset, int val)
{
  char tmp[_str_size];
  snprintf(tmp, _str_size, "%d", val);
  write_string(offset, tmp);
  return true;
}

bool Eeprom_esp::read_float( uint16_t offset, float *val)
{
  char tmp[_str_size];
  read_string(offset, tmp);
  if (strlen(tmp)) {
    *val = atof(tmp);
    return true;
  } else return false;
}

bool Eeprom_esp::write_float( uint16_t offset, float val)
{
  char tmp[_str_size];
  dtostrf(val, 3, 1, tmp);
  write_string(offset, tmp);
  return true;
}

