// September 2026
// Sending signals to the Atari 2600 as a joystick

const byte UP_PIN    = 2;
const byte DOWN_PIN  = 3;
const byte LEFT_PIN  = 4;
const byte RIGHT_PIN = 5;
const byte FIRE_PIN  = 6;

void setup() {
  // HIGH-Z means MOSFET off; Atari signal is released.
  pinMode(UP_PIN, INPUT);
  pinMode(DOWN_PIN, INPUT);
  pinMode(LEFT_PIN, INPUT);
  pinMode(RIGHT_PIN, INPUT);
  pinMode(FIRE_PIN, INPUT);
}

void setAtariSignal(byte pin, bool active) {
  if (active) {
    // Turn on the MOSFET and pull the Atari line to ground.
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  } else {
    // Release the Atari line; do not drive it HIGH.
    pinMode(pin, INPUT);
  }
}

void loop() {
  // Singaling with a joystick object.
  bool up    = false;
  bool down  = false;
  bool left  = false;
  bool right = false;
  bool fire  = false;
  // When using a physical joystick.
  //int x = analogRead(A0);
  //int y = analogRead(A1);

  //bool left  = x < 350;
  //bool right = x > 670;
  //bool up    = y < 350;
  //bool down  = y > 670;

  setAtariSignal(UP_PIN, up);
  setAtariSignal(DOWN_PIN, down);
  setAtariSignal(LEFT_PIN, left);
  setAtariSignal(RIGHT_PIN, right);
  setAtariSignal(FIRE_PIN, fire);

  delay(2);
}

