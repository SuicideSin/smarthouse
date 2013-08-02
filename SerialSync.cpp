//GK12 Serial Sync Source
//	Created By:	Mike Moss
//	Modified On:	04/22/2013

#include "SerialSync.hpp"

#include <stdexcept>

SerialSync::SerialSync(const std::string& port_name,const unsigned int baud):
	_name(port_name),_baud(baud),_port(_name,_baud),_current_index(0),_packet_pointer(0),_temp_data(0)
{
	for(unsigned int ii=0;ii<SERIAL_SYNC_DATA_SIZE;++ii)
		_data[ii]=0;
}

uint16_t SerialSync::get(const uint8_t index) const
{
	return _data[index];
}

void SerialSync::set(const uint8_t index,const uint16_t value)
{
	_data[index]=value;
	_port<<"uaf"<<index;
	_port.write((uint8_t*)&value,2);
}

void SerialSync::setup()
{
	_port.connect();

	if(!_port.good())
		throw std::runtime_error("SerialSync::setup() - Invalid serial port!");

	uint8_t update_packet[4]={'u','a','f','!'};
	_port.write(update_packet,4);
}

void SerialSync::loop()
{
	while(_port.available()>0)
	{
		uint8_t read_byte;

		if(_port.read((uint8_t*)&read_byte,1)==1)
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
				if(read_byte=='!')
				{
					for(unsigned int ii=0;ii<SERIAL_SYNC_DATA_SIZE;++ii)
						set(ii,get(ii));

					_packet_pointer=0;
				}
				else if(_current_index<SERIAL_SYNC_DATA_SIZE)
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
				_temp_data=0;
				_packet_pointer=0;
			}
		}
	}
}