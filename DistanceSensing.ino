#include <ESP32Servo.h> 
#define SERVO1 23 
#define TRIG 5
#define ECHO 16
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

      lcd.clear();
      lcd.drawString(0,0,"Sweep: " + String(sweep+1));
      lcd.drawString(0,15,"Angle: " + String(degree)); 
      lcd.drawString(0,30,"Dist: " + String(d) + " cm");
      lcd.display();

      delay(15);
    }
  }

  // Display best result + decision
  lcd.clear();
  lcd.drawString(0,0,"BEST RESULT");
  lcd.drawString(0,15,"Angle: " + String(bestAngle));
  lcd.drawString(0,30,"Dist: " + String(bestDistance) + " cm");

  lcd.display();
  return bestAngle;
}

void loop() { 
  scan();
  delay(4000);  // show for 4s before next sweep
}
