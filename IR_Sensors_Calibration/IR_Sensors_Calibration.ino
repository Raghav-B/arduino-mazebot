#include <MeMCore.h>

#define LEFT_IR A0
#define RIGHT_IR A1

MeRGBLed led(0);

void setup() {
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  Serial.begin(9600);
  led.setpin(13);
  led.setColor(0, 0, 0);
  led.show();
}

void loop() {
  Serial.println(centering());
}

int centering() {
  int left_dist = analogRead(LEFT_IR);
  int right_dist = analogRead(RIGHT_IR);
  int difference = right_dist - left_dist;
  
  if (difference >= 100) {
    //Serial.println("TURN LEFT!");
  } else if (difference <= -100) {
    //Serial.println("TURN RIGHT!");
  } else {
    //Serial.println("STRAIGHT");
    difference = 0;
  }
  delay(100);

  Serial.print("LEFT: ");
  Serial.print(left_dist);
  Serial.print(", RIGHT: ");
  Serial.print(right_dist);
  Serial.print(", DIFF: ");
  return difference;
  //Negative difference means the mbot is skewing to the left, positive means skewing right.
}
