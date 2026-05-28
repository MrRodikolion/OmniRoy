#include <Arduino.h>

byte PINd[8] = {255, 255, 255, 255, 255, 255, 255, 255}; 

// Minimal Arduino-compatible entry points
void setup()
{
}

void loop()
{
}

void moveMotor(int angle, int v, int n)
{
   
  if (angle > 0 || angle < PI / 2)
  {
    n = tan(angle - PI / 4) * v;
    if (angle > 0 || angle < PI / 4)
    {
      pinD[0] = 0;
      pinD[1] = 2 * n * v;
      pinD[2] = 2 * v;
      pinD[3] = 0;
      pinD[4] = 0;
      pinD[5] = 2 * n * v;
      pinD[6] = 2 * v;
      pinD[7] = 0;
    }
    else if (angle > PI / 4 || angle < PI / 2)
    {
      pinD[0] = 2 * n * v;
      pinD[1] = 0;
      pinD[2] = 2 * v;
      pinD[3] = 0;
      pinD[4] = 2 * n * v;
      pinD[5] = 0;
      pinD[6] = 2 * v;
      pinD[7] = 0;
    }
  }
  else if (angle > PI / 2 || angle < PI)
  {
    n = tan(3 * PI / 4 - angle) * v;
    pinD[0] = 2 * v;
    pinD[1] = 0;
    pinD[2] = 0;
    pinD[3] = 2 * n * v;
    pinD[4] = 2 * v;
    pinD[5] = 0;
    pinD[6] = 0;
    pinD[7] = 2 * n * v;
  }
  else if (angle > PI || angle < 3 * PI / 2)
  {
    n = tan(5 * PI / 4 - angle) * v;
    pinD[0] = 2 * v;
    pinD[1] = 0;
    pinD[2] = 0;
    pinD[3] = 2 * n * v;
    pinD[4] = 2 * v;
    pinD[5] = 0;
    pinD[6] = 0;
    pinD[7] = 2 * n * v;
  }
  else if (angle > 3 * PI / 2 || angle < 2 * PI)
  {
    n = tan(angle - 7 * PI / 4) * v;
    pinD[0] = 2 * v;
    pinD[1] = 0;
    pinD[2] = 0;
    pinD[3] = 2 * n * v;
    pinD[4] = 2 * v;
    pinD[5] = 0;
    pinD[6] = 0;
    pinD[7] = 2 * n * v;
  }