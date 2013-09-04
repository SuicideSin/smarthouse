//Serial Sync Header
//	Created By:		Mike Moss
//	Modified On:	08/28/2013

//ARDUINO VERSION

//Begin Define Guards
#ifndef MSL_SERIALSYNC_H
#define MSL_SERIALSYNC_H

//Arduino Header
#include <Arduino.h>

//Integer Standard Types Header
#include <inttypes.h>

//Serial Header
#include <HardwareSerial.h>

//Number of Serial Variables
#define MSL_SERIALSYNC_VARIABLES 85

//MSL Namespace
namespace msl
{
	//Serial Sync Class Declaration
	class serial_sync
	{
		public:
			//Constructor (Default)
			serial_sync(HardwareSerial& serial,const uint32_t baud=57600);

			//Setup Function (Sets up serial port)
			void setup();

			//Update RX Function (Receives updates over link)
			void update_rx();

			//Update TX Function (Sends updates over link)
			void update_tx();

			//Get Function (Gets a value from a variable)
			int16_t get(const uint8_t index);

			//Set Function (Sets a variable to a value)
			void set(const uint8_t index,const int16_t value);

		private:
			//Calculate CRC Function (XORs all bytes together)
			uint8_t calculate_crc(const uint8_t* buffer,const uint8_t size) const;

			//Member Variables
			uint32_t _baud;
			HardwareSerial* _serial;
			int16_t _data[MSL_SERIALSYNC_VARIABLES];
			uint8_t _flags[MSL_SERIALSYNC_VARIABLES];
			uint8_t _tx_packet[3+1+MSL_SERIALSYNC_VARIABLES*3+1];
			uint8_t _rx_packet[3+1+MSL_SERIALSYNC_VARIABLES*3+1];
			uint32_t _rx_counter;
	};
}

//End Define Guards
#endif