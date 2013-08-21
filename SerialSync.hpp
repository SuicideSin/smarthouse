//GK12 Serial Sync Header
//	Created By:	Mike Moss
//	Modified On:	04/22/2013

#ifndef SERIAL_SYNC_H
#define SERIAL_SYNC_H

#include "msl/serial.hpp"
#include <string>
#include <inttypes.h>

#define SERIAL_SYNC_DATA_SIZE 64

class SerialSync
{
	public:
		SerialSync(const std::string& port_name,const unsigned int baud);
		uint16_t get(const uint8_t index) const;
		void set(const uint8_t index,const uint16_t value);
		void setup();
		void loop();

	private:
		std::string _name;
		unsigned int _baud;
		msl::serial _port;
		uint16_t _data[SERIAL_SYNC_DATA_SIZE];
		uint8_t _current_index;
		uint8_t _packet_pointer;
		uint16_t _temp_data;
};

#endif