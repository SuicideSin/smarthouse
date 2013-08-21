#include <SerialSyncArduino.h>

byte ws2803_clock_pin=7;
byte ws2803_data_pin=8;
const byte ws2803_num_leds=12;
uint8_t ws2803_bar[ws2803_num_leds];
long temp_timer=millis()+100;

byte light_relay_pin=2;

SerialSync ss(Serial,9600);

void setup()
{
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

  ws2803_setup();
}

void loop()
{
  ss.loop();

  ss.set(0,analogRead(A0));
  ss.set(1,analogRead(A1));
  ss.set(2,analogRead(A2));
  ss.set(3,analogRead(A3));
  ss.set(4,analogRead(A4));

  for(int ii=0;ii<12;++ii)
    ws2803_bar[ii]=ss.get(ii+9);

  digitalWrite(10,ss.get(5));
  digitalWrite(11,ss.get(6));
  digitalWrite(12,ss.get(7));
  digitalWrite(13,ss.get(8));

  if(!digitalRead(10)&&!digitalRead(11)&&!digitalRead(12)&&!digitalRead(13))
    digitalWrite(light_relay_pin,LOW);
  else
    digitalWrite(light_relay_pin,HIGH);

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
