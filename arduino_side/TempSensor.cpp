#include "TempSensor.h"

TempSensor::TempSensor(const uint8_t pin):_ds(pin),_current_id(0),_timer(0),_reading(false)
{
	pinMode(pin,INPUT);

	for(uint8_t ii=0;ii<12;++ii)
		_data[ii]=0x00;

	for(uint8_t ii=0;ii<8;++ii)
		_addr[ii]=0x00;
}

void TempSensor::loop(const int16_t size,float* values,uint8_t ids[][8])
{
	if(!_reading)
	{
		bool type_check=(_addr[0]==0x10);
		bool crc_check=(OneWire::crc8(_addr,7)==_addr[7]);

		if(_ds.search(_addr))
		{
			bool type_check=(_addr[0]==0x10);
			bool crc_check=(OneWire::crc8(_addr,7)==_addr[7]);

			if(type_check&&crc_check)
			{
				bool found_id=false;

				for(int16_t ii=1;ii<size;++ii)
				{
					if(_compare_ids(_addr,ids[ii]))
					{
						_current_id=ii;
						found_id=true;
						break;
					}
				}

				if(found_id)
				{
					_ds.reset();
					_ds.select(_addr);
					_ds.write(0x44, 1);
					_timer=millis()+1000;
					_reading=true;
				}
			}
		}
		else
		{
			_ds.reset_search();
		}
	}

	if(_reading&&millis()>=_timer)
	{
		_ds.reset();
		_ds.select(_addr);
		_ds.write(0xBE);

		for(uint8_t ii=0;ii<9;++ii)
			_data[ii]=_ds.read();

		OneWire::crc8(_data,8);

		int16_t raw=*(int16_t*)&_data[0];
		raw=raw<<3;
		raw=(raw&0xFFF0)+12-_data[6];
		values[_current_id]=raw/16.0*1.8+32.0;
		_reading=false;
		_current_id=0;
	}
}

bool TempSensor::_compare_ids(uint8_t* lhs,uint8_t* rhs)
{
	bool match=true;

	for(int16_t ii=0;ii<8;++ii)
	{
		if(lhs[ii]!=rhs[ii])
		{
			match=false;
			break;
		}
	}

	return match;
}
