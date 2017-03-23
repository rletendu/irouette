#ifndef OREGON_H
#define OREGON_H

#include "Arduino.h"

//#define OREGON_DEBUG
// Setup debug printing macros.
#ifdef OREGON_DEBUG
// Define where debug output will be printed.
#ifndef DEBUG_OREGON_PRINTER
#define DEBUG_OREGON_PRINTER Serial
#endif
#define DEBUG_OREGON_PRINT(...) { DEBUG_OREGON_PRINTER.print(__VA_ARGS__); }
#define DEBUG_OREGON_PRINTLN(...) { DEBUG_OREGON_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_OREGON_PRINT(...) {}
#define DEBUG_OREGON_PRINTLN(...) {}
#endif

#define OREGON_MAX_MESSAGE_SIZE 11


class Oregon
{
  public:
    Oregon(void);
    void begin(byte tx_pin);
    void end(void);
    void send_temperature(uint8_t ch, uint8_t id, float temperature, byte battery);
    void send_temperature_hum(uint8_t ch, uint8_t id, float temperature, byte hum, byte battery);

  private:
    byte OregonMessage[OREGON_MAX_MESSAGE_SIZE];
    byte OregonMessageSize;
    byte OregonTxPin;
    void sendMessage(void);
    void setTemperature(float temp);
    void calculateAndSetChecksum(void);
    void setHumidity(byte hum);
    void setPressure(float pres);
    int Sum(byte count);
    inline void sendZero(void);
    inline void sendOne(void);
    void sendOregon(void);
    inline void sendQuarterLSB(const byte data);
    inline void sendQuarterMSB(const byte data);
    inline void sendPreamble(void);
    inline void sendPostamble(void);
    
    

    
};

// Very Scrappy way to add modules in arduino subfolder ...
// But the only one found using ARDUINO IDE
#ifdef ARDUINO

#endif

#endif //OREGON_H
