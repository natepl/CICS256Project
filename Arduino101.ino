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

void loop() { 

  float bestDistance = 0;
  int bestAngle = servoMin;

  // TWO sweeps
  for (int sweep = 0; sweep < 2; sweep++) {
    for (int degree = 0; degree <= 180; degree++) { // logical 0–180
      int mappedAngle = map(degree, 0, 180, servoMin, servoMax); // map to SG90 range
      myservo.write(mappedAngle);

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

  // Adjusted thresholds for 0–40° sweep
  String decision;
  // 0–40° sweep:
  // Left: 0–13, Forward: 14–27, Right: 28–40
  // For a full 180° sweep, thresholds could be:
  // Left: 0–60, Forward: 61–120, Right: 121–180
  if (bestAngle <= 13) {
    decision = "TURN LEFT";
  } 
  else if (bestAngle >= 28) {
    decision = "TURN RIGHT";
  } 
  else {
    decision = "GO FORWARD";
  }

  // Display best result + decision
  lcd.clear();
  lcd.drawString(0,0,"BEST RESULT");
  lcd.drawString(0,15,"Angle: " + String(bestAngle));
  lcd.drawString(0,30,"Dist: " + String(bestDistance) + " cm");
  lcd.drawString(0,45,"Decision: " + decision);
  lcd.display();

  Serial.println("----- RESULT -----");
  Serial.print("Best angle = "); Serial.println(bestAngle);
  Serial.print("Best dist  = "); Serial.println(bestDistance);
  Serial.print("Decision   = "); Serial.println(decision);
  Serial.println("-----------------");

  delay(4000);  // show for 4s before next sweep
}
