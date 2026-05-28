#include <Arduino.h>
#include "avr/interrupt.h"
#include "avr/io.h"

volatile byte TPIN[] = {12, 11,
                        8, 9,
                        7, 5,
                        4, 3}; // B C D A
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
      pinD[i]=j;
    }
    delay(10);
  }
  for (int j = 255; j > 0; j--)
  {
    for (int i = 0; i < 8; i += 2)
    {
      pinD[i]=j;
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
