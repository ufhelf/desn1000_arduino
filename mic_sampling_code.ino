
const int micPinLeft = A0;
const int micPinRight = A1;
const int irPin = A2;

const float VREF = 5.0;
const int ADC_MAX = 1023;
const float MICROPHONE_DIFFERENCE_TOLERANCE = 0.01;
const float IR_MIN_ON_VOLTAGE = 0.5;

const unsigned long SAMPLE_TIME = 100000;

void calculateRMS(float *rmsLeft, float *rmsRight) {
  unsigned long startTime = micros();

  double sumLeft = 0, sumRight = 0;
  double sumSqLeft = 0, sumSqRight = 0;
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

  *rmsLeft = sqrt(sumSqLeft / sampleCount - meanLeft * meanLeft);
  *rmsRight = sqrt(sumSqRight / sampleCount - meanRight * meanRight);
}

void input_processing(float rmsLeft, float rmsRight, float irVoltage) {
  float difference = abs(rmsLeft - rmsRight);

  //If ir sensor is fully off --> motor should not be turning
  //if ()

  if (difference < MICROPHONE_DIFFERENCE_TOLERANCE) {
    Serial.print("Equal\n");
  } else if (rmsLeft > rmsRight) {
    Serial.print("Wind left\n");
  } else {
    Serial.print("Wind right\n");
  }
}

void setup() {
    Serial.begin(9600);
}

void loop() {
  float rmsLeft, rmsRight;

  calculateRMS(&rmsLeft, &rmsRight);

  //Serial.print("Left RMS: ");
  //Serial.print(rmsLeft, 8);

  //Serial.print(" V | Right RMS: ");
  //Serial.println(rmsRight, 4);

  input_processing(rmsLeft, rmsRight, 1);

  delay(100);
}
