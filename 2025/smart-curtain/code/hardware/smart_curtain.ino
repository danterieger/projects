// Smart curtain, hardware prototype (Arduino Uno)
// Three positions (open, half, closed) and three modes (automatic, manual, wireless).
// Transcribed from the project report's program listing.

#include <Servo.h>

#define LDR_PIN A0
#define SERVO_PIN 9
#define BTN_OPEN 2
#define BTN_CLOSE 3
#define BTN_HALF 6
#define BTN_MODE 4
#define BTN_WIRELESS 5
#define LED_STATUS 13

Servo curtainServo;

int LIGHT_HIGH = 700;
int LIGHT_MID = 500;
int LIGHT_LOW = 300;

// Servo positions
int OPEN_ANGLE = 180;
int HALF_ANGLE = 90;
int CLOSED_ANGLE = 0;

enum CurtainState {
  CLOSED = 0,
  HALF_OPEN = 1,
  FULLY_OPEN = 2
};

int controlMode = 0;
int currentPosition = 0;
int targetPosition = 0;
CurtainState curtainState = CLOSED;

unsigned long lastPrint = 0;
unsigned long lastBlink = 0;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT);
  pinMode(BTN_OPEN, INPUT_PULLUP);
  pinMode(BTN_CLOSE, INPUT_PULLUP);
  pinMode(BTN_HALF, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_WIRELESS, INPUT_PULLUP);

  curtainServo.attach(SERVO_PIN);
  curtainServo.write(CLOSED_ANGLE);
  currentPosition = CLOSED_ANGLE;
  targetPosition = CLOSED_ANGLE;

  Serial.println("========================================");
  Serial.println(" SMART CURTAIN SYSTEM INITIALIZED");
  Serial.println(" 3 POSITIONS: OPEN | HALF | CLOSED");
  Serial.println("========================================");
  Serial.println();
  printMode();
}

void loop() {
  checkModeButton();
  switch (controlMode) {
    case 0:
      automaticMode();
      break;
    case 1:
      manualMode();
      break;
    case 2:
      wirelessMode();
      break;
  }
  moveServoSmooth();
  delay(10);
}

void checkModeButton() {
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounce = 0;
  bool currentButtonState = digitalRead(BTN_MODE);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    if (millis() - lastDebounce > 300) {
      controlMode++;
      if (controlMode > 2) controlMode = 0;
      printMode();
      lastDebounce = millis();
    }
  }
  lastButtonState = currentButtonState;
}

void automaticMode() {
  int lightLevel = analogRead(LDR_PIN);
  if (millis() - lastPrint > 1000) {
    Serial.print("[AUTO] Light Level: ");
    Serial.print(lightLevel);
    Serial.print(" | Curtain: ");
    printCurtainState();
    Serial.print(" | Position: ");
    Serial.print(currentPosition);
    Serial.println("°");
    lastPrint = millis();
  }
  if (lightLevel > LIGHT_HIGH && curtainState != FULLY_OPEN) {
    Serial.println(">>> BRIGHT LIGHT DETECTED - Fully opening curtain!");
    fullyOpenCurtain();
  }
  else if (lightLevel >= LIGHT_MID && lightLevel <= LIGHT_HIGH && curtainState != HALF_OPEN) {
    Serial.println(">>> MEDIUM LIGHT DETECTED - Half opening curtain!");
    halfOpenCurtain();
  }
  else if (lightLevel < LIGHT_LOW && curtainState != CLOSED) {
    Serial.println(">>> LOW LIGHT DETECTED - Closing curtain!");
    closeCurtain();
  }
}

void manualMode() {
  static bool lastOpenState = HIGH;
  static bool lastCloseState = HIGH;
  static bool lastHalfState = HIGH;
  static unsigned long lastDebounce = 0;
  bool openButtonState = digitalRead(BTN_OPEN);
  bool closeButtonState = digitalRead(BTN_CLOSE);
  bool halfButtonState = digitalRead(BTN_HALF);

  if (openButtonState == LOW && lastOpenState == HIGH) {
    if (millis() - lastDebounce > 200) {
      Serial.println(">>> MANUAL: Open button pressed");
      fullyOpenCurtain();
      lastDebounce = millis();
    }
  }
  lastOpenState = openButtonState;

  if (halfButtonState == LOW && lastHalfState == HIGH) {
    if (millis() - lastDebounce > 200) {
      Serial.println(">>> MANUAL: Half button pressed");
      halfOpenCurtain();
      lastDebounce = millis();
    }
  }
  lastHalfState = halfButtonState;

  if (closeButtonState == LOW && lastCloseState == HIGH) {
    if (millis() - lastDebounce > 200) {
      Serial.println(">>> MANUAL: Close button pressed");
      closeCurtain();
      lastDebounce = millis();
    }
  }
  lastCloseState = closeButtonState;
}

