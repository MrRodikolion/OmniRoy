#include <Arduino.h>
#include <math.h>
#include "avr/interrupt.h"
#include "avr/io.h"

volatile byte TPIN[] = {7, 5, 
                        4, 3,
                        12, 11,
                        8, 9,
                        }; // B C D A - > D A B C 
volatile byte pinD[] = {0, 0,
                        0, 0,
                        0, 0,
                        0, 0};
volatile byte pinC[] = {0, 0, 0, 0, 0, 0, 0, 0};

float x, y;
float angleR = 0.0f;

void moveMotor(int angle, int v);

void setup()
{
  for (int i = 0; i < 8; i++)
  {
    pinMode(TPIN[i], OUTPUT);
  }

  Serial.begin(115200);

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
  delay(100);
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

void moveMotor(float angle, int v)
{
  int n = 0;
  if (angle >= 0.0f && angle < PI / 2.0f)
  {
    n = tan(angle - PI / 4.0f) * v;
    if (angle >= 0.0f && angle < PI / 4.0f)
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
    else if (angle >= PI / 4.0f && angle < PI / 2.0f)
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
  else if (angle >= PI / 2.0f && angle < PI)
  {
    n = tan(3.0f * PI / 4.0f - angle) * v;
    if (angle >= PI / 2.0f && angle < 3.0f * PI / 4.0f)
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
    else if (angle >= 3.0f * PI / 4.0f && angle < PI)
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
  else if (angle >= PI && angle < 3.0f * PI / 2.0f)
  {
    n = tan(5.0f * PI / 4.0f - angle) * v;
    if (angle >= PI && angle < 5.0f * PI / 4.0f)
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
    else if (angle >= 5.0f * PI / 4.0f && angle < 3.0f * PI / 2.0f)
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
  else if (angle >= 3.0f * PI / 2.0f && angle < 2.0f * PI)
  {
    n = tan(angle - 7.0f * PI / 4.0f) * v;
    if (angle >= 3.0f * PI / 2.0f && angle < 7.0f * PI / 4.0f)
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
    else if (angle >= 7.0f * PI / 4.0f && angle < 2.0f * PI)
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
