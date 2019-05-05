#include "MeMCore.h"
#include <stdbool.h>

// Initialising onboard button
#define BUTTON 7

// Initialising left and right motors
MeDCMotor l_motor(M1);
MeDCMotor r_motor(M2);
// Motor Magic Numbers
#define FWDSPD 255 // Speed during forward movement
#define TURNSPD 180 // Speed during motor turning
// Turn delay is the time (ms) to turn 90 degrees at TURNSPD
#define LTURNDELAY 340
#define RTURNDELAY 330

// Initialising ultrasonic sensor
MeUltrasonicSensor ultrasonic(PORT_2);
// Distance (cm) from wall to stop at
#define BUFFER_DIST 11.0

// Initialising line sensor
MeLineFollower line_sensor(PORT_1);

// Initialising buzzer
MeBuzzer buzzer;

// Initialising analog input for IR sensors
#define LEFT_IR A0
#define RIGHT_IR A1
// Threshold voltage of IR receiver for wall avoidance.
#define IR_BUFFER_V 400

// Initialising LED and LDR
MeRGBLed led(0, 2);
MeLightSensor ldr(PORT_8);
// Array to store output voltages from LDR for colour being detected.
float colour_arr[] = {0, 0, 0};
// Arrays store hardcoded voltages from LDR for black and grey colours.
float black_arr[] = {385.00, 339.80, 392.20};
float grey_arr[] = {543.60, 561.80, 547.60};
// Delays for LED and LDR
#define RGB_WAIT 50
#define LDR_WAIT 10

// Initialising analog inputs for sound detector circuit
#define LOW_HZ A2 // Low pass input
#define HIGH_HZ A3 // High pass input

void setup() {
  // Setting up pins
  pinMode(BUTTON, INPUT);
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  led.setpin(13);
  led.setColor(0, 0, 0);
  led.show();
  pinMode(LOW_HZ, INPUT);
  pinMode(HIGH_HZ, INPUT);
  Serial.begin(9600);

  // Infinite loop to prevent further code execution until button 
  // is pressed.
  while (analogRead(BUTTON) > 100) {
    delay(50);
  }
  // Startup chime
  buzzer.tone(200, 200);
  buzzer.tone(400, 200);
  buzzer.tone(600, 200);
  buzzer.tone(800, 200);
  buzzer.tone(1000, 200);
  buzzer.tone(1200, 200);
  buzzer.tone(1400, 200);
  buzzer.tone(1600, 200);
  delay(5000);  
}

void loop() {
  move_forward();
}

/** 
 * This function returns a bool based on whether a black line is detected.
 *  
 * @return Returns true if a black line is detected by both line sensors, 
 *         else returns false.
 */
bool is_black() {
  int line_sensor_state = line_sensor.readSensors();
  switch (line_sensor_state) {
    case 3:
      return false;
    default:
      return true;
  }
}

/**
 * Function turns the mBot clockwise or anti-clockwise for a certain angle.
 * 
 * @param[in] dir Either 0 or 1 for turning anti-clockwise and clockwise 
 *                respectively.
 * @param[in] angle Angle to turn. Can range from 0 to 360 degrees.
 */
void perpen_turn(int dir, float angle){
  int mot_speed = TURNSPD;
  
  // Conditional makes motspeed negative if direction to turn is
  // anti-clockwise.
  if (dir == 1){
    mot_speed *= -1;
  }
  
  l_motor.run(mot_speed);
  r_motor.run(mot_speed);
  // Delay below controls how long the mBot turns in either direction.
  // The TURNDELAY values can be adjusted to calibrate the amount the 
  // mBot turns.
  delay(dir == 1 ? RTURNDELAY * (angle / 90) : LTURNDELAY * (angle / 90));
  l_motor.stop();
  r_motor.stop();
}

/**
 * This function carries out different turns based on the detected colour.
 * 
 * @param[in] colour This is a character representing the colour detected.
 */
