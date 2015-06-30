/*
  Reading PWM from Interrupts and ICP -- Compare Methods
*/

const int inputInterruptPin = 2;
const int inputCapturePin = 48; // Oder 49

volatile unsigned long pwm_val = 0;
volatile unsigned long tmp_count = 0;

#define inter0 (PINE & 0b00010000)

const int prescale = 8;
const byte prescaleBits = B010; // Prescale Factor factor8 = B010; factor1 = B001;
const long precision = (1000000/(F_CPU / 1000)) * prescale;

const int numberOfEntries = 8;
const int countSamplePeriod = 350;

volatile byte index = 0;
volatile byte count_enabled = 0; // 0 disables - 1 Enables
volatile unsigned int results[numberOfEntries]; // 16-bit value
volatile int overflows = 0;

unsigned long pulse_ON = 0;  // Time of enconder on
unsigned long pulse_OFF = 0; // Time of enconder off

unsigned long pulse_ON_avg = 0;  // Average time on
unsigned long pulse_OFF_avg = 0; // Average time off

const float encConversion = 359.91; // Rotary encoder conversion constant
float angle = 0.0; // Should be float?
unsigned long signalDuration = 0;

/* ICR interrupt vector */
ISR(TIMER5_CAPT_vect) {
  TCNT5 = 0; // Reset the counter
  if (count_enabled) {
    if (index != 0 || bitRead(TCCR5B, ICES5) == true) { // Wait Raising Edge
      if (index < numberOfEntries) {
        results[index] = ICR5; // Save the InputCaptureRegister
        index++;
      }
    }
  }
  TCCR5B ^= _BV(ICES5);  // togge bit to trigger on the other edge
}
/* Allow Overflow interruptions from timer5 ATMega2560*/
ISR(TIMER5_OVF_vect) {
  overflows++;
}

void analyze() {
  if (inter0)
    tmp_count = micros(); // positive edge
  else
    pwm_val = micros() - tmp_count; // negative edge; get pulse width
}

void setup() {
  pinMode(inputCapturePin, INPUT);
  pinMode(inputInterruptPin, INPUT);

  TCCR5A = 0; // Normal Counting Mode
  TCCR5B = prescaleBits; // set prescale bits
  TCCR5B |= _BV(ICES5); // enable input capture

  bitSet(TIMSK5, ICIE5); // enable input capture interrupt for timer5
  bitSet(TIMSK5, TOIE5); // Enable overflow interrupt

  Serial.begin(9600);
  Serial.print("Pulses with precision: ");
  Serial.println(precision); // report duration of each tick in nanoseconds
  Serial.println("Ready for compare");
  attachInterrupt(0, analyze, CHANGE);
}

void loop() {
  count_enabled = 1;
  delay(countSamplePeriod);
  count_enabled = 0;
  if (index > 0) {
    //Serial.println("Duration in microseconds are: ");
    for (byte i=0; i < numberOfEntries; i++) {
      if (i % 2 == 0) {
        pulse_OFF += results[i] * precision;
      }
      else {
        pulse_ON += results[i] * precision;
      }
    }
    //Serial.println(results[1]);
    pulse_ON_avg = pulse_ON / (500 * numberOfEntries);
    pulse_OFF_avg = pulse_OFF / (500 * numberOfEntries);
    signalDuration = (pulse_ON_avg + pulse_OFF_avg);
    /*angle = ((pulse_ON_avg * signalDuration)/(pulse_ON_avg + pulse_OFF_avg)) - 1;*/
    /*angle = (angle * encConversion) / signalDuration;*/
    Serial.print("signal length: ");
    Serial.println(signalDuration , DEC); // duration in microseconds
    Serial.print("high length: ");
    Serial.println(pulse_ON_avg , DEC); // duration in microseconds
    Serial.print("low length: ");
    Serial.println(pulse_OFF_avg , DEC); // duration in microseconds
    Serial.print("overflows: ");
    Serial.println(overflows, DEC);
    /*Serial.print("angle: ");*/
    /*Serial.println(angle , DEC); // duration in microseconds*/
    index = 0;
    pulse_OFF = 0;
    pulse_ON = 0;
    overflows = 0;
  }
    Serial.println(pwm_val, DEC);
}
