//GK12 Serial Sync Arduino Header
//	Created By:	Mike Moss
//	Modified On:	04/22/2013

#ifndef SERIAL_SYNC_H
#define SERIAL_SYNC_H

#include <Arduino.h>
#include <HardwareSerial.h>

#define SERIAL_SYNC_DATA_SIZE 64

class SerialSync
{
  public:
    SerialSync(HardwareSerial& serial,const unsigned int baud);
    uint16_t get(const uint8_t index) const;
    void set(const uint8_t index,const uint16_t value);
    void setup();
    void loop();

  private:
    HardwareSerial* _serial;
    unsigned int _baud;
    uint16_t _data[SERIAL_SYNC_DATA_SIZE];
    bool _change[SERIAL_SYNC_DATA_SIZE];
    uint8_t _current_index;
    uint8_t _packet_pointer;
    uint16_t _temp_data;
    long _time;
};

#endif
