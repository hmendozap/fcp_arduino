/*
  Vanes implementation for interruption based pulse count
  author: HM
  Based on Arduino Cookbook
*/

int signalLength = 0;
unsigned long accum = 0;

#define inter0 (PIND & 0b00000100) // To start with positive edge
#define inter1 (PIND & 0b00001000) // To start with positive edge

const byte mask = B11111000;
const int prescale = 0x01;

const int hrzInterruptPin = 2; // Horizontal sensor
const int vertInterruptPin = 3; // Vertical sensor
const int numberOfEntries = 5; // Actually just 4 (+1 to discard)
const int averageEntries = 32;

const int countSamplePeriod = 50; // numberOfEntries * AvePulseWidth ~= 20ms
const int calibrationPeriod = 510;

volatile unsigned long microhorz = 0;
volatile unsigned long microverts = 0;
volatile unsigned long microsecs = 0;
volatile byte vert_index = 0;
volatile byte horz_index = 0;
volatile byte index = 0;
volatile byte sample_enabled = 0;
volatile byte calibration_enabled = 0;
volatile unsigned long results_hrz[numberOfEntries];
volatile unsigned long results_vert[numberOfEntries];
volatile unsigned long calibration[averageEntries];

unsigned long vertON = 0;  // Time of enconder on
unsigned long vertON_avg = 0;  // Average time on

unsigned long horzON = 0;  // Time of enconder on
unsigned long horzON_avg = 0;  // Average time on

const int CALIBRATIONCOUNT = 1000;
const int MAX_WIDTH_PULSE = 4302; // From MA3-P12 datasheet
int count = CALIBRATIONCOUNT;

const float encConversion = 359.91; // Rotary encoder conversion constant
float angleVert = 0.0;
float angleHorz = 0.0;
unsigned long signalVert = 0;
unsigned long signalHorz = 0;
int vert_valid = 0;
int horz_valid = 0;

void setup() {
  pinMode(hrzInterruptPin, INPUT);
  pinMode(vertInterruptPin, INPUT);
  TCCR1B = TCCR1B & mask | prescale;
  Serial.begin(9600);
  Serial.println("Ready for counting");
  attachInterrupt(0, analyze_Horz, CHANGE);
  attachInterrupt(1, analyze_Vert, CHANGE);
  results_hrz[0] = 0;
  results_vert[0] = 0;
}

void loop() {
  if (count == CALIBRATIONCOUNT) {
    signalHorz = mCalibration(0);
    signalVert = mCalibration(1);
    count = 0;
  }
  sample_enabled = 1; // enable sampling
  delay(countSamplePeriod);
  sample_enabled = 0; // disable sampling
  /*Serial.println(vert_index);*/
  if (vert_index > 0 || horz_index > 0) {
    /*Serial.println(signalVert);*/
    /*Serial.println(signalHorz);*/
    for (byte i=1; i < numberOfEntries; i++) {
      /*Serial.println(results_vert[i]);*/
      /*Serial.println(results_hrz[i]);*/
      if (results_vert[i] > 0 && results_vert[i] <= signalVert) {
        vertON += results_vert[i];
        vert_valid++;
      }
      if (results_hrz[i] > 0 && results_hrz[i] <= signalHorz) {
        horzON += results_hrz[i];
        horz_valid++;
      }
    }
    vertON_avg = vertON / vert_valid;
    horzON_avg = horzON / horz_valid;

    if (vertON_avg > 0 && vertON_avg <= signalVert) {
      angleVert = ((vertON_avg-1) * encConversion) / signalVert;
    } else {
      angleVert = 0;
    }
    if (horzON_avg > 0 && horzON_avg <= signalHorz) {
      angleHorz = ((horzON_avg-1) * encConversion) / signalHorz;
    } else {
      angleHorz = 0;
    }

    Serial.print("vertical angle:\t\t");
    Serial.println(angleVert , DEC);
    Serial.print("horizontal angle:\t");
    Serial.println(angleHorz , DEC);

    vert_index = 0;
    vert_valid = 0;
    vertON = 0;
    horz_index = 0;
    horz_valid = 0;
    horzON = 0;
    count++;
  }
}

void analyze_Vert() {
  if (sample_enabled) {
    if (vert_index < numberOfEntries) {
      if (inter1) {
        microverts = micros();
      } else {
        results_vert[vert_index] = micros() - microverts;
        vert_index++;
      }
    }
  }
}

void analyze_Horz() {
  if (sample_enabled) {
    if (vert_index < numberOfEntries) {
      if (inter0) {
        microhorz = micros();
      } else {
        results_hrz[horz_index] = micros() - microhorz;
        horz_index++;
      }
    }
  }
}

unsigned long mCalibration(int pPin) {
  detachInterrupt(pPin);
  int valid_accum = 0;
  attachInterrupt(pPin, calibrate, RISING);
  /*Serial.println("Starting calibration");*/
  calibration_enabled = 1;
  delay(calibrationPeriod);
  calibration_enabled = 0;
  if (index >= averageEntries) {
    for (byte i=1; i < averageEntries; i++) {
      /*Serial.println(calibration[i]);*/
      if (calibration[i] <= MAX_WIDTH_PULSE) {
        accum += calibration[i];
        valid_accum++;
      }
    }
    signalLength =  accum / (valid_accum);
    Serial.print("signal Length: ");
    Serial.println(signalLength);
  }
  accum = 0;
  index = 0;
  valid_accum = 0;
  detachInterrupt(pPin);
  if (pPin) {
    attachInterrupt(pPin, analyze_Vert, CHANGE);
  } else {
    attachInterrupt(pPin, analyze_Horz, CHANGE);
  }
  return signalLength;
}

void calibrate() {
  if (calibration_enabled) {
    if (index < averageEntries){
      if (index > 0) {
        calibration[index] = micros() - microsecs;
      }
      index++;
    }
    microsecs = micros();
  }
}
