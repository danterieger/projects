// Importing Libraries
#include <Servo.h>
#include <IRremote.h>
#include <LiquidCrystal.h>

// LCD Initialization (Parallel 16x2 LCD using available pins)
LiquidCrystal lcd(4, 5, 6, 8, 10, 11); 

// Photoresistor / Function Generator input pin
const int ldrPin = A0;

// Microservo Pin
const int servoPin = 9;

// Curtain Open and Close Pushbuttons
const int buttonOpenPin = 2;
const int buttonClosePin = 12;

// IR Receiver Signal Pin
const int irPin = 7; 

// Servomotor created in TinkerCad environment
Servo curtainServo;

// Modes
enum Mode { AUTO, MANUAL, WIRELESS };
Mode currentMode = AUTO; 

// Light thresholds
const int lightThresholdOpen = 700;
const int lightThresholdClose = 300;

// Track curtain position/state
bool curtainOpen = false; 

// Noise control flag
bool noiseEnabled = false;

// Setup Function 
void setup() {
  Serial.begin(9600);  
  curtainServo.attach(servoPin);
  pinMode(buttonOpenPin, INPUT_PULLUP);
  pinMode(buttonClosePin, INPUT_PULLUP);

  IrReceiver.begin(irPin, ENABLE_LED_FEEDBACK);

  // LCD Setup
  lcd.begin(16, 2);         
  lcd.setCursor(0, 0);      
  lcd.print("Smart Curtain");
  updateLCD(); // Display mode & noise status

  // Setup confirmation in Serial
  Serial.println("Smart Curtain System Initialized");
  Serial.println("Current Mode: AUTO");
  Serial.println("Type 'NOISE' in Serial Monitor to toggle noise ON/OFF");
}

// Main Loop 
void loop() {
  checkModeSwitch(); 

  switch (currentMode) {
    case AUTO:
      autoControl();
      break;
    case MANUAL:
      manualControl();
      break;
    case WIRELESS:
      wirelessControl();
      break;
  }

  delay(100);
}

// Auto Mode Function
void autoControl() {
  int lightLevel = analogRead(ldrPin);

  // Add software noise if enabled
  if (noiseEnabled) {
    lightLevel += random(-50, 50);   // noise ±50
    lightLevel = constrain(lightLevel, 0, 1023);
  }

  // Serial output showing if noise is ON or OFF
  if (noiseEnabled) {
    Serial.print("Light Level (noise ON): ");
  } else {
    Serial.print("Light Level (noise OFF): ");
  }
  Serial.println(lightLevel);

  // Curtain logic
  if (curtainOpen) {
    if (lightLevel < lightThresholdClose) closeCurtain();
  } else {
    if (lightLevel > lightThresholdOpen) openCurtain();
  }
}

// Manual Mode Function
void manualControl() {
  if (digitalRead(buttonOpenPin) == LOW) {
    openCurtain();
    delay(300);
  }
  if (digitalRead(buttonClosePin) == LOW) {
    closeCurtain();
    delay(300);
  }
}

// Wireless Mode Function
void wirelessControl() {
  if (IrReceiver.decode()) {
    int value = IrReceiver.decodedIRData.command;
    Serial.print("IR Command Received: ");
    Serial.println(value);

    // Example button mapping
    if (value == 10) openCurtain();
    else if (value == 8) closeCurtain();
    else if (value == 5) toggleCurtain();
    else if (value == 2) toggleNoise(); // IR noise toggle

    IrReceiver.resume(); // Ready for next input
  }
}

// Curtain Control Functions 
void openCurtain() {
  if (!curtainOpen) {
    curtainServo.write(0);
    curtainOpen = true;
    Serial.println("Curtain Opened (Servo = 0°)");
  }
}

void closeCurtain() {
  if (curtainOpen) {
    curtainServo.write(180);
    curtainOpen = false;
    Serial.println("Curtain Closed (Servo = 180°)");
  }
}

void toggleCurtain() {
  if (curtainOpen) closeCurtain();
  else openCurtain();
}

// Noise Functions
void toggleNoise() {
  noiseEnabled = !noiseEnabled;
  Serial.print("Noise is now ");
  Serial.println(noiseEnabled ? "ON" : "OFF");
  updateLCD();  // update LCD with new noise status
}

// Mode Switching and Serial Commands
void checkModeSwitch() {
  if (Serial.available()) {
    String modeCommand = Serial.readStringUntil('\n');
    modeCommand.trim();

    if (modeCommand.equalsIgnoreCase("AUTO")) currentMode = AUTO;
    else if (modeCommand.equalsIgnoreCase("MANUAL")) currentMode = MANUAL;
    else if (modeCommand.equalsIgnoreCase("WIRELESS")) currentMode = WIRELESS;
    else if (modeCommand.equalsIgnoreCase("NOISE")) toggleNoise();

    // States Current Mode to Serial Monitor
    Serial.print("Mode changed to: ");
    if (currentMode == AUTO) Serial.println("AUTO");
    else if (currentMode == MANUAL) Serial.println("MANUAL");
    else Serial.println("WIRELESS");

    updateLCD();  // update LCD with mode and noise info
  }
}

// Helper function to update LCD
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Curtain");

  lcd.setCursor(0, 1);
  lcd.print("Mode:");
  if (currentMode == AUTO) lcd.print("AUTO");
  else if (currentMode == MANUAL) lcd.print("MANUAL");
  else lcd.print("WIRELESS");

  lcd.setCursor(12, 1); // Show noise status on same row
  if (noiseEnabled) lcd.print("N:ON");
  else lcd.print("N:OFF");
}