void do_movement(char colour) {
  switch(colour){
      // Green detected: right turn
      case 'G': 
          perpen_turn(1, 90);
          break;
      
      // Red detected: left turn
      case 'R':
          perpen_turn(0, 90);
          break;
          
      // White detected: u-turn in anti-clockwise direction)
      case 'L': 
          perpen_turn(0, 170);
          break;
          
      // White detected: u-turn in clockwise direction
      case 'F':
          //Serial.print("white right detected");
          perpen_turn(1, 170);
          break;
     
     // Orange derected: two successive left turns
     case 'O':
        // Turn 90 degrees anti-clockwise initially
        perpen_turn(0, 90);
        delay(100);

        // Moving forward until BUFFER_DIST
        while (ultrasonic.distanceCm() > BUFFER_DIST) {
          // Calling move_forward() allows IR sensors to be used
          // during forward movement
          move_forward();
          delay(100);
        }
        l_motor.stop();
        r_motor.stop();
        delay(300);
        
        // Turn 90 degrees anti-clockwise again
        perpen_turn(0, 90);
      break;
        
      // Blue detected: two successive right turns
      case 'B':
        // Turn 90 degrees clockwise initially
        perpen_turn(1, 90);

        // Moving forward until BUFFER_DIST
        delay(100);
        while (ultrasonic.distanceCm() > BUFFER_DIST){
          // Calling move_forward() allows IR sensors to be used
          // during forward movement
          move_forward();
          delay(100);
        }
        l_motor.stop();
        r_motor.stop();
        delay(300);

        // Turn 90 degrees clockwise again
        perpen_turn(1, 90);      
      break;
      
      // Black detected: check for sound and carry out corresponding
      // movement
      case 'X':
        do_movement(sound_detection());
      break;
  }  
}

/**
 * This is the primary function which moves the mBot forward. It 
 * employs the IR sensors to prevent wall collisions.
 */
void move_forward() {
  // Conditional executes the colour_detection() function when a
  // black line is detected.
  if (is_black()) {
    l_motor.stop();
    r_motor.stop();
    delay(100);
    colour_detection();
  }
  
  int left_dist = analogRead(LEFT_IR);
  int right_dist = analogRead(RIGHT_IR);

  // Conditionals below cause the mBot to turn away from the wall
  // when the IR receiver output on either side becomes less than
  // or equal to IR_BUFFER_V. 
  if (left_dist <= IR_BUFFER_V) {
    r_motor.stop();
    l_motor.run(-FWDSPD);
  } else if (right_dist <= IR_BUFFER_V) {
    l_motor.stop();
    r_motor.run(FWDSPD);
  } else {
    // Executes when mBot is centered between the walls.
    l_motor.run(-FWDSPD);
    r_motor.run(FWDSPD);
  }
}

/**
 * This function detects the colour when the mBot reaches a  
 * challenge location. 
 *  
 * @pre Function is called when the mBot reaches a black line.
 * @post The values inside colour_arr[] are changed based on the
 *       detected colour.
 */
void colour_detection() {
  // Loop below runs thrice to get the R, G, and B values of 
  // the colour to be detected.
  for (int i = 0; i < 3; i += 1) {
    if (i == 0) {
      // Set Red
      led.setColor(255, 0, 0);
    } else if (i == 1) {
      // Set Green
      led.setColor(0, 255, 0);
    } else {
      // Set Blue
      led.setColor(0, 0, 255);
    }
    led.show();
     
    delay(RGB_WAIT);
    // Reading 5 voltage values from the LDR and finding
    // their average.
    colour_arr[i] = avg_reading(5);
    // Converting voltage reading to a corresponding RGB
    // value between 0 and 255.
    colour_arr[i] = ((colour_arr[i] - black_arr[i]) /
        grey_arr[i]) * 255;
    led.setColor(0, 0, 0);
    led.show();
    delay(RGB_WAIT);
    // In the event that colour_arr[i] is somehow negative,
    // conditional below sets it to 0 as this is the lowest
    // RGB value possible.
    if (int(colour_arr[i] < 0)) {
      colour_arr[i] = 0;
    }
  }
  
  // Calling do_movement to carry out a movement based on
  // the current RGB values stored in colour_arr.
  do_movement(output_colour(colour_arr));
}

/**
 * Function returns a character representing the colour
 * detected or the movement to carry out. The function 
 * contains conditionals that act as thresholds to assign
 * the detected RGB values an approximate colour.
 * 
 * @pre The RGB values for the detected colour have been stored
 *      inside colour_arr[]. 
 * @param[in] arr This is the array containing the RGB values
 *                of the colour that was detected.
 * @return Returns a character to represent the movement to
 *         carry out.
 */
