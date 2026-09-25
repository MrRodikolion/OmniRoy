#include <Arduino.h>
#include <math.h>
#include "avr/interrupt.h"
#include "avr/io.h"

volatile byte TPIN[] = {
    7,
    5,
    4,
    3,
    12,
    11,
    8,
    9,
}; // B C D A - > D A B C
volatile byte pinD[] = {0, 0,
                        0, 0,
                        0, 0,
                        0, 0};
volatile byte pinC[] = {0, 0, 0, 0, 0, 0, 0, 0};

const float h = 0.0435f;

void setMotorSpeed(byte motor, int speed);
void setMotorSpeeds(const int speeds[4]);
void driveByAngle(float V_angle, float speed);

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

  TCCR2A |= (1 << WGM21); // CTC mode
  TCCR2B |= (1 << CS22);  // prescaler 64
  OCR2A = 249;            // 1 kHz interrupt at 16 MHz: 16e6 / (64 * (249 + 1))
  TIMSK2 |= (1 << OCIE2A);

  sei();
}

void loop()
{
  static unsigned long lastUpdate = 0;
  const unsigned long now = millis();

  if (now - lastUpdate >= 20)
  {
    lastUpdate = now;
    driveByAngle(radians(45), 0.5);
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

  interrupts();
}

void setMotorSpeeds(const int speeds[4])
{
  noInterrupts();

  for (byte motor = 0; motor < 4; motor++)
  {
    const int speed = constrain(speeds[motor], -255, 255);
    const byte forwardChannel = motor * 2;
    const byte reverseChannel = forwardChannel + 1;

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
  }

  interrupts();
}

const float MAX_PSI = 20.0f;
int rad2PWM(float psi)
{
  float ratio = psi / MAX_PSI;
  int pwm = round(ratio * 255);

  if (pwm > 255) return 255;
  if (pwm < -255) return -255;
  return pwm;
}

void driveByAngle(float V_angle, float speed)
{
  float Vx = speed * cos(V_angle);
  float Vy = speed * sin(V_angle);

  float psi_13 = (Vx - Vy) / h;
  float psi_24 = -(Vx + Vy) / h;

  const int motorSpeeds[4] = {
      rad2PWM(psi_13),
      rad2PWM(psi_24),
      rad2PWM(psi_13),
      rad2PWM(psi_24)};
  setMotorSpeeds(motorSpeeds);
}