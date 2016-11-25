#ifndef __EEPROM_ESP_H__
#define __EEPROM_ESP_H__

#include <EEPROM.h>


class Eeprom_esp
{
  public:
    Eeprom_esp(uint16_t size, uint8_t str_size);
    bool clear(void);
    bool read_string(uint16_t offset, char *str);
    bool write_string(uint16_t offset, char *str);
    bool read_int( uint16_t offset, int *val);
    bool write_int( uint16_t offset, int val);
    bool read_float( uint16_t offset, float *val);
    bool write_float( uint16_t offset, float val);

  private:
    uint16_t _size;
    uint8_t _str_size;
};


#endif
