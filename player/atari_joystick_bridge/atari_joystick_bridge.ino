/*
 * Atari 2600 joystick relay bridge for a classic 5 V Arduino Nano.
 *
 * Linux host -> USB serial -> Nano -> ULN2803A -> five normally-open relays.
 * The relays' dry contacts, not Nano GPIO, connect to the Atari controller
 * port.  All outputs start neutral and return to neutral when a command's
 * requested hold time expires or when malformed input is received.
 *
 * Line protocol (newline terminated, 115200 baud):
 *   SET <mask> <duration_ms>  Apply a joystick mask for 1..250 ms.
 *   STOP                      Open every relay immediately.
 *   PING                      Reply without changing relay state.
 *
 * Mask bits: 1=up, 2=down, 4=left, 8=right, 16=fire.  For example,
 * "SET 17 100" holds up+fire for 100 ms.  Opposing direction pairs and
 * masks with other bits are rejected.  The host must renew a held action
 * before it expires; failure to do so is deliberately neutral.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const uint8_t PIN_UP = 2;
const uint8_t PIN_DOWN = 3;
const uint8_t PIN_LEFT = 4;
const uint8_t PIN_RIGHT = 5;
const uint8_t PIN_FIRE = 6;

const uint8_t ACTION_UP = 1 << 0;
const uint8_t ACTION_DOWN = 1 << 1;
const uint8_t ACTION_LEFT = 1 << 2;
const uint8_t ACTION_RIGHT = 1 << 3;
const uint8_t ACTION_FIRE = 1 << 4;
const uint8_t ACTION_MASK = ACTION_UP | ACTION_DOWN | ACTION_LEFT |
                            ACTION_RIGHT | ACTION_FIRE;

// SunFounder TS0010D relay modules use a low-level trigger: LOW closes the
// module's NO-to-COM contact, while HIGH leaves it open.
const uint8_t RELAY_ACTIVE_LEVEL = LOW;
const uint8_t RELAY_INACTIVE_LEVEL = HIGH;

const unsigned long MAX_HOLD_MS = 250;
const size_t INPUT_CAPACITY = 48;

char input[INPUT_CAPACITY];
size_t inputLength = 0;
uint8_t appliedMask = 0;
unsigned long releaseAt = 0;
bool actionActive = false;

void setRelays(uint8_t mask) {
  digitalWrite(PIN_UP, (mask & ACTION_UP) ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
  digitalWrite(PIN_DOWN, (mask & ACTION_DOWN) ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
  digitalWrite(PIN_LEFT, (mask & ACTION_LEFT) ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
  digitalWrite(PIN_RIGHT, (mask & ACTION_RIGHT) ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
  digitalWrite(PIN_FIRE, (mask & ACTION_FIRE) ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL);
  appliedMask = mask;
}

void neutralize() {
  setRelays(0);
  actionActive = false;
}

bool parseUnsigned(const char *text, unsigned long *value) {
  if (text == NULL || *text == '\0' || *text == '-') {
    return false;
  }

  char *end = NULL;
  unsigned long parsed = strtoul(text, &end, 10);
  if (*end != '\0') {
    return false;
  }
  *value = parsed;
  return true;
}

bool legalMask(unsigned long mask) {
  if (mask > ACTION_MASK) {
    return false;
  }
  if ((mask & ACTION_UP) && (mask & ACTION_DOWN)) {
    return false;
  }
  if ((mask & ACTION_LEFT) && (mask & ACTION_RIGHT)) {
    return false;
  }
  return true;
}

void replyError(const __FlashStringHelper *reason) {
  neutralize();
  Serial.print(F("ERR "));
  Serial.println(reason);
}

void handleLine(char *line) {
  char *save = NULL;
  char *command = strtok_r(line, " \t", &save);
  if (command == NULL) {
    replyError(F("FORMAT"));
    return;
  }

  if (strcmp(command, "STOP") == 0 && strtok_r(NULL, " \t", &save) == NULL) {
    neutralize();
    Serial.println(F("OK 0"));
    return;
  }

  if (strcmp(command, "PING") == 0 && strtok_r(NULL, " \t", &save) == NULL) {
    Serial.print(F("PONG "));
    Serial.println(appliedMask);
    return;
  }

  if (strcmp(command, "SET") != 0) {
    replyError(F("COMMAND"));
    return;
  }

  char *maskText = strtok_r(NULL, " \t", &save);
  char *durationText = strtok_r(NULL, " \t", &save);
  if (maskText == NULL || durationText == NULL || strtok_r(NULL, " \t", &save) != NULL) {
    replyError(F("FORMAT"));
    return;
  }

  unsigned long requestedMask = 0;
  unsigned long durationMs = 0;
  if (!parseUnsigned(maskText, &requestedMask) || !parseUnsigned(durationText, &durationMs)) {
    replyError(F("NUMBER"));
    return;
  }
  if (!legalMask(requestedMask)) {
    replyError(F("MASK"));
    return;
  }
  if (durationMs == 0 || durationMs > MAX_HOLD_MS) {
    replyError(F("DURATION"));
    return;
  }

  setRelays((uint8_t)requestedMask);
  releaseAt = millis() + durationMs;
  actionActive = requestedMask != 0;
  Serial.print(F("OK "));
  Serial.print(appliedMask);
  Serial.print(' ');
  Serial.println(durationMs);
}

void readSerial() {
  while (Serial.available() > 0) {
    char received = (char)Serial.read();
    if (received == '\r') {
      continue;
    }
    if (received == '\n') {
      input[inputLength] = '\0';
      handleLine(input);
      inputLength = 0;
      continue;
    }
    if (!isPrintable((unsigned char)received) || inputLength >= INPUT_CAPACITY - 1) {
      inputLength = 0;
      replyError(F("INPUT"));
      continue;
    }
    input[inputLength++] = received;
  }
}

void setup() {
  // Latch the inactive HIGH level before enabling each output.  This avoids a
  // low-going startup pulse on the TS0010D's active-low inputs.
  digitalWrite(PIN_UP, RELAY_INACTIVE_LEVEL);
  digitalWrite(PIN_DOWN, RELAY_INACTIVE_LEVEL);
  digitalWrite(PIN_LEFT, RELAY_INACTIVE_LEVEL);
  digitalWrite(PIN_RIGHT, RELAY_INACTIVE_LEVEL);
  digitalWrite(PIN_FIRE, RELAY_INACTIVE_LEVEL);
  pinMode(PIN_UP, OUTPUT);
  pinMode(PIN_DOWN, OUTPUT);
  pinMode(PIN_LEFT, OUTPUT);
  pinMode(PIN_RIGHT, OUTPUT);
  pinMode(PIN_FIRE, OUTPUT);
  neutralize();

  Serial.begin(115200);
  Serial.println(F("ATARI_BRIDGE READY"));
}

void loop() {
  readSerial();
  // Signed subtraction remains correct when millis() wraps around.
  if (actionActive && (long)(millis() - releaseAt) >= 0) {
    neutralize();
    Serial.println(F("TIMEOUT"));
  }
}
