#ifndef SMARTHOUSE_TEMPSENSOR_H
#define SMARTHOUSE_TEMPSENSOR_H

#include <Arduino.h>

#include "OneWire.h"

class TempSensor
{
	public:
		TempSensor(const uint8_t pin);
		void loop(const int16_t size,float* values,uint8_t ids[][8]);

	private:
		bool _compare_ids(uint8_t* lhs,uint8_t* rhs);

		OneWire _ds;
		int16_t _current_id;
		int32_t _timer;
		bool _reading;
		uint8_t _data[12];
		uint8_t _addr[8];
};

#endif
