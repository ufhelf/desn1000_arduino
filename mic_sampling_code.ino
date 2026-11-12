
#include <math.h>

// Pin configuration
const int micPinLeft = A0;
const int micPinRight = A1;
const int irPin = A2;

const int forwardOutPin = 2;
const int backwardOutPin = 4;

// Voltage settings
const float VREF = 5.0;
const int ADC_MAX = 1023;

const float MICROPHONE_DIFFERENCE_TOLERANCE = 0.008;
const float MIN_MIC_VOLTAGE = 0.004;
const float IR_MIN_ON_VOLTAGE = 3;

// Sampling settings
const unsigned long SAMPLE_TIME = 100000;

// Motor timing
const unsigned long deadTime = 100;

int currentDirection = 0;
//  0 = stopped
//  1 = left
// -1 = right


// =====================================
// MOTOR CONTROL
// =====================================

void motorStop() {
  digitalWrite(forwardOutPin, LOW);
  digitalWrite(backwardOutPin, LOW);
}

void setMotor(int direction) {

  if (direction == currentDirection) {
    return;
  }

  // Stop both outputs before changing direction
  motorStop();

  // Wait only when changing between directions
  if (currentDirection != 0 && direction != 0) {
    delay(deadTime);
  }

  if (direction == 1) {
    digitalWrite(forwardOutPin, HIGH);
    Serial.println("Motor LEFT");
  }
  else if (direction == -1) {
    digitalWrite(backwardOutPin, HIGH);
    Serial.println("Motor RIGHT");
  }

  currentDirection = direction;
}


// =====================================
// RMS CALCULATION
// =====================================

void calculateRMS(float *rmsLeft, float *rmsRight) {

  unsigned long startTime = micros();

  double sumLeft = 0;
  double sumRight = 0;

  double sumSqLeft = 0;
  double sumSqRight = 0;

  int sampleCount = 0;

  while (micros() - startTime < SAMPLE_TIME) {

    float left = analogRead(micPinLeft) * VREF / ADC_MAX;
    float right = analogRead(micPinRight) * VREF / ADC_MAX;

    sumLeft += left;
    sumRight += right;

    sumSqLeft += left * left;
    sumSqRight += right * right;

    sampleCount++;
  }

  float meanLeft = sumLeft / sampleCount;
  float meanRight = sumRight / sampleCount;

  float varianceLeft =
      sumSqLeft / sampleCount - meanLeft * meanLeft;

  float varianceRight =
      sumSqRight / sampleCount - meanRight * meanRight;

  if (varianceLeft < 0) varianceLeft = 0;
  if (varianceRight < 0) varianceRight = 0;

  *rmsLeft = sqrt(varianceLeft);
  *rmsRight = sqrt(varianceRight);
}


// =====================================
// INPUT PROCESSING
// =====================================

void input_processing(float rmsLeft, float rmsRight, float irVoltage) {

  // Check IR sensor first
  if (irVoltage < IR_MIN_ON_VOLTAGE) {

    Serial.println("IR OFF - MOTOR STOPPED");

    setMotor(0);
    return;
  }

  float difference = abs(rmsLeft - rmsRight);

  // Both microphones quiet or approximately equal
  if (difference < MICROPHONE_DIFFERENCE_TOLERANCE ||
      (rmsLeft < MIN_MIC_VOLTAGE &&
       rmsRight < MIN_MIC_VOLTAGE)) {

    Serial.println("Equal");

    setMotor(0);
  }

  // Left microphone louder
  else if (rmsLeft > rmsRight) {

    Serial.println("Wind left");

    setMotor(1);
  }

  // Right microphone louder
  else {

    Serial.println("Wind right");

    setMotor(-1);
  }
}


// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(9600);

  pinMode(forwardOutPin, OUTPUT);
  pinMode(backwardOutPin, OUTPUT);

  pinMode(irPin, INPUT);

  motorStop();
}


// =====================================
// MAIN LOOP
// =====================================

void loop() {

  float rmsLeft;
  float rmsRight;

  // Read microphone RMS
  calculateRMS(&rmsLeft, &rmsRight);

  // Read IR sensor voltage
  float irVoltage = analogRead(irPin) * VREF / ADC_MAX;

  // Print sensor readings
  //Serial.print("Left RMS: ");
  //Serial.print(rmsLeft, 6);

  //Serial.print(" | Right RMS: ");
  //Serial.print(rmsRight, 6);

  //Serial.print(" | IR Voltage: ");
  //Serial.println(irVoltage, 3);

  // Process all inputs
  input_processing(rmsLeft, rmsRight, irVoltage);
}
