//GK12 Serial Sync Arduino Source
//	Created By:	Mike Moss
//	Modified On:	04/22/2013

#include "SerialSyncArduino.h"

SerialSync::SerialSync(HardwareSerial& serial,const unsigned int baud):_serial(&serial),
  _baud(baud),_current_index(0),_packet_pointer(0),_temp_data(0),_time(millis())
{
  for(unsigned int ii=0;ii<SERIAL_SYNC_DATA_SIZE;++ii)
  {
    _data[ii]=0;
    _change[ii]=false;
  }
}

uint16_t SerialSync::get(const uint8_t index) const
{
  return _data[index];
}

void SerialSync::set(const uint8_t index,const uint16_t value)
{
  _data[index]=value;
  _change[index]=true;
}

void SerialSync::setup()
{
  _serial->begin(_baud);
}

void SerialSync::loop()
{
  if(_serial->available()>0)
  {
    uint8_t read_byte;

    if(_serial->readBytes((char*)&read_byte,1)==1)
    {
      if(_packet_pointer==0)
      {
        if(read_byte=='u')
          ++_packet_pointer;
      }
      else if(_packet_pointer==1)
      {
        if(read_byte=='a')
          ++_packet_pointer;
        else
          _packet_pointer=0;
      }
      else if(_packet_pointer==2)
      {
        if(read_byte=='f')
          ++_packet_pointer;
        else
          _packet_pointer=0;
      }
      else if(_packet_pointer==3)
      {
        if(_current_index<SERIAL_SYNC_DATA_SIZE)
        {
          _current_index=read_byte;
          ++_packet_pointer;
        }
        else
        {
          _packet_pointer=0;
        }
      }
      else if(_packet_pointer==4)
      {
        ((uint8_t*)&_temp_data)[0]=read_byte;
        ++_packet_pointer;
      }
      else if(_packet_pointer==5)
      {
        ((uint8_t*)&_temp_data)[1]=read_byte;
        _data[_current_index]=_temp_data;
        _packet_pointer=0;
      }
    }
  }

  if(millis()>=_time+20)
  {
    for(unsigned int ii=0;ii<SERIAL_SYNC_DATA_SIZE;++ii)
    {
      if(_change[ii])
      {
        _serial->write('u');
        _serial->write('a');
        _serial->write('f');
        _serial->write(ii);
        _serial->write((uint8_t*)&_data[ii],2);
        _change[ii]=false;
      }
    }

    _time=millis();
  }
}
