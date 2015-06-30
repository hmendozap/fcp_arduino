/*
  Reading PWM from Interrupts
  Code taken from cheapscience.com/2010/01/reading-servo-pwm-with-an-arduino.html
*/

const int inputInterruptPin = 2;
const int inputCapturePin = 48; // Oder 49
volatile long pwm_val = 0;
volatile long tmp_count = 0;

#define inter0 (PINE & 0b00010000) // Faster than digital read ???

void analyze() {
  if (inter0)
    tmp_count = micros(); // Positive edge
  else
    pwm_val = micros() - tmp_count; // Negative edge; get Pulse width
}

void setup() {
  pinMode(inputCapturePin, INPUT);
  pinMode(inputInterruptPin, INPUT);
  Serial.begin(9600);
  Serial.println("Ready for counting");
  attachInterrupt(0, analyze, CHANGE);
}

void loop() {
  delay(150);
  Serial.println(pwm_val, DEC);
}
