#include <SerialSyncArduino.h>

SerialSync ss(Serial,9600);

void setup()
{
  ss.setup();
}

void loop()
{
  ss.loop();

  ss.set(0,analogRead(A0));
  ss.set(1,analogRead(A1));
  ss.set(2,analogRead(A2));
  ss.set(3,analogRead(A3));
  ss.set(4,analogRead(A4));
}