void wirelessMode() {
  static bool lastWirelessState = HIGH;
  static unsigned long lastDebounce = 0;
  bool wirelessButtonState = digitalRead(BTN_WIRELESS);
  if (wirelessButtonState == LOW && lastWirelessState == HIGH) {
    if (millis() - lastDebounce > 200) {
      Serial.println(">>> WIRELESS: Button pressed - CYCLING curtain position");
      cycleCurtainPosition();
      lastDebounce = millis();
    }
  }
  lastWirelessState = wirelessButtonState;

  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'O':
      case 'o':
        Serial.println(">>> WIRELESS: Serial command 'O' - FULLY OPENING");
        fullyOpenCurtain();
        break;
      case 'H':
      case 'h':
        Serial.println(">>> WIRELESS: Serial command 'H' - HALF OPENING");
        halfOpenCurtain();
        break;
      case 'C':
      case 'c':
        Serial.println(">>> WIRELESS: Serial command 'C' - CLOSING");
        closeCurtain();
        break;
      case 'S':
      case 's':
        printStatus();
        break;
      case 'T':
      case 't':
        Serial.println(">>> WIRELESS: Serial command 'T' - CYCLING");
        cycleCurtainPosition();
        break;
    }
  }
}

void fullyOpenCurtain() {
  if (curtainState != FULLY_OPEN) {
    targetPosition = OPEN_ANGLE;
    curtainState = FULLY_OPEN;
    Serial.println(" Fully opening curtain...");
  } else {
    Serial.println(" Curtain already fully open!");
  }
}

void halfOpenCurtain() {
  if (curtainState != HALF_OPEN) {
    targetPosition = HALF_ANGLE;
    curtainState = HALF_OPEN;
    Serial.println(" Half opening curtain...");
  } else {
    Serial.println(" Curtain already half open!");
  }
}

void closeCurtain() {
  if (curtainState != CLOSED) {
    targetPosition = CLOSED_ANGLE;
    curtainState = CLOSED;
    Serial.println(" Closing curtain...");
  } else {
    Serial.println(" Curtain already closed!");
  }
}

void cycleCurtainPosition() {
  switch (curtainState) {
    case CLOSED:
      halfOpenCurtain();
      break;
    case HALF_OPEN:
      fullyOpenCurtain();
      break;
    case FULLY_OPEN:
      closeCurtain();
      break;
  }
}

// Moves the servo 2 degrees at a time with a 15 ms pause, so the curtain glides.
void moveServoSmooth() {
  if (currentPosition != targetPosition) {
    if (currentPosition < targetPosition) {
      currentPosition += 2;
      if (currentPosition > targetPosition) currentPosition = targetPosition;
    }
    else if (currentPosition > targetPosition) {
      currentPosition -= 2;
      if (currentPosition < targetPosition) currentPosition = targetPosition;
    }
    curtainServo.write(currentPosition);
    delay(15);
    if (currentPosition == targetPosition) {
      Serial.print(" ✓ Curtain ");
      printCurtainState();
      Serial.println();
    }
  }
}

void printCurtainState() {
  switch (curtainState) {
    case CLOSED:
      Serial.print("CLOSED ");
      break;
    case HALF_OPEN:
      Serial.print("HALF OPEN");
      break;
    case FULLY_OPEN:
      Serial.print("FULLY OPEN");
      break;
  }
}

void printMode() {
  Serial.println();
  Serial.println("========================================");
  Serial.print(" MODE CHANGED TO: ");
  switch (controlMode) {
    case 0:
      Serial.println("AUTOMATIC");
      Serial.println(" Curtain position based on light level:");
      Serial.print(" >"); Serial.print(LIGHT_HIGH); Serial.println(" = Fully Open");
      Serial.print(" "); Serial.print(LIGHT_MID); Serial.print("-"); Serial.print(LIGHT_HIGH); Serial.println(" = Half Open");
      Serial.print(" <"); Serial.print(LIGHT_LOW); Serial.println(" = Closed");
      Serial.println(" LED: Slow blink (1 sec)");
      break;
    case 1:
      Serial.println("MANUAL");
      Serial.println(" OPEN (Pin 2) = Fully Open");
      Serial.println(" HALF (Pin 6) = Half Open");
      Serial.println(" CLOSE (Pin 3) = Closed");
      Serial.println(" LED: Medium blink (0.5 sec)");
      break;
    case 2:
      Serial.println("WIRELESS");
      Serial.println(" Press WIRELESS button (Pin 5) to cycle");
      Serial.println(" OR type in Serial Monitor:");
      Serial.println(" O = Fully Open H = Half Open");
      Serial.println(" C = Close T = Cycle");
      Serial.println(" S = Status");
      Serial.println(" LED: Fast blink (0.2 sec)");
      break;
  }
  Serial.println("========================================");
  Serial.println();
}

void printStatus() {
  int lightLevel = analogRead(LDR_PIN);
  Serial.println();
  Serial.println("========== SYSTEM STATUS ==========");
  Serial.print("Control Mode: ");
  switch (controlMode) {
    case 0: Serial.println("AUTOMATIC"); break;
    case 1: Serial.println("MANUAL"); break;
    case 2: Serial.println("WIRELESS"); break;
  }
  Serial.print("Curtain State: ");
  printCurtainState();
  Serial.println();
  Serial.print("Servo Position: ");
  Serial.print(currentPosition);
  Serial.println("°");
  Serial.print("Light Level: ");
  Serial.print(lightLevel);
  Serial.print(" (High: >");
  Serial.print(LIGHT_HIGH);
  Serial.print(", Mid: ");
  Serial.print(LIGHT_MID);
  Serial.print("-");
  Serial.print(LIGHT_HIGH);
  Serial.print(", Low: <");
  Serial.print(LIGHT_LOW);
  Serial.println(")");
  Serial.println("===================================");
  Serial.println();
}
