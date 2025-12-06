// --- SAME HEADERS AS BEFORE ---
#include <ESP32Servo.h>
#define SERVO1 23
#define TRIG 5
#define ECHO 16
#include <SSD1306Wire.h>
SSD1306Wire lcd(0x3c, SDA, SCL);

// --- STEPPER MOTOR SETUP ---
#define IN1 15
#define IN2 13
#define IN3 12
#define IN4 14

#include <Stepper.h>
const int STEPS_PER_REV = 2048;

Stepper stepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);

int servoMin = 60;    
int servoMax = 160;

int currentSteps = 0;

int degreeToSteps(int deg) {
  return (deg * STEPS_PER_REV) / 360;
}

void moveToPhysicalAngle(int deg) {
  int targetSteps = degreeToSteps(deg);
  int delta = targetSteps - currentSteps;
  stepper.step(delta);
  currentSteps = targetSteps;
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIG, LOW);

  stepper.setSpeed(10);

  lcd.init();
  lcd.flipScreenVertically();
  lcd.clear();
  lcd.setColor(WHITE);
  lcd.drawString(0,0,"Radar Init...");
  lcd.display();

  // --- FIXED: Always start at physical -90° ---
  moveToPhysicalAngle(-90);

  delay(1000);
}

float readDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  unsigned long timeout = micros() + 26233L;
  while ((digitalRead(ECHO) == LOW) && (micros() < timeout));
  unsigned long start_time = micros();
  timeout = start_time + 26233L;
  while ((digitalRead(ECHO) == HIGH) && (micros() < timeout));
  unsigned long lapse = micros() - start_time;

  return lapse * 0.01716f;
}

int moveToAngle(int deg) {
  // deg = logical 0–180
  // logical 0 = physical -90
  int physicalAngle = deg - 90;
  moveToPhysicalAngle(physicalAngle);
  return physicalAngle;
}

int scan() {
  float bestDistance = 0;
  int bestAngle = servoMin;

  for (int sweep = 0; sweep < 2; sweep++) {

    // FIXED: Reset before sweep 2 → go back to physical -90°
    if (sweep == 1) {
      moveToAngle(servoMin);
      delay(300);
    }

    for (int degree = servoMin; degree <= servoMax; degree++) {
      moveToAngle(degree);

      float d = readDistance();

      if (d > bestDistance) {
        bestDistance = d;
        bestAngle = degree;
      }

      lcd.clear();
      lcd.drawString(0,0,"Sweep: " + String(sweep+1));
      lcd.drawString(0,15,"Angle: " + String(degree));
      lcd.drawString(0,30,"Dist: " + String(d) + " cm");
      lcd.display();

      delay(15);
    }
  }

  lcd.clear();
  lcd.drawString(0,0,"BEST RESULT");
  lcd.drawString(0,15,"Angle: " + String(bestAngle));
  lcd.drawString(0,30,"Dist: " + String(bestDistance) + " cm");
  lcd.display();

  return bestAngle;
}

void loop() {
  scan();
  delay(4000);
}
