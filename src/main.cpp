#include <Arduino.h>
#include <math.h>
#include "avr/interrupt.h"
#include "avr/io.h"

// TODO
/*
Калибровочные коэфициенты для моторов и откалибровать мотры

*/

constexpr uint8_t TPIN[] = {7, 5,
                            4, 3,
                            11, 12,
                            9, 8}; // D A B C

constexpr float h = 0.0435f;
constexpr float MAX_PSI = 20.0f;
constexpr float MAX_SPEED = 0.5f;
constexpr float SPEED_RAMP_RATE = 0.5f;
constexpr uint16_t MOTOR_UPDATE_MS = 10U;
constexpr uint16_t PWM_TIMER_COMPARE = 15U;
constexpr uint16_t PWM_TIMER_PRESCALER = 64U;
constexpr uint16_t PWM_MAX_VALUE = 255U;

volatile uint8_t pinD[] = {0, 0,
                           0, 0,
                           0, 0,
                           0, 0};
volatile uint8_t pinC[] = {0, 0, 0, 0, 0, 0, 0, 0};

float speed = 0.0f;
float currentSpeed = 0.0f;
float angle = 90.0f;

void setMotorSpeed(byte motor, int speed);
void setMotorSpeeds(const int speeds[4]);
void driveByAngle(float V_angle, float speed);

static inline void setPinFast(uint8_t pin, bool level)
{
  const uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN)
  {
    return;
  }

  volatile uint8_t *out = portOutputRegister(port);
  const uint8_t mask = digitalPinToBitMask(pin);

  if (level)
  {
    *out |= mask;
  }
  else
  {
    *out &= static_cast<uint8_t>(~mask);
  }
}

void setup()
{
  for (uint8_t i = 0; i < 8; ++i)
  {
    pinMode(TPIN[i], OUTPUT);
    digitalWrite(TPIN[i], LOW);
  }

  Serial.begin(115200);

  cli();
  TCCR2A = 0;
  TCCR2B = 0;

  TCCR2A |= (1 << WGM21);    // CTC mode
  TCCR2B |= (1 << CS22);     // prescaler 64
  OCR2A = PWM_TIMER_COMPARE; // ~15.6 kHz ISR: 16e6 / (64 * (15 + 1))
  TIMSK2 |= (1 << OCIE2A);

  sei();
}

void loop()
{
  static unsigned long lastUpdate = 0;
  const unsigned long now = millis();

  if (now - lastUpdate >= MOTOR_UPDATE_MS)
  {
    lastUpdate = now;
    const float maxSpeedStep = SPEED_RAMP_RATE * MOTOR_UPDATE_MS / 1000.0f;
    currentSpeed += constrain(speed - currentSpeed, -maxSpeedStep, maxSpeedStep);
    driveByAngle(radians(angle), currentSpeed);
  }

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
          char *angleStr = data;
          char *speedStr = comma + 1;

          angle = atof(angleStr);
          speed = constrain(atof(speedStr), 0.0f, MAX_SPEED);
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
  for (uint8_t i = 0; i < 8; ++i)
  {
    if (pinC[i] == 0 && pinD[i] > 0)
    {
      setPinFast(TPIN[i], HIGH);
    }
    if (pinC[i] == pinD[i])
    {
      setPinFast(TPIN[i], LOW);
    }
    pinC[i]++;
  }
}

void setMotorSpeed(uint8_t motor, int speed)
{
  if (motor >= 4)
  {
    return;
  }

  speed = constrain(speed, -255, 255);

  const uint8_t forwardChannel = motor * 2;
  const uint8_t reverseChannel = forwardChannel + 1;

  noInterrupts();

  if (speed > 0)
  {
    pinD[forwardChannel] = speed;
    pinD[reverseChannel] = 0;
  }
  else if (speed < 0)
  {
    pinD[forwardChannel] = 0;
    pinD[reverseChannel] = static_cast<uint8_t>(-speed);
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

  for (uint8_t motor = 0; motor < 4; ++motor)
  {
    const int value = constrain(speeds[motor], -255, 255);
    const uint8_t forwardChannel = motor * 2;
    const uint8_t reverseChannel = forwardChannel + 1;

    if (value > 0)
    {
      pinD[forwardChannel] = value;
      pinD[reverseChannel] = 0;
    }
    else if (value < 0)
    {
      pinD[forwardChannel] = 0;
      pinD[reverseChannel] = static_cast<uint8_t>(-value);
    }
    else
    {
      pinD[forwardChannel] = 0;
      pinD[reverseChannel] = 0;
    }
  }

  interrupts();
}

static inline int rad2PWM(float psi)
{
  const float ratio = psi / MAX_PSI;
  int pwm = static_cast<int>(round(ratio * static_cast<float>(PWM_MAX_VALUE)));

  if (pwm > static_cast<int>(PWM_MAX_VALUE))
    return static_cast<int>(PWM_MAX_VALUE);
  if (pwm < -static_cast<int>(PWM_MAX_VALUE))
    return -static_cast<int>(PWM_MAX_VALUE);
  return pwm;
}

void driveByAngle(float V_angle, float speed)
{
  const float Vx = speed * cos(V_angle);
  const float Vy = speed * sin(V_angle);

  const float psi_13 = (Vx - Vy) / h;
  const float psi_24 = -(Vx + Vy) / h;

  const int motorSpeeds[4] = {
      rad2PWM(psi_13),
      rad2PWM(psi_24),
      rad2PWM(psi_13),
      rad2PWM(psi_24)};

  setMotorSpeeds(motorSpeeds);
}