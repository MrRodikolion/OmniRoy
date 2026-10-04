#include <Arduino.h>
#include <math.h>
#include <avr/interrupt.h>

void moveRobot(float x, float y);
void stopRobot();

// Определение пинов для 4 моторов (драйвер L9110S)
const byte MOTOR_PINS[4][2] = {
    {15, 16}, // Мотор A (Пара AC)
    {1, 2}, // Мотор C (Пара AC)
    {12, 13}, // Мотор B (Пара BD)
    {8, 9}  // Мотор D (Пара BD)
};

// Переменные для программного ШИМ
volatile int target_speeds[4] = {0, 0, 0, 0}; 
volatile byte pwm_counter = 0;

// Прерывание Таймера 2 — генерация программного ШИМ
ISR(TIMER2_COMPB_vect) {
    pwm_counter++;
    
    for (byte i = 0; i < 4; i++) {
        int speed = target_speeds[i];
        
        if (speed == 0) {
            digitalWrite(MOTOR_PINS[i][0], LOW);
            digitalWrite(MOTOR_PINS[i][1], LOW);
        } 
        else if (speed > 0) {
            digitalWrite(MOTOR_PINS[i][1], LOW);
            if (pwm_counter < (byte)speed) {
                digitalWrite(MOTOR_PINS[i][0], HIGH);
            } else {
                digitalWrite(MOTOR_PINS[i][0], LOW);
            }
        } 
        else {
            digitalWrite(MOTOR_PINS[i][0], LOW);
            if (pwm_counter < (byte)abs(speed)) {
                digitalWrite(MOTOR_PINS[i][1], HIGH);
            } else {
                digitalWrite(MOTOR_PINS[i][1], LOW);
            }
        }
    }
}

void setup() {
    // Настройка пинов моторов
    for (byte i = 0; i < 4; i++) {
        pinMode(MOTOR_PINS[i][0], OUTPUT);
        pinMode(MOTOR_PINS[i][1], OUTPUT);
    }

    // Инициализация USB-соединения (Serial) на скорость 115200 бод
    Serial.begin(115200);

    // Настройка Таймера 2
    TCCR2A = 0; 
    TCCR2B = (0 << CS22) | (0 << CS21) | (1 << CS20); 
    TIMSK2 |= (1 << OCIE2B); 
    sei(); 
}

void loop() {
    // Проверяем, пришли ли данные от компьютера по USB
    if (Serial.available() > 0) {
        // Читаем координату X (до запятой)
        float x = Serial.parseFloat(); 
        
        // Читаем координату Y (до символа новой строки)
        float y = Serial.parseFloat(); 

        // Очищаем буфер от символа '\n' или лишних знаков
        while (Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
            Serial.read();
        }

        // Мертвая зона для защиты от шумов джойстика/ошибок
        if (abs(x) < 0.1 && abs(y) < 0.1) {
            stopRobot();
        } else {
            moveRobot(x, y);
        }
    }
}

// Расчет скоростей по кинематической схеме
void moveRobot(float x, float y) {
    float alpha = atan2(y, x);
    if (alpha < 0) alpha += 2 * M_PI;

    float n = 0;
    float speed_AC = 0;
    float speed_BD = 0;
    float V_MAX = 255.0;

    if (alpha >= 0 && alpha <= M_PI / 2) {
        n = tan(alpha - M_PI / 4);
        speed_AC = V_MAX;      
        speed_BD = n * V_MAX;  
    } 
    else if (alpha > M_PI / 2 && alpha <= M_PI) {
        n = tan(3 * M_PI / 4 - alpha);
        speed_AC = n * V_MAX;  
        speed_BD = V_MAX;      
    } 
    else if (alpha > M_PI && alpha <= 3 * M_PI / 2) {
        n = tan(5 * M_PI / 4 - alpha);
        speed_AC = -V_MAX;     
        speed_BD = n * V_MAX;  
    } 
    else {
        n = tan(alpha - 7 * M_PI / 4);
        speed_AC = n * V_MAX;  
        speed_BD = -V_MAX;     
    }

    // Передаем скорости в прерывание
    target_speeds[0] = (int)constrain(speed_AC, -255, 255); // Мотор A
    target_speeds[1] = (int)constrain(speed_AC, -255, 255); // Мотор C
    target_speeds[2] = (int)constrain(speed_BD, -255, 255); // Мотор B
    target_speeds[3] = (int)constrain(speed_BD, -255, 255); // Мотор D
}

void stopRobot() {
    for (byte i = 0; i < 4; i++) {
        target_speeds[i] = 0;
    }
}