#ifndef SMARTHOUSE_TEMPSENSOR_H
#define SMARTHOUSE_TEMPSENSOR_H

#include <Arduino.h>

#include "OneWire.h"

class TempSensor
{
	public:
		TempSensor(const byte pin);
		void loop(const int size,float* values,byte ids[][8]);

	private:
		bool _compare_ids(byte* lhs,byte* rhs);

		OneWire _ds;
		int _current_id;
		long _timer;
		bool _reading;
		byte _data[12];
		byte _addr[8];
};

#endif
