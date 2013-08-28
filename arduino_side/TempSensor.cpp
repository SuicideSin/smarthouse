#include "TempSensor.h"

#include "OneWire.h"

static int current_id=0;
static long temp_timer=0;
static bool temp_reading=false;
static byte data[12];
static byte addr[8];

static bool temperature_compare_ids(byte* lhs,byte* rhs)
{
  bool match=true;

  for(int ii=0;ii<8;++ii)
  {
    if(lhs[ii]!=rhs[ii])
    {
      match=false;
      break;
    }
  }

  return match;
}

void temperature_update(int pin,int temp_id_size,float* temp_values,byte temp_id[][8])
{
  static OneWire ds(pin);
  
  if(!temp_reading)
  {
    if(ds.search(addr)&&OneWire::crc8(addr,7)==addr[7]&&addr[0]==0x10)
    {
      bool found=false;

      for(int ii=1;ii<temp_id_size;++ii)
      {
        if(temperature_compare_ids(addr,temp_id[ii]))
        {
          current_id=ii;
          found=true;
          break;
        }
      }

      if(found)
      {
        ds.reset();
        ds.select(addr);
        ds.write(0x44, 1);
        temp_timer=millis()+1000;
        temp_reading=true;
      }
    }
    else
    {
      ds.reset_search();
    }
  }

  if(temp_reading&&millis()>=temp_timer)
  {
    ds.reset();
    ds.select(addr);
    ds.write(0xBE);

    for (int ii=0;ii<9;++ii)
      data[ii]=ds.read();

    OneWire::crc8(data, 8);

    int16_t raw = (data[1] << 8) | data[0];
    raw = raw << 3;
    if (data[7] == 0x10)
      raw = (raw & 0xFFF0) + 12 - data[6];
    temp_values[current_id]= raw / 16.0 * 1.8 + 32.0;
    temp_reading=false;
    current_id=0;
  }
}
