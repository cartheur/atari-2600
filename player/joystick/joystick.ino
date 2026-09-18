const byte PIN_UP    = 2;
const byte PIN_DOWN  = 3;
const byte PIN_LEFT  = 4;
const byte PIN_RIGHT = 5;
const byte PIN_FIRE  = 6;

void setup() {
  pinMode(PIN_UP,    OUTPUT);
  pinMode(PIN_DOWN,  OUTPUT);
  pinMode(PIN_LEFT,  OUTPUT);
  pinMode(PIN_RIGHT, OUTPUT);
  pinMode(PIN_FIRE,  OUTPUT);

  digitalWrite(PIN_UP,    LOW);
  digitalWrite(PIN_DOWN,  LOW);
  digitalWrite(PIN_LEFT,  LOW);
  digitalWrite(PIN_RIGHT, LOW);
  digitalWrite(PIN_FIRE,  LOW);

  Serial.begin(115200);
}

void setControl(char control, bool pressed) {
  byte pin;

  switch (control) {
    case 'U': pin = PIN_UP;    break;
    case 'D': pin = PIN_DOWN;  break;
    case 'L': pin = PIN_LEFT;  break;
    case 'R': pin = PIN_RIGHT; break;
    case 'F': pin = PIN_FIRE;  break;
    default: return;
  }

  digitalWrite(pin, pressed ? HIGH : LOW);
}

void loop() {
  if (Serial.available() >= 2) {
    char control = Serial.read();
    char state   = Serial.read();

    if (state == '0' || state == '1') {
      setControl(control, state == '1');
      Serial.println("OK");
    }

    // Discard the command's newline and any extra whitespace
    while (Serial.available() && Serial.peek() != '\n') {
      Serial.read();
    }
    if (Serial.available() && Serial.peek() == '\n') {
      Serial.read();
    }
  }
}
