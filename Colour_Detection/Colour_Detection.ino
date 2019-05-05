#include <MeMCore.h>

#define LDRWait 10
#define RGBWait 50

MeRGBLed led(0, 2);
MeLightSensor ldr(PORT_8);

float colour_arr[] = {0, 0, 0};
float white_arr[] = {0, 0, 0};
float black_arr[] = {0, 0, 0};
float grey_diff[] = {0, 0, 0};

void setup() {
  led.setpin(13);
  led.setColor(0, 0, 0);
  led.show();
  Serial.begin(9600);
  ldr_calibration();
}

void loop() {
  colour_detection();
  char colour = output_colour(colour_arr);
}

void colour_detection() {
  int average_iter = 10;
  int sum = 0;
  for (int i = 0; i < 3; i += 1) {
    if (i == 0) {
      Serial.print("R = ");
      led.setColor(255, 0, 0);
    } else if (i == 1) {
      Serial.print("G = ");
      led.setColor(0, 255, 0);
    } else {
      Serial.print("B = ");
      led.setColor(0, 0, 255);
    }
    sum = 0;
    led.show();
    delay(RGBWait);
    colour_arr[i] = avg_reading(5);
    colour_arr[i] = (colour_arr[i] - black_arr[i]) / (grey_diff[i]) * 255;
    led.setColor(0, 0, 0);
    led.show();
    delay(RGBWait);
    if (int(colour_arr[i] < 0)) {
      colour_arr[i] = 0;
    }
    for (int j = average_iter; j > 0; j -= 1) {
      sum += colour_arr[i];
    }
    Serial.println(int(sum / average_iter));
    colour_arr[i] = sum / average_iter;
  }
}

void ldr_calibration() {
  //White Calibration
  Serial.println("Put White Sample For Calibration! :D");
  delay(5000);
  led.setColor(0, 0, 0);
  led.show();
  for (int i = 0; i < 3; i += 1) {
    if (i == 0) {
      led.setColor(255, 0, 0);
    } else if (i == 1) {
      led.setColor(0, 255, 0);
    } else {
      led.setColor(0, 0, 255);
    }
    led.show();
    delay(RGBWait);
    white_arr[i] = avg_reading(5);
    led.setColor(0, 0, 0);
    led.show();
    delay(RGBWait);
    Serial.println(white_arr[i]);
  }
  //Black Calibration
  Serial.println("Put Black Sample For Calibration! ;D");
  delay(5000);
  for (int i = 0; i < 3; i += 1) {
    if (i == 0) {
      led.setColor(255, 0, 0);
    } else if (i == 1) {
      led.setColor(0, 255, 0);
    } else {
      led.setColor(0, 0, 255);
    }
    led.show();
    delay(RGBWait);
    black_arr[i] = avg_reading(5);
    led.setColor(0, 0, 0);
    led.show();
    delay(RGBWait);
    grey_diff[i] = white_arr[i] - black_arr[i];
    Serial.print("Black: ");
    Serial.println(black_arr[i]);
    Serial.print("Grey: ");
    Serial.println(grey_diff[i]);
  }
  Serial.println("LDR IS READY!!! xD");
  delay(5000);
}

float avg_reading(int times) {
  float reading = 0;
  float total = 0;
  for (int i = 0; i < times; i += 1) {
    reading = ldr.read();
    total += reading;
    delay(LDRWait);
  }
  return total / times;
}

char output_colour(float *arr) {
  if (arr[0] >= 220 && arr[1] >= 220 && arr[2] >= 220) {
    Serial.println("White");
    return 'W';
  } else if (arr[0] >= 215 && arr[1] >= 75 && arr[2] >= 75) {
    Serial.println("Orange");
    return 'O';
  } else if (arr[0] >= 190 && arr[1] >= 60 && arr[2] >= 60) {
    Serial.println("Red");
    return 'R';
  } else if (arr[2] >= 160 && arr[1] >= 110 && arr[0] >= 40) {
    Serial.println("Blue");
    return 'B';
  } else if (arr[2] >= 50 && arr[1] >= 90 && arr[0] >= 50) {
    Serial.println("Green");
    return 'G';
  } else if (arr[0] >= 0 && arr[1] >= 0 && arr[2] >= 0) {
    Serial.println("Black");
    return 'X';
  } else {
    Serial.println("???");
    return '?';
  }
}
