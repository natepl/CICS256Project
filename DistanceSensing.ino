#include <ESP32Servo.h> 
#include <math.h>
#define SERVO1 23 
#define TRIG 5
#define ECHO 16
#define CX 60        
#define CY 25
#define MOUTH_RAD 32
#define MOUTH_Y_BASE 48   
#define COLOR 1
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


// Adjust for your SG90 actual range
int servoMin = 60;    // mechanical min angle
int servoMax = 160;   // mechanical max angle
int currentSteps = 0;

int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;

int motor2Pin1 = 25; 
int motor2Pin2 = 33; 
int enable2Pin = 32;

const int freq = 30000;
const int pwmChannel1 = 0;
const int resolution = 10;
int dutyCycle1 = 800;

const int pwmChannel2 = 1;  
int dutyCycle2 = 800;     

// Stepper functions
int degreeToSteps(int deg) {
  return (deg * STEPS_PER_REV) / 360;
}

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
  
  // Stepper set up
  stepper.setSpeed(10);
  moveToPhysicalAngle(-90);

  // Motor related set up
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  ledcSetup(pwmChannel1, freq, resolution); // Configure Channel 1
  ledcAttachPin(enable1Pin, pwmChannel1);   // Attach Pin 1 to Channel 1
  ledcWrite(pwmChannel1, dutyCycle1);      // Set speed for Motor 1

  ledcSetup(pwmChannel2, freq, resolution); // Configure Channel 2
  ledcAttachPin(enable2Pin, pwmChannel2);   // Attach Pin 2 to Channel 2
  ledcWrite(pwmChannel2, dutyCycle2);

  lcd.init(); 
  lcd.flipScreenVertically();
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

  return lapse * 0.01716f; // cm
}

void faceResponse(float distance) {
  lcd.clear();
  if(distance > 12){
    int prev_x = -MOUTH_RAD;
    int prev_y = 0;
    for (int x = -MOUTH_RAD; x <= MOUTH_RAD; x += 2) {
      int y_offset = (int)sqrt(pow(MOUTH_RAD, 2) - pow(x, 2));
      int current_screen_x = CX + x;
      int current_screen_y = (CY + 2) + y_offset * 0.7;
      
      if (x == -MOUTH_RAD) {
          prev_x = current_screen_x;
          prev_y = current_screen_y;
      }
      lcd.drawLine(prev_x, prev_y, current_screen_x, current_screen_y);
      prev_x = current_screen_x;
      prev_y = current_screen_y;
    }
  } else if(distance > 8){
    lcd.drawLine(CX-30,CY+10,CX+30,CY+10);
  } else {
    int prev_x = -MOUTH_RAD;
    int prev_y = MOUTH_Y_BASE; 
    for (int x = -MOUTH_RAD; x <= MOUTH_RAD; x += 2) {
      int arch_height = (int)sqrt(pow(MOUTH_RAD, 2) - pow(x, 2));
      arch_height = arch_height * 0.6; 
      int current_screen_x = CX + x;
      int current_screen_y = MOUTH_Y_BASE - arch_height;
      if (x == -MOUTH_RAD) {
          prev_x = current_screen_x;
          prev_y = current_screen_y;
      }
      lcd.drawLine(prev_x, prev_y, current_screen_x, current_screen_y);
      prev_x = current_screen_x;
      prev_y = current_screen_y;
    }
  
  }
  lcd.display();
}

void turn(int bestAngle) {
  int centerAngle = (servoMin + servoMax) / 2; 
  int error = bestAngle - centerAngle;

  // Control constants
  int baseSpeed = 180;  // Base reverse speed (0-255)
  int Kp = 4;           // Proportional Gain: Higher = sharper turns

  int turnAdjustment = error * Kp;
  
  int leftSpeed = baseSpeed + turnAdjustment;
  int rightSpeed = baseSpeed - turnAdjustment;

  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);

  ledcWrite(pwmChannel1, leftSpeed);
  ledcWrite(pwmChannel2, rightSpeed);

  delay(800);

  ledcWrite(pwmChannel1, 0);
  ledcWrite(pwmChannel2, 0);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}

void driveForward(int time) {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  delay(time);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}

void driveReverse(int time){
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  delay(time);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}

int moveToAngle(int deg) {
  // deg = logical 0–180
  // logical 0 = physical -90
  int physicalAngle = deg - 90;
  moveToPhysicalAngle(physicalAngle);
  return physicalAngle;
}


int scan(){
  float bestDistance = 0;
  int bestAngle = servoMin;

  for (int sweep = 0; sweep < 2; sweep++) {
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

      faceResponse(d);

      delay(15);
    }
  }
  return bestAngle;
}

void move() {
  for(int i = 0; i < 4; i++){
    float d = readDistance();
    faceResponse(d);
    if(d < 8){
      driveReverse(1000);
      break;
    } else if (d < 14) {
      driveForward(1000);
      break;
    } else {
      driveForward(1000);
    }
    delay(15);
  }
}

void loop() { 
  int bestAngle = scan();
  turn(bestAngle);
  delay(1000);
  move();
  delay(1000);
  
}

