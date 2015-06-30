/*
  Vanes implementation for interruption based pulse count
  authrz: HM
  Based on Arduino Cookbook
*/

int signalLength = 0;
unsigned long accum = 0;

#define inter0 (PINE & 0b00010000) // To start with positive edge

const int hrzInterruptPin = 2; // Horizontal sensor
const int vertInterruptPin = 3; // Vertical sensor
const int numberOfEntries = 64;

const int countSamplePeriod = 1000;

volatile unsigned long microhorz = 0;
volatile unsigned long microverts = 0;
volatile byte vert_index = 0;
volatile byte horz_index = 0;
volatile byte sample_enabled = 0;
volatile unsigned long results_hrz[numberOfEntries];
volatile unsigned long results_vert[numberOfEntries];

unsigned long vertON = 0;  // Time of enconder on
unsigned long vertOFF = 0; // Time of enconder off
unsigned long vertON_avg = 0;  // Average time on
unsigned long vertOFF_avg = 0; // Average time off

unsigned long horzON = 0;  // Time of enconder on
unsigned long horzOFF = 0; // Time of enconder off
unsigned long horzON_avg = 0;  // Average time on
unsigned long horzOFF_avg = 0; // Average time off

const float encConversion = 359.91; // Rotary encoder conversion constant
float angleVert = 0.0;
float angleHorz = 0.0;
unsigned long signalVert = 0;
unsigned long signalHorz = 0;

void setup() {
  pinMode(hrzInterruptPin, INPUT);
  pinMode(vertInterruptPin, INPUT);

  Serial.begin(9600);
  Serial.println("Ready for counting");
  attachInterrupt(0, analyze_Horz, CHANGE);
  attachInterrupt(1, analyze_Vert, CHANGE);
  results_hrz[0] = 0;
  results_vert[0] = 0;
}

void loop() {
  sample_enabled = 1; // enable sampling
  delay(countSamplePeriod);
  sample_enabled = 0; // disable sampling

  if (vert_index > 0) {
    Serial.println("Duration in microsecs are: ");
    for (byte i=0; i < numberOfEntries; i++) {
      /*Serial.println(results_vert[i]);*/
      if (i % 2 == 0) {
        vertOFF += results_vert[i];
        horzOFF += results_hrz[i];
      }
      else {
        vertON += results_vert[i];
        horzON += results_hrz[i];
      }
    }
    vertON_avg = (2 * vertON) / (numberOfEntries);
    vertOFF_avg = (2 * vertOFF) / (numberOfEntries);
    horzON_avg = (2 * horzON) / (numberOfEntries);
    horzOFF_avg = (2 * horzOFF) / (numberOfEntries);

    signalVert = (vertON_avg + vertOFF_avg);
    signalHorz = (horzON_avg + horzOFF_avg);

    angleVert = ((vertON_avg * signalVert)/(vertON_avg + vertOFF_avg)) - 1;
    angleVert = (angleVert * encConversion) / signalVert;
    angleHorz = ((horzON_avg * signalHorz)/(horzON_avg + horzOFF_avg)) - 1;
    angleHorz = (angleHorz * encConversion) / signalHorz;

    Serial.print("vertical angle: ");
    Serial.println(angleVert , DEC);
    Serial.print("horizontal angle: ");
    Serial.println(angleHorz , DEC);

    vert_index = 0;
    vertOFF = 0;
    vertON = 0;
    horz_index = 0;
    horzOFF = 0;
    horzON = 0;
  }
}

void analyze_Vert() {
  if (sample_enabled) {
    if (vert_index != 0 || !inter0) {
      if (vert_index < numberOfEntries) {
        results_vert[vert_index] = micros() - microverts;
      }
      vert_index++;
    }
    microverts = micros();
  }
}

void analyze_Horz() {
  if (sample_enabled) {
    if (horz_index != 0 || !inter0) {
      if (horz_index < numberOfEntries) {
        results_hrz[horz_index] = micros() - microhorz;
      }
      horz_index++;
    }
    microhorz = micros();
  }
}
