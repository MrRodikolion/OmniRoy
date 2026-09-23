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

void setMotorSpeed(byte motor, int speed);

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

  TCCR2A |= (1 << WGM21);     // CTC mode
  TCCR2B |= (1 << CS22);      // prescaler 64
  OCR2A = 249;                // 1 kHz interrupt at 16 MHz: 16e6 / (64 * (249 + 1))
  TIMSK2 |= (1 << OCIE2A);

  sei();
}

void loop()
{
  static char data[32];
  static byte index = 0;

  while (Serial.available() > 0)
  {
    char ch = Serial.read();

    if (ch == '\n' || ch == '\r')
    {
      if (index > 0)
      {
        data[index] = '\0';

        char *comma = strchr(data, ',');
        if (comma != nullptr)
        {
          *comma = '\0';
          char *motorStr = data;
          char *speedStr = comma + 1;

          int motorNumber = atoi(motorStr);
          int speed = atoi(speedStr);

          if (motorNumber >= 1 && motorNumber <= 4)
          {
            setMotorSpeed(motorNumber - 1, speed);
          }

          Serial.print(motorNumber);
          Serial.print(',');
          Serial.println(speed);
        }

        index = 0;
      }
    }
    else
    {
      if (index < sizeof(data) - 1)
      {
        data[index++] = ch;
      }
    }
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

void setMotorSpeed(byte motor, int speed)
{
  if (motor >= 4)
  {
    return;
  }

  speed = constrain(speed, -255, 255);

  byte forwardChannel = motor * 2;
  byte reverseChannel = forwardChannel + 1;

  noInterrupts();

  if (speed > 0)
  {
    pinD[forwardChannel] = speed;
    pinD[reverseChannel] = 0;
  }
  else if (speed < 0)
  {
    pinD[forwardChannel] = 0;
    pinD[reverseChannel] = -speed;
  }
  else
  {
    pinD[forwardChannel] = 0;
    pinD[reverseChannel] = 0;
  }

  pinC[forwardChannel] = 0;
  pinC[reverseChannel] = 0;

  interrupts();
}
