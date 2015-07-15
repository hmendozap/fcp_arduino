/*
  Input Capture - Recipe 18.7 Arduino Cookbook
*/

const int inputCapturePin = 48; // Oder 49

const int prescale = 8;
const byte prescaleBits = B010; // Prescale Factor
/*const long precision = (1000000/(F_CPU / 1000))*prescale;*/
const long precision = 500;

const int numberOfEntries = 4;
const int countSamplePeriod = 500;

volatile byte index = 0;
volatile byte count_enabled = 0; // 0 disables - 1 Enables
volatile unsigned int results[numberOfEntries]; // 16-bit value
volatile int overflows = 0;

unsigned long pulse_ON = 0;  // Time of enconder on
unsigned long pulse_OFF = 0; // Time of enconder off

unsigned long pulse_ON_avg = 0;  // Average time on
unsigned long pulse_OFF_avg = 0; // Average time off

const float encConversion = 359.91; // Rotary encoder conversion constant
float angle = 0.0;
unsigned long signalDuration = 0;

/* ICR interrupt vector */
ISR(TIMER5_CAPT_vect) {
  TCNT5 = 0; // Reset the counter
  if (count_enabled) {
    if (bitRead(TCCR5B, ICES5) == true) { // Wait rising edge
      if (index < numberOfEntries) {
        results[index] = ICR5; // Save the InputCompareRegister -- in microseconds
        index++;
      }
    }
  }
  /*TCCR5B ^= _BV(ICES5);  // togge bit to trigger on the other edge*/
}

/* Allow Overflow interruptions from timer5 ATMega2560*/
ISR(TIMER5_OVF_vect) {
  overflows++;
}

void setup() {
  Serial.begin(9600);
  pinMode(inputCapturePin, INPUT);  // ICP pin as input
  TCCR5A = 0; // Normal Counting Mode
  TCCR5B = prescaleBits; // set prescale bits
  TCCR5B |= _BV(ICES5); // enable input capture

  OCR5A = 0;
  OCR5B = 0;
  bitSet(TIMSK5, ICIE5); // enable input capture interrupt for timer5
  bitSet(TIMSK5, TOIE5); // Enable overflow interrupt
  Serial.print(F_CPU);
  Serial.print("Pulses with precision: ");
  Serial.println(precision); // report duration of each tick in nanoseconds
}

// This loops prints the numbe of pulses in the last second, showing min and max pulse widths
void loop() {
  count_enabled = 1; // enable sampling
  delay(countSamplePeriod);
  count_enabled = 0; // disable sampling
  if (index > 0) {
    //Serial.println("Duration in microseconds are: ");
    for (byte i=0; i < numberOfEntries; i++) {
      unsigned long duration;
      duration = results[i] * precision; //pulse duration in nanoseconds
      Serial.println(duration/ 1000); // duration in microseconds
      if (i % 2 == 0) {
        pulse_OFF += results[i] * precision;
      }
      else {
        pulse_ON += results[i] * precision;
      }
    }
    pulse_ON_avg = pulse_ON / (500 * numberOfEntries);
    pulse_OFF_avg = pulse_OFF / (500 * numberOfEntries);
    signalDuration = (pulse_ON_avg + pulse_OFF_avg);
    angle = ((pulse_ON_avg * signalDuration)/(pulse_ON_avg + pulse_OFF_avg)) - 1;
    angle = (angle * encConversion) / signalDuration;
    //Serial.print("signal length: ");
    //Serial.println(signalDuration , DEC); // duration in microseconds
    //Serial.print("high length: ");
    //Serial.println(pulse_ON_avg , DEC); // duration in microseconds
    //Serial.print("low length: ");
    //Serial.println(pulse_OFF_avg , DEC); // duration in microseconds
    if (overflows == 0) {
      Serial.print("angle: ");
      Serial.println(angle , DEC); // duration in microseconds
    }
    Serial.print("overflows: ");
    Serial.println(overflows, DEC);
    index = 0;
    pulse_OFF = 0;
    pulse_ON = 0;
    overflows=0;
  }
}
