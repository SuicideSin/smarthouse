#include "SerialSyncArduino.h"
#include "TempSensor.h"

long temp_timer=0;
const int temp_id_size=1+4;
float temp_values[temp_id_size];
byte temp_id[temp_id_size][8]=
{
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x10,0x28,0x7a,0xc2,0x02,0x08,0x00,0x3b},
  {0x10,0xd8,0x75,0xc2,0x02,0x08,0x00,0x26},
  {0x10,0x0c,0x76,0xc2,0x02,0x08,0x00,0x70},
  {0x10,0x5e,0x8c,0xc2,0x02,0x08,0x00,0x2b}
};

byte ws2803_clock_pin=7;
byte ws2803_data_pin=8;
const byte ws2803_num_leds=12;
uint8_t ws2803_bar[ws2803_num_leds];

byte light_relay_pin=2;

SerialSync ss(Serial,9600);

void setup()
{
  pinMode(3,INPUT);
  pinMode(ws2803_clock_pin,OUTPUT);
  pinMode(ws2803_data_pin,OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(light_relay_pin,OUTPUT);

  digitalWrite(10,LOW);
  digitalWrite(11,LOW);
  digitalWrite(12,LOW);
  digitalWrite(13,LOW);
  digitalWrite(light_relay_pin,LOW);

  ss.setup();
  
  ss.set(0,75);
  ss.set(1,75);
  ss.set(2,75);
  ss.set(3,75);

  ws2803_setup();
}

void loop()
{
  temperature_update(3,temp_id_size,temp_values,temp_id);
  
  ss.loop();

  if(millis()>temp_timer)
  {
    ss.set(0,temp_values[1]*100);
    ss.set(1,temp_values[2]*100);
    ss.set(2,temp_values[3]*100);
    ss.set(3,temp_values[4]*100);
    temp_timer=millis()+500;
  }

  ss.set(4,analogRead(A4));

  for(int ii=0;ii<12;++ii)
    ws2803_bar[ii]=ss.get(ii+9);

  if(ss.get(6)!=60)
    ss.set(6,60);
    
  if(ss.get(7)!=80)
    ss.set(7,80);

  if(ss.get(5)<ss.get(6))
    ss.set(5,ss.get(6));
  
  if(ss.get(5)>ss.get(7))
    ss.set(5,ss.get(7));
  
  for(int room=0;room<4;++room)
  {
    if(ss.get(room)/100.0>ss.get(5))
      digitalWrite(10+room,LOW);
    
    if(ss.get(room)/100.0<ss.get(5))
      digitalWrite(10+room,HIGH);
  }

  if(!digitalRead(10)&&!digitalRead(11)&&!digitalRead(12)&&!digitalRead(13))
    digitalWrite(light_relay_pin,HIGH);
  else
    digitalWrite(light_relay_pin,LOW);

  ws2803_loop();
}

void ws2803_setup()
{
  digitalWrite(ws2803_clock_pin,LOW);
  delayMicroseconds(600);

  for(int ii=0;ii<ws2803_num_leds;++ii)
    ws2803_bar[ii]=0;

  ws2803_loop();
}

void ws2803_loop()
{
    for(int ii=0;ii<ws2803_num_leds;++ii)
      shiftOut(ws2803_data_pin,ws2803_clock_pin,MSBFIRST,ws2803_bar[ii]);

    delayMicroseconds(1000);
}
