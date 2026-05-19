#include <Arduino.h>

// Omnidirectional 4-wheel robot control with software PWM on 8 digital pins.
// The robot accepts coordinate targets from USB serial and drives toward them.

const uint8_t MOTOR_COUNT = 4;
const uint8_t PWM_PIN_COUNT = 8;

const uint8_t pwmPins[PWM_PIN_COUNT] = {2, 4, 7, 8, 12, 13, A0, A1};
const uint8_t pwmPortIndex[PWM_PIN_COUNT] = {0, 0, 0, 1, 1, 1, 2, 2};
const uint8_t pwmPortMask[PWM_PIN_COUNT] = {
  _BV(PORTD2), _BV(PORTD4), _BV(PORTD7),
  _BV(PORTB0), _BV(PORTB4), _BV(PORTB5),
  _BV(PORTC0), _BV(PORTC1)
};

volatile uint8_t pwmValues[PWM_PIN_COUNT] = {0};
volatile uint8_t pwmCounter = 0;

float currentX = 0.0f;
float currentY = 0.0f;
float targetX = 0.0f;
float targetY = 0.0f;
bool moveToTarget = false;

float velX = 0.0f;
float velY = 0.0f;
float velR = 0.0f;

unsigned long lastMotionMillis = 0;

char inputBuffer[64];
uint8_t inputPos = 0;

const float kMaxVelocity = 0.8f;
const float kCoordinateSpeed = 40.0f;      // coordinate units per second when moving toward target
const float kArrivalThreshold = 2.0f;      // coordinate units to consider arrived

void applyMotorSpeed(uint8_t motor, int16_t speedValue);
void updateAllMotors();
void processSerialLine(const char *line);
void setTargetPosition(float x, float y);
void setVelocity(float x, float y, float r);
void stopMotion();
void updateMotionState(float dt);

ISR(TIMER2_COMPA_vect) {
  pwmCounter++;
  uint8_t outD = 0;
  uint8_t outB = 0;
  uint8_t outC = 0;

  for (uint8_t i = 0; i < PWM_PIN_COUNT; i++) {
    if (pwmCounter < pwmValues[i]) {
      switch (pwmPortIndex[i]) {
        case 0: outD |= pwmPortMask[i]; break;
        case 1: outB |= pwmPortMask[i]; break;
        case 2: outC |= pwmPortMask[i]; break;
      }
    }
  }

  PORTD = (PORTD & ~(_BV(PORTD2) | _BV(PORTD4) | _BV(PORTD7))) | outD;
  PORTB = (PORTB & ~(_BV(PORTB0) | _BV(PORTB4) | _BV(PORTB5))) | outB;
  PORTC = (PORTC & ~(_BV(PORTC0) | _BV(PORTC1))) | outC;
}

void setupTimer2() {
  noInterrupts();
  TCCR2A = (1 << WGM21);          // CTC mode, OCR2A top
  TCCR2B = (1 << CS21);           // prescaler 8
  OCR2A = 124;                    // 16 MHz / 8 / 125 = 16 kHz interrupt
  TIMSK2 = (1 << OCIE2A);         // enable compare match A interrupt
  TCNT2 = 0;
  interrupts();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  for (uint8_t i = 0; i < PWM_PIN_COUNT; i++) {
    pinMode(pwmPins[i], OUTPUT);
    digitalWrite(pwmPins[i], LOW);
  }

  lastMotionMillis = millis();
  updateAllMotors();
  setupTimer2();

  Serial.println(F("Omni 4-wheel control ready. Send commands like: GOTO x y, VEL x y, ROT r, STOP"));
  Serial.println(F("Coordinates range -100..100, velocity range -100..100"));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      inputBuffer[inputPos] = '\0';
      if (inputPos > 0) {
        processSerialLine(inputBuffer);
      }
      inputPos = 0;
    } else if (inputPos + 1 < sizeof(inputBuffer)) {
      inputBuffer[inputPos++] = c;
    }
  }

  unsigned long now = millis();
  float dt = (now - lastMotionMillis) * 0.001f;
  if (dt > 0.0f) {
    updateMotionState(dt);
    lastMotionMillis = now;
  }
}

