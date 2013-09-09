//Serial Sync Variables
//0		Temp Sensor * 100
//1		Temp Sensor * 100
//2		Temp Sensor * 100
//3		Temp Sensor * 100
//4		Light Sensor
//5		UNUSED
//6		Temp Min
//7		Temp Max
//8		Fans (Bit Field)
//9-11		BRG
//12-14		BRG
//15-17		BRG
//18-20		BRG
//21		Gas Sensor
//22		Motion Sensor
//23		Key Sensor
//24		Temp Desired 0
//25		Temp Desired 1
//26		Temp Desired 2
//27		Temp Desired 3

//Serial Sync Header
#include "serial_sync.h"

//Temperature Sensor Header
#include "TempSensor.h"

//Temperature Sensor ID Number (index 0 is a place for invalid)
const uint8_t temp_id_size=1+4;

//Temperature Sensor ID Array
byte temp_id[temp_id_size][8]=
{
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x10,0xd8,0x75,0xc2,0x02,0x08,0x00,0x26},
  {0x10,0x28,0x7a,0xc2,0x02,0x08,0x00,0x3b},
  {0x10,0x0c,0x76,0xc2,0x02,0x08,0x00,0x70},
  {0x10,0x5e,0x8c,0xc2,0x02,0x08,0x00,0x2b}
};

//Temperature Sensor Value Array
float temp_values[temp_id_size]={0.0,0.0,0.0,0.0,0.0};

//Temperature Reader Variables
uint8_t temp_pin=3;
TempSensor temp_reader(temp_pin);
long temp_timer=0;

//RGB Light Variables
byte ws2803_clock_pin=7;
byte ws2803_data_pin=8;
const byte ws2803_num_leds=12;
uint8_t ws2803_bar[ws2803_num_leds];

//Heater Variables
byte heater_pin=2;

//Fan Variables
const uint8_t fan_size=4;
byte fans[fan_size]={11,10,12,13};

//Lock Variables
uint8_t lock_pin=6;

//Gas Sensor Variables
uint8_t gas_pin=A0;

//Motion Sensor Variables
uint8_t motion_pin=A1;

//Light Sensor Variables
uint8_t light_pin=A4;

//Serial Sync Controller
msl::serial_sync ss(Serial,9600);

//Setup Function
void setup()
{
  //RGB Light Setup
  pinMode(ws2803_clock_pin,OUTPUT);
  pinMode(ws2803_data_pin,OUTPUT);
  ws2803_setup();
  
  //Heater Setup
  pinMode(heater_pin,OUTPUT);
  digitalWrite(heater_pin,LOW);

  //Fan Setup
  for(int ii=0;ii<fan_size;++ii)
  {
    pinMode(fans[ii],OUTPUT);
    digitalWrite(fans[ii],HIGH);
  }

  //Lock Setup
  pinMode(lock_pin,INPUT);

  //Gas Sensor Setup
  pinMode(gas_pin,INPUT);

  //Motion Sensor Setup
  pinMode(motion_pin,INPUT);
  
  //Light Sensor Setup
  pinMode(light_pin,INPUT);

  //Serial Sync Setup
  ss.setup();
}

//Loop Function
void loop()
{
  //Update Temperatures
  temp_reader.loop(temp_id_size,temp_values,temp_id);

  //Update Serial Sync RX Side
  ss.update_rx();

  //Send Temperature Values
  if(millis()>temp_timer)
  {
    ss.set(0,temp_values[1]*100);
    ss.set(1,temp_values[2]*100);
    ss.set(2,temp_values[3]*100);
    ss.set(3,temp_values[4]*100);
    temp_timer=millis()+500;
  }

  //Send Light Value
  ss.set(4,analogRead(light_pin));

  //Get RGB Light Values
  for(int ii=0;ii<12;++ii)
    ws2803_bar[ii]=ss.get(ii+9);

  //Get Temperature Min Value
  if(ss.get(6)!=60)
    ss.set(6,60);

  //Get Temperature Max Value
  if(ss.get(7)!=80)
    ss.set(7,80);

  //Clamp Desired Temperature Values
  for(uint8_t ii=24;ii<28;++ii)
  {
    if(ss.get(ii)<ss.get(6))
      ss.set(ii,ss.get(6));
    if(ss.get(ii)>ss.get(7))
      ss.set(ii,ss.get(7));
  }

  //Regulate Room Temperatures
  for(int room=0;room<4;++room)
  {
    if(ss.get(room)/100.0>ss.get(24+room))
      digitalWrite(fans[room],LOW);

    if(ss.get(room)/100.0<ss.get(24+room))
      digitalWrite(fans[room],HIGH);
  }

  //Turn Heater On or Off
  digitalWrite(heater_pin,!fans_to_bitfield());

  //Update RGB Lights
  ws2803_loop();

  //Send Fan Values
  ss.set(8,fans_to_bitfield());

  //Send Gas Sensor Value
  ss.set(21,analogRead(gas_pin));

  //Send Motion Sensor Value
  ss.set(22,!digitalRead(motion_pin));

  //Send Lock Value
  ss.set(23,digitalRead(lock_pin));

  //Update Serial Sync TX Side
  ss.update_tx();
}

//WS2803 Setup Function
void ws2803_setup()
{
  //Pull Clock Low
  digitalWrite(ws2803_clock_pin,LOW);

  //Delay (Must for Protocol)
  delayMicroseconds(600);

  //Set Lights
  for(int ii=0;ii<ws2803_num_leds;++ii)
    ws2803_bar[ii]=0;

  //Update Lights
  ws2803_loop();
}

//WS2803 Loop Function
void ws2803_loop()
{
    //Update Lights
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[3]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[4]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[5]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[0]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[1]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[2]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[9]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[10]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[11]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[6]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[7]);
    shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[8]);

    //Delay (Must for Protocol)
    delayMicroseconds(1000);
}

//Fan to Bitfield Function (Creates Bitfield from fan pins)
uint8_t fans_to_bitfield()
{
  //Return Value Variable
  uint8_t ret=0x00;

  //Populate Bitfield
  for(int ii=0;ii<fan_size;++ii)
    ret|=(digitalRead(fans[ii])<<ii);

  //Return Bitfield
  return ret;
}
