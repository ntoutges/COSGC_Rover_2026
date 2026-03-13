#include <ICM_20948.h>
#include <Adafruit_GFX.h>
#include <HCSR04.h>
#include <SparkFun_TB6612.h>
#include <math.h>

// Thresholds
#define STOP_DIST    122 // 4 ft in cm
#define TURN_DIST    91  // 3 ft in cm
#define CLEAR_DIST   152 // 5 ft in cm
#define STUCK_THRESH 0.05 // placeholder IMU velocity threshold

#define TURN_DURATION 1000 // ms, timed turn duration
#define BACKWARD_DURATION 500 // ms, reverse duration when boxed in

// FSM States
typedef enum
{
  STATE_FORWARD,
  STATE_BACKWARD,
  STATE_LEFT,
  STATE_RIGHT,
  STATE_STOP,
  STATE_ERROR,
  STATE_CHECK
} rover_state;

rover_state current_state = STATE_STOP;
rover_state last_state = STATE_STOP;

// Timing Variables
unsigned long state_start_time = 0;

// Sensor Values 
long dist_left = 0, dist_front = 0, dist_right = 0;
float velocity_est = 0;

// Motor Init (pins blank) 
#define AIN1 
#define AIN2 
#define PWMA 
#define BIN1 
#define BIN2 
#define PWMB 
#define STBY 

const int offsetA = 1;
const int offsetB = 1;

Motor left_motor = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor right_motor = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

// Ultrasonic Init (pins blank)
#define TRIGGER_L 
#define ECHO_L 
#define TRIGGER_M 
#define ECHO_M 
#define TRIGGER_R 
#define ECHO_R 

Ultrasonic ultrasonic_left(TRIGGER_L, ECHO_L);
Ultrasonic ultrasonic_middle(TRIGGER_M, ECHO_M);
Ultrasonic ultrasonic_right(TRIGGER_R, ECHO_R);

// IMU Init
ICM_20948_I2C my_icm;

// Function Prototypes 
void read_sensor();
void update_state();
void execute_state();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  my_icm.begin(Wire, 1);
}

void loop() {
  // Detect state entry
  if (current_state != last_state) {
    state_start_time = millis();
    last_state = current_state;
  }

  read_sensor();
  update_state();
  execute_state();
}

// Read Sensors 
void read_sensor() {
  dist_left = ultrasonic_left.read();
  dist_front = ultrasonic_middle.read();
  dist_right = ultrasonic_right.read();

  if (my_icm.dataReady()) 
  {
    my_icm.getAGMT();
    velocity_est = my_icm.gyrZ(); // placeholder for forward velocity
  }
}

// Update FSM State 
void update_state() {

  // Prevent interrupting timed turn or backward
  if (current_state == STATE_LEFT || current_state == STATE_RIGHT || current_state == STATE_BACKWARD) 
  {
    return;
  }

  // Stuck Detection
  if (current_state == STATE_FORWARD && fabs(velocity_est) < STUCK_THRESH)
  {
    current_state = STATE_ERROR;
    return;
  }

  // Boxed In -> Backward
  if (dist_left < TURN_DIST && dist_front < TURN_DIST && dist_right < TURN_DIST)
  {
    current_state = STATE_BACKWARD;
    return;
  }

  // Emergency Front Stop -> Turn
  if (dist_front < TURN_DIST)
  {
    if (dist_left > dist_right)
      current_state = STATE_LEFT;
    else
      current_state = STATE_RIGHT;
    
    return;
  }

    //  Path Clear -> Forward
    if (dist_left > CLEAR_DIST && dist_front > CLEAR_DIST && dist_right > CLEAR_DIST) 
    {
      current_state = STATE_FORWARD;
      return;
    }

    // Default -> Stop
    current_state = STATE_STOP;
}

// Execute Current State 
void execute_state() {
  unsigned long elapsed = millis() - state_start_time;

  switch(current_state) 
  {
    case STATE_FORWARD:
      left_motor.drive(150);
      right_motor.drive(150);
    break;

    case STATE_BACKWARD:
      if (elapsed < BACKWARD_DURATION) 
      {  
        left_motor.drive(-120);
        right_motor.drive(-120);
      } 
      else 
      {
        current_state = STATE_CHECK; // after backing up, recheck
      }
    break;

      case STATE_LEFT:
        if (elapsed < TURN_DURATION) 
        {
          left_motor.drive(-120);
          right_motor.drive(120);
        }
        else 
        {
          current_state = STATE_CHECK; // after turn, recheck
        }
      break;

      case STATE_RIGHT:
        if (elapsed < TURN_DURATION) {
          left_motor.drive(120);
          right_motor.drive(-120);
        } 
        else 
        {
          current_state = STATE_CHECK; // after turn, recheck
        }
        break;

      case STATE_STOP:
      case STATE_ERROR:
        left_motor.brake();
        right_motor.brake();
      break;

      default:
      break;
    }
}