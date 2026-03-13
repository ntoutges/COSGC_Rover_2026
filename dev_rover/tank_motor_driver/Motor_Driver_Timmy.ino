#include <HCSR04.h>
#include <SparkFun_TB6612.h>

// SoftwareSerial library to create a virtual serial port on digital pins

#define AIN1 9
#define AIN2 8
#define PWMA 5
#define BIN1 4
#define BIN2 2
#define PWMB 3
#define STBY 6

const int off_set_A = 1;

Motor motor1 = Motor(AIN1, AIN2, PWMA, off_set_A, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, off_set_A, STBY);

byte triggerPin = 11;
byte echoPin = 12;

// LED Pins
const int red    = 16;
const int yellow = 14;
const int green  = 15;

// == Function Declarations ==
void set_leds(bool r, bool y, bool g);
double getDistance();
void turnRight(int duration);
void turnLeft(int duration);
void stopMotors();
void avoidObstacle();

// == Setup ==
void setup() {
  Serial.begin(9600);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);

  pinMode(red,    OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green,  OUTPUT);

  HCSR04.begin(triggerPin, echoPin);
}

// == Main Loop ==
void loop() {
  double distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance <= 0) {
    stopMotors();
    set_leds(true, false, false);
    delay(100);

  } else if (distance < 30) {
    avoidObstacle();

  } else if (distance >= 30 && distance < 50) {
    set_leds(false, true, false);  // YELLOW = caution
    motor1.drive(150);
    motor2.drive(150);

  } else {
    set_leds(false, false, true);  // GREEN = go
    motor1.drive(255);
    motor2.drive(255);
  }

  delay(100);
}

// == Function Definitions ==
void set_leds(bool r, bool y, bool g) {
  digitalWrite(red,    r ? HIGH : LOW);
  digitalWrite(yellow, y ? HIGH : LOW);
  digitalWrite(green,  g ? HIGH : LOW);
}

double getDistance() {
  double* distances = HCSR04.measureDistanceCm();
  return distances[0];
}

void turnRight(int duration) {
  motor1.drive(-200);
  motor2.drive(200);
  delay(duration);
}

void turnLeft(int duration) {
  motor1.drive(200);
  motor2.drive(-200);
  delay(duration);
}

void stopMotors() {
  motor1.brake();
  motor2.brake();
}

void avoidObstacle() {
  set_leds(true, false, false);
  stopMotors();
  vTaskDelay(pdMS_TO_TICK(500));

  // Peek RIGHT
  turnRight(400);
  stopMotors();
  vTaskDelay(pdMS_TO_TICK(500));
  double rightDist = getDistance();
  Serial.print("Right distance: "); Serial.println(rightDist);

  // Come back to center
  turnLeft(400);
  stopMotors();
  vTaskDelay(pdMS_TO_TICK(500));

  // Peek LEFT
  turnLeft(400);
  stopMotors();
  vTaskDelay(pdMS_TO_TICK(500));
  double leftDist = getDistance();
  Serial.print("Left distance: "); Serial.println(leftDist);

  // Come back to center
  turnRight(400);
  stopMotors();
  vTaskDelay(pdMS_TO_TICK(150));

  // Decide which way to go
  if (rightDist > 30 && rightDist > leftDist) {
    Serial.println("Turning RIGHT - clear");
    set_leds(false, true, false);
    turnRight(600);
  } else if (leftDist > 30) {
    Serial.println("Turning LEFT - clear");
    set_leds(false, true, false);
    turnLeft(600);
  } else {
    // Both sides blocked — reverse and try again
    Serial.println("Both blocked - reversing");
    set_leds(true, true, false);
    motor1.drive(-180);
    motor2.drive(-180);
    delay(500);
    stopMotors();
    delay(200);
    avoidObstacle();
  }
}