char output_colour(float *arr) {
  // White is detected
  if (arr[0] >= 220 && arr[1] >= 220 && arr[2] >= 220) {
    // The IR sensors are checked to ensure the mBot turns in
    // the direction in which there is more space from the wall.
    int left_dist = analogRead(LEFT_IR);
    int right_dist = analogRead(RIGHT_IR);
    if (left_dist < right_dist){
      return 'F'; // 'F' represents clockwise u-turn
    } else {
      return 'L'; // 'L' represents anti-clockwise u-turn
    }
  } 

  // Orange is detected
  else if (arr[0] >= 230 && arr[1] >= 60 && arr[2] >= 30) {
    return 'O';
  }

  // Red is detected
  else if (arr[0] >= 190 && arr[1] >= 20 && arr[2] >= 20) {
    return 'R';
  } 

  // Blue is detected
  else if (arr[2] >= 160 && arr[1] >= 110 && arr[0] >= 40) {
    return 'B';
  } 

  // Green is detected
  else if (arr[2] >= 50 && arr[1] >= 90 && arr[0] >= 50) {
    return 'G';
  } 

  // Black is detected
  else {
    return 'X';
  }
}

/**
 * Function to find average of a number of voltage readings
 * from the LDR.
 * 
 * @param[in] times The number of readings to average.
 * @return Returns average of 'times' number of readings.
 */
float avg_reading(int times) {
  float total = 0;
  for (int i = 0; i < times; i += 1) {
    total += ldr.read();
    delay(LDR_WAIT);
  }
  return total / times;
}

/**
 * Sound detection function that outputs a character to
 * represent the movement to carry out. The function plays
 * the victory tone if the sound detected is close to 0.
 * It uses the differences between the two frequencies detected
 * to determine the movement the movement to carry out.
 * 
 * @pre mBot has detected the colour black at a challenge.
 * @return Returns a character to carry out a certain movement
 *         based on the sound detected.
 */
char sound_detection() {
  int low_sum = 0;
  int high_sum = 0;

  // Block of code gets the average of 50 voltages for the low 
  // and average frequencies detected by the microphone.
  for (int i = 0; i < 50; i += 1) {
    low_sum += analogRead(LOW_HZ);
    high_sum += analogRead(HIGH_HZ);
    delay(5);
  }
  low_sum /= 50;
  high_sum /= 50;
  int difference = low_sum - high_sum;
  
  // If voltages are close to 0, victory_tone() is called.
  if (low_sum >= -10 && low_sum <= 10 && high_sum 
      >= -10 && high_sum <= 10) {
    victory_tone();
  } 

  // Right turn
  else if (difference <= -100) {
    return 'G';
  } 

  // Left turn
  else if (difference >= 150) {
    return 'R';
  } 

  // U-turn for specific band of difference
  else if (difference > -100 && difference < 150) {
    // The IR sensors are checked to ensure the mBot turns in
    // the direction in which there is more space from the wall.
    int left_dist = analogRead(LEFT_IR);
    int right_dist = analogRead(RIGHT_IR);
    if (left_dist < right_dist){
      return 'F'; // 'F' represents clockwise u-turn
    } else {
      return 'L'; // 'L' represents anti-clockwise u-turn
    }
  }
}

/**
 * This function plays Darude - Sandstorm through the mBot's 
 * buzzer.
 * 
 * @pre mBot has detected black and no sound has been detected.
 * @post mBot goes into an infinite empty loop (all actions 
 *       terminated).
 */
void victory_tone() {
  // Array for the frequencies of every note in the song.
  int melody[] = {330, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 330, 330, 330, 
    330, 330, 330, 330, 294, 294, 294, 294, 294, 294, 294, 220, 220, 247, 247, 247, 247, 247, 247, 
    247, 247, 247, 247, 247, 247, 330, 330, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 
    247, 330, 330, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 330, 330, 330, 330, 
    330, 330, 330, 294, 294, 294, 294, 294, 294, 294, 220, 220, 247, 247, 247, 247, 247, 247, 247, 
    247, 247, 247, 247, 247, 330, 330, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 
    330, 330, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 0};

  // Array for the duration of every note.
  int note_durations[] = {111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 111, 111, 111, 
    111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 111, 
    111, 111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 
    222, 111, 111, 111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 111, 111, 111, 111, 
    111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 111, 111, 
    111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 
    111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 222, 111, 111, 111, 111, 111, 111, 222, 222, 0};

  // Playing the song.
  for (int note = 0; note < sizeof(melody) / sizeof(int); note++) {    
    buzzer.tone(melody[note] * 12, note_durations[note] * 0.1);    
    delay(note_durations[note]);    
    buzzer.noTone();
  }

  // Infinite loop - end of program.
  while(1) {
    
  }
}
