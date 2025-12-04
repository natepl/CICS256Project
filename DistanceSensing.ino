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

Servo myservo;

// Adjust for your SG90 actual range
int servoMin = 0;    // mechanical min angle
int servoMax = 40;   // mechanical max angle

void setup() { 
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIG, LOW);

  myservo.attach(SERVO1, 500, 2400);

  lcd.init(); 
  lcd.flipScreenVertically(); 
  lcd.clear();
  lcd.setColor(WHITE);
  lcd.drawString(0,0,"Radar Init...");
  lcd.display();
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

int scan(){
  float bestDistance = 0;
  int bestAngle = servoMin;

  // TWO sweeps
  for (int sweep = 0; sweep < 2; sweep++) {
    for (int degree = servoMin; degree <= servoMax; degree++) { // logical 0–180
      myservo.write(degree);

      float d = readDistance();

      if (d > bestDistance) {
        bestDistance = d;
        bestAngle = degree; // logical 0–180 for decision
      }
      faceResponse(d);
    }
  }
  return bestAngle;
}

void loop() { 
  scan();
  delay(4000);  // show for 4s before next sweep
}
