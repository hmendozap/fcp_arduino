#include <Vanes.h>
#include <Wire.h>

#define I2CAdress 2

uint8_t packageID = 0;
uint8_t buffer[5];
volatile float alphaAngleRaw;
volatile int alphaAngle;
volatile int betaAngle;
// Change in frequency - Cookbook Page 566
const byte mask = B11111000; // mask for TIMER3-TCCR3B
int prescale = 0x01; // mask for prescale

Vanes alpha(3);
//Vanes beta(3);

void setup()
{
TCCR3B = (TCCR3B & mask) | prescale;
Serial.begin(9600);
Serial.println("ready for calibration");
delay(1000);
alpha.calibrate();
//beta.calibrate();
Wire.begin(I2CAdress);
Wire.onRequest(sendData);
}

void loop()
{
/* get values from sensors */
alphaAngleRaw = alpha.getVal();
Serial.print("raw val: ");
Serial.println(alphaAngleRaw);
alphaAngle =(int) (alpha.getVal() * 100);
//betaAngle =(int) (beta.getVal() * 100);
packageID += 1;


/*prepare data for I2C transmission*/

buffer[0] = packageID;
buffer[1] = alphaAngle >> 8;
buffer[2] = alphaAngle & 255;
//buffer[3] = betaAngle >> 8;
//buffer[4] = betaAngle & 255;
printSerial();
delay(250);

}

void sendData()
{
Wire.write(buffer, 5);

}

void printSerial()
{
 uint16_t alph = buffer[1];
 alph = alph << 8;
 alph = alph | buffer[2];

 //uint16_t beth = buffer[3];
 //beth = beth << 8;
 //beth = beth | buffer[4];

 Serial.print(" ID: ");
 Serial.print(buffer[0]);
 Serial.print("  alpha: ");
 Serial.println(alph);
 //Serial.print("  beta: ");
 //Serial.println(beth);

}
