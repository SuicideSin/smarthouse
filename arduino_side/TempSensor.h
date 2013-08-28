#ifndef SMARTHOUSE_TEMPSENSOR_H
#define SMARTHOUSE_TEMPSENSOR_H

#include <Arduino.h>

void temperature_update(int pin,int temp_id_size,float* temp_values,byte temp_id[][8]);

#endif
