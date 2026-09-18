// Atari 2600 joystick output test
// Each control is activated for one second.
//
// D2 = Fire
// D3 = Up
// D4 = Down
// D5 = Left
// D6 = Right

const byte PIN_FIRE  = 2;
const byte PIN_UP    = 3;
const byte PIN_DOWN  = 4;
const byte PIN_LEFT  = 5;
const byte PIN_RIGHT = 6;

const byte controls[] = {
  PIN_FIRE,
  PIN_UP,
  PIN_DOWN,
  PIN_LEFT,
  PIN_RIGHT
};

const byte controlCount = sizeof(controls) / sizeof(controls[0]);

void releaseAll() {
  for (byte i = 0; i < controlCount; i++) {
    digitalWrite(controls[i], LOW);
  }
}

void pressControl(byte pin) {
  releaseAll();
  digitalWrite(pin, HIGH);
}

void setup() {
  for (byte i = 0; i < controlCount; i++) {
    pinMode(controls[i], OUTPUT);
  }

  releaseAll();

  // Allow time to connect the Atari joystick cable
  delay(2000);
}

void loop() {
  // Neutral
  releaseAll();
  delay(1000);

  // Fire
  pressControl(PIN_FIRE);
  delay(1000);

  // Neutral
  releaseAll();
  delay(1000);

  // Up
  pressControl(PIN_UP);
  delay(1000);

  // Neutral
  releaseAll();
  delay(1000);

  // Down
  pressControl(PIN_DOWN);
  delay(1000);

  // Neutral
  releaseAll();
  delay(1000);

  // Left
  pressControl(PIN_LEFT);
  delay(1000);

  // Neutral
  releaseAll();
  delay(1000);

  // Right
  pressControl(PIN_RIGHT);
  delay(1000);

  // Neutral before repeating
  releaseAll();
  delay(1000);
}
