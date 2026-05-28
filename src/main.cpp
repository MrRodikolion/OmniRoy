#include <Arduino.h>
#include <math.h>
#include "avr/interrupt.h"
#include "avr/io.h"

volatile byte TPIN[] = {12, 11,
                        8, 9,
                        7, 5,
                        4, 3}; // B C D A
volatile byte pinD[] = {0, 0,
                        0, 0,
                        0, 0,
                        0, 0};
volatile byte pinC[] = {0, 0, 0, 0, 0, 0, 0, 0};

float x, y;
float angleR;

void moveMotor(int angle, int v);

void setup()
{
  for (int i = 0; i < 8; i++)
  {
    pinMode(TPIN[i], OUTPUT);
  }

  Serial.begin(1150200);

  cli();
  TCCR2A = 0;
  TCCR2B = 0;

  TCCR2B = 0 << CS22 | 0 << CS21 | 1 << CS20;
  TIMSK2 |= (1 << OCIE2A);

  sei();
}

void loop()
{
  if (Serial.available() > 0)
  {
    String data = Serial.readStringUntil('\n');
    
    int commaIndex = data.indexOf(',');
    if (commaIndex > 0)
    {
      x = data.substring(0, commaIndex).toFloat();
      y = data.substring(commaIndex + 1).toFloat();
      
      float baseAngle = 0;
      if (x == 0 && y == 0)
      {
        angleR = 0;
      }
      else if (x == 0)
      {
        angleR = (y > 0) ? PI / 2 : 3 * PI / 2;
      }
      else if (y == 0)
      {
        angleR = (x > 0) ? 0 : PI;
      }
      else
      {
        baseAngle = atan(fabs(y) / fabs(x));
        if (x > 0 && y > 0)
        {
          angleR = baseAngle;
        }
        else if (x < 0 && y > 0)
        {
          angleR = PI - baseAngle;
        }
        else if (x < 0 && y < 0)
        {
          angleR = PI + baseAngle;
        }
        else
        {
          angleR = 2 * PI - baseAngle;
        }
      }
    }
  }

  moveMotor(angleR, 100);
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

void moveMotor(int angle, int v)
{
  int n = 0;
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