void processSerialLine(const char *line) {
  char buffer[64];
  strncpy(buffer, line, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  char *token = strtok(buffer, " \t");
  if (token == NULL) {
    return;
  }

  if (strcasecmp(token, "STOP") == 0 || strcasecmp(token, "ZERO") == 0) {
    stopMotion();
    Serial.println(F("STOP"));
    return;
  }

  float x = 0.0f;
  float y = 0.0f;
  float r = 0.0f;
  bool parsed = false;
  bool coordinateCommand = false;
  bool velocityCommand = false;

  if (strcasecmp(token, "GOTO") == 0 || strcasecmp(token, "MOVE") == 0) {
    char *px = strtok(NULL, " \t");
    char *py = strtok(NULL, " \t");
    if (px && py) {
      x = atof(px);
      y = atof(py);
      coordinateCommand = true;
      parsed = true;
    }
  } else if (strcasecmp(token, "VEL") == 0 || strcasecmp(token, "XY") == 0) {
    char *px = strtok(NULL, " \t");
    char *py = strtok(NULL, " \t");
    char *pr = strtok(NULL, " \t");
    if (px && py) {
      x = atof(px);
      y = atof(py);
      if (pr) {
        r = atof(pr);
      }
      velocityCommand = true;
      parsed = true;
    }
  } else if (strcasecmp(token, "ROT") == 0) {
    char *pr = strtok(NULL, " \t");
    if (pr) {
      r = atof(pr);
      velocityCommand = true;
      parsed = true;
    }
  } else if (strcasecmp(token, "POS") == 0) {
    Serial.print(F("POS "));
    Serial.print(currentX, 2);
    Serial.print(F(","));
    Serial.println(currentY, 2);
    return;
  } else {
    char *px = token;
    char *py = strtok(NULL, " \t");
    if (py) {
      x = atof(px);
      y = atof(py);
      coordinateCommand = true;
      parsed = true;
    }
  }

  if (!parsed) {
    Serial.print(F("Unknown command: "));
    Serial.println(line);
    Serial.println(F("Examples: GOTO 50 0, MOVE 50 0, VEL 0.5 0.0, ROT 30, STOP"));
    return;
  }

  if (coordinateCommand) {
    x = constrain(x, -100.0f, 100.0f);
    y = constrain(y, -100.0f, 100.0f);
    setTargetPosition(x, y);
    Serial.print(F("TARGET "));
    Serial.print(x, 2);
    Serial.print(F(","));
    Serial.println(y, 2);
  } else {
    x = constrain(x, -100.0f, 100.0f) / 100.0f;
    y = constrain(y, -100.0f, 100.0f) / 100.0f;
    r = constrain(r, -100.0f, 100.0f) / 100.0f;
    setVelocity(x, y, r);
    Serial.print(F("VELOCITY X="));
    Serial.print(x, 2);
    Serial.print(F(" Y="));
    Serial.print(y, 2);
    Serial.print(F(" R="));
    Serial.println(r, 2);
  }
}

void setTargetPosition(float x, float y) {
  targetX = x;
  targetY = y;
  moveToTarget = true;
  updateMotionState(0.0f);
}

void setVelocity(float x, float y, float r) {
  velX = x;
  velY = y;
  velR = r;
  moveToTarget = false;
  updateAllMotors();
}

void stopMotion() {
  moveToTarget = false;
  velX = 0;
  velY = 0;
  velR = 0;
  updateAllMotors();
}

void updateMotionState(float dt) {
  if (moveToTarget) {
    float dx = targetX - currentX;
    float dy = targetY - currentY;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist <= kArrivalThreshold) {
      currentX = targetX;
      currentY = targetY;
      stopMotion();
      Serial.print(F("ARRIVED "));
      Serial.print(currentX, 2);
      Serial.print(F(","));
      Serial.println(currentY, 2);
    } else {
      float nx = dx / dist;
      float ny = dy / dist;
      velX = nx * kMaxVelocity;
      velY = ny * kMaxVelocity;
      velR = 0.0f;
      updateAllMotors();

      if (dt > 0.0f) {
        float travel = kCoordinateSpeed * dt;
        if (travel > dist) {
          travel = dist;
        }
        currentX += nx * travel;
        currentY += ny * travel;
      }
    }
  } else {
    if (dt > 0.0f) {
      currentX += velX * kCoordinateSpeed * dt;
      currentY += velY * kCoordinateSpeed * dt;
    }
  }
}

void updateAllMotors() {
  const float invSqrt2 = 0.70710678f;

  float vx = velX;
  float vy = velY;
  float omega = velR;

  float m[MOTOR_COUNT];
  m[0] = invSqrt2 * (vx + vy) + omega;
  m[1] = invSqrt2 * (-vx + vy) - omega;
  m[2] = invSqrt2 * (-vx - vy) + omega;
  m[3] = invSqrt2 * (vx - vy) - omega;

  float maxValue = 0.0f;
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    float absValue = fabs(m[i]);
    if (absValue > maxValue) {
      maxValue = absValue;
    }
  }
  if (maxValue > 1.0f) {
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
      m[i] /= maxValue;
    }
  }

  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    int16_t pwm = (int16_t)round(m[i] * 127.0f);
    applyMotorSpeed(i, pwm);
  }
}

void applyMotorSpeed(uint8_t motor, int16_t speedValue) {
  if (motor >= MOTOR_COUNT) {
    return;
  }

  uint8_t pwmIndexA = motor * 2;
  uint8_t pwmIndexB = pwmIndexA + 1;

  if (speedValue > 0) {
    uint8_t duty = (uint8_t)constrain(speedValue, 0, 127);
    pwmValues[pwmIndexA] = duty;
    pwmValues[pwmIndexB] = 0;
  } else if (speedValue < 0) {
    uint8_t duty = (uint8_t)constrain(-speedValue, 0, 127);
    pwmValues[pwmIndexA] = 0;
    pwmValues[pwmIndexB] = duty;
  } else {
    pwmValues[pwmIndexA] = 0;
    pwmValues[pwmIndexB] = 0;
  }
}
