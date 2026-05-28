#include <Arduino.h>
#include "avr/interrupt.h"
#include "avr/io.h"

volatile byte TPIN[] = {7, 5, 
                        4, 3,
                        12, 11,
                        8, 9,
                        }; // B C D A - > D A B C 
volatile byte pinD[] = {100, 0,
                        0, 100,
                        0, 100,
                        100, 0};
volatile byte pinC[] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
  for (int i = 0; i < 8; i++)
  {
    pinMode(TPIN[i], OUTPUT);
  }

  cli();
  TCCR2A = 0;
  TCCR2B = 0;

  TCCR2B = 0 << CS22 | 0 << CS21 | 1 << CS20;
  TIMSK2 |= (1 << OCIE2A);

  sei();
}

void loop()
{
  for (int j = 0; j < 255; j++)
  {
    for (int i = 0; i < 8; i += 2)
    {
      pinD[i] = j;
    }
    delay(10);
  }
  for (int j = 255; j > 0; j--)
  {
    for (int i = 0; i < 8; i += 2)
    {
      pinD[i] = j;
    }
    delay(10);
  }
}

ISR(TIMER2_COMPA_vect)
{
  for (int i = 0; i < 8; i++)
  {
    if (pinC[i] == 0 && pinD[i] > 0)
    {
      digitalWrite(TPIN[i], HIGH);
    }
    if (pinC[i] == pinD[i])
    {
      digitalWrite(TPIN[i], LOW);
    }
    pinC[i]++;
  }
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
  if (angle > PI / 2 || angle < 3 * PI / 4)
  {
    pinD[0] = 2 * v;
    pinD[1] = 0;
    pinD[2] = 2 * n * v;
    pinD[3] = 0;
    pinD[4] = 2 * v;
    pinD[5] = 0;
    pinD[6] = 2 * n * v;
    pinD[7] = 0;
  }
  else if (angle > 3 * PI / 4 || angle < PI)
  {
    pinD[0] = 2 * v;
    pinD[1] = 2 * n * v;
    pinD[2] = 0;
    pinD[3] = 0;
    pinD[4] = 2 * v;
    pinD[5] = 0;
    pinD[6] = 0;
    pinD[7] = 2 * n * v;
  }
}
else if (angle > PI || angle < 3 * PI / 2)
{
  n = tan(5 * PI / 4 - angle) * v;
  if (angle > PI || angle < 5 * PI / 4)
  {
    pinD[0] = 2 * n * v;
    pinD[1] = 0;
    pinD[2] = 0;
    pinD[3] = 2 * v;
    pinD[4] = 2 * n * v;
    pinD[5] = 0;
    pinD[6] = 0;
    pinD[7] = 2 * v;
  }
  else if (angle > 5 * PI / 4 || angle < 3 * PI / 2)
  {
    pinD[0] = 0;
    pinD[1] = 2 * n * v;
    pinD[2] = 0;
    pinD[3] = 2 * v;
    pinD[4] = 0;
    pinD[5] = 2 * n * v;
    pinD[6] = 0;
    pinD[7] = 2 * v;
  }
}
else if (angle > 3 * PI / 2 || angle < 2 * PI)
{
  n = tan(angle - 7 * PI / 4) * v;
  if (angle > 3 * PI / 2 || angle < 7 * PI / 4)
  {
    pinD[0] = 0;
    pinD[1] = 2 * v;
    pinD[2] = 0;
    pinD[3] = 2 * n * v;
    pinD[4] = 0;
    pinD[5] = 2 * v;
    pinD[6] = 0;
    pinD[7] = 2 * n * v;
  }
  else if (angle > 7 * PI / 4 || angle < 2 * PI)
  {
    pinD[0] = 0;
    pinD[1] = 2 * v;
    pinD[2] = 2 * n * v;
    pinD[3] = 0;
    pinD[4] = 0;
    pinD[5] = 2 * v;
    pinD[6] = 2 * n * v;
    pinD[7] = 0;
  }
}
}
