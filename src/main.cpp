#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
  int helloworld();
  // What happens if I put code after the return statement? heeloworld() will never be called because the function will exit immediately after the return statement. Any code placed after a return statement in a function will not be executed.
} 