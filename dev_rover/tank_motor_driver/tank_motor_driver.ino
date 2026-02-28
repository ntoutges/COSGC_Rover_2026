/**
 * @brief Test via WASD_ system
 * @author Nicholas T.
 * @copyright 2026 PiCO
 */
// 

// Motor driver pins
#define MA1 2
#define MA2 3
#define MB1 0
#define MB2 1

void set_side_dir(uint8_t pin1, uint8_t pin2, int8_t dir);

void setup() {

  // Setup all pins
  pinMode(MA1, OUTPUT);
  pinMode(MA2, OUTPUT);
  pinMode(MB1, OUTPUT);
  pinMode(MB2, OUTPUT);

  set_side_dir(MA1, MA2, 0);
  set_side_dir(MB1, MB2, 0);

  // Conenct to serial monitor
  Serial.begin(115200);
  while (!Serial);
}

void loop() {

  // Do nothing while waiting for serial input
  if (!Serial.available()) return;

  char ctrl = Serial.read();

  // ctrl.toUpperCase()
  if (ctrl >= 'a' && ctrl <= 'z') {
    ctrl = ctrl - 'a' + 'A';
  }

  // Control direction of each side (-1, 0, 1)
  int8_t a_dir = 0;
  int8_t b_dir = 0;

  switch (ctrl) {
    case 'W':
      a_dir = 1;
      b_dir = 1;
      break;
    
    case 'A':
      a_dir = -1;
      b_dir = 1;
      break;

    case 'S':
      a_dir = -1;
      b_dir = -1;
      break;

    case 'D':
      a_dir = 1;
      b_dir = -1;
      break;

    // Redundantly reset directions
    case ' ':
    case '_':
      a_dir = 0;
      b_dir = 0;
      break;
  }

  set_side_dir(MA1, MA2, a_dir);
  set_side_dir(MB1, MB2, b_dir);

  Serial.print("Movement: (");
  Serial.print(a_dir < 0 ? 'v' : a_dir > 0 ? '^' : '|');
  Serial.print(b_dir < 0 ? 'v' : b_dir > 0 ? '^' : '|');
  Serial.println(")");
}

/**
 * Update some set of pins to match the direction
 * @param pin1  The first pin in a matched pair
 * @param pin2  The second pin in a matched pair
 * @param dir   Indicates the direction to move in. (> 0) => Forwards; (< 0) => Reverse; (== 0) => Stopped 
 */
void set_side_dir(uint8_t pin1, uint8_t pin2, int8_t dir) {
  digitalWrite(pin1, dir < 0 ? HIGH : LOW);
  digitalWrite(pin2, dir > 0 ? HIGH : LOW);
}
