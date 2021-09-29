/*! \mainpage MPU-6050, 6 DOF gyro and accelerometer

  Read data from MPU-6050 gyro.

  Acceleration values are displayed for a gnuplot 3D plot.

  ![Sketch](../Read_MPU_6050_Sketch.png)

  <pre>
  (c)2014-20206 Ingenieurbüro Dr. Friedrich Haase
             Consulting - Automatisierungstechnik

  Based on
  http://playground.arduino.cc/Main/MPU-6050

  date       name       action
  Dec 2014   Dr. Haase  created
  Sept 2016  Dr. Haase  minor updates
  Sept 2017  Dr. Haase  minor updates
  Nov 2020   Dr. Haase  modified for gnuplot
  </pre>
*/

//! standard Arduino header file
#include <Arduino.h>
//! include wire library header file
#include <Wire.h>

int    LEDpin = 13;                     //! usual arduino pin 13 LED

int    MPU = 0x68;                      //! I2C address of the MPU-6050

int    AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; //! raw values

int    nDelay = 1000;                   //! loop delay in msec

//! read from MPU-6050 starting with register 0x3B (ACCEL_XOUT_H)
void Read_MPU6050()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);      // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

//! usual arduino init function
void setup()
{
  Serial.begin(115200);                 // init arduino serial
#if defined (__AVR_ATmega32U4__)
  while ( ! Serial )
    ;                                   // wait for serial port, Arduino Leonardo only
#endif

  pinMode(LEDpin, OUTPUT);              // init arduino LED pin as an output

  Serial.println("# Read MPU-6050 acceleration and print for gnuplot");

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);                     // PWR_MGMT_1 register
  Wire.write(0);                        // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  delay(30);                            // MPU-6050 start-up time

  Read_MPU6050();                       // get initial values
}

//! usual arduino steadily called function
void loop()
{
  digitalWrite(LEDpin, ! digitalRead(LEDpin)); // toggle output to LED

  Read_MPU6050();

#if 1
  double        x, y, z, scale;
  static char   szB_X[40], szB_Y[40], szB_Z[40];
  String        s;


  x = AcX;
  y = AcY;
  z = AcZ;

  scale = sqrt(x * x + y * y + z * z);
  //  scale = max(x, max(y, z));

  x /=  scale;
  y /=  scale;
  z /=  scale;

  // cross and vector data for gnuplot splot
  s = char(12);                         // formfeed
  // cube
  s += "-0.1 0 0\n0.1 0 0\n\n\n0 -0.1 0\n0 0.1 0\n\n\n0 0 -0.1\n0 0 0.1\n\n\n0 0 0\n";

  dtostrf(x, 7, 1, szB_X);
  dtostrf(y, 7, 1, szB_Y);
  dtostrf(z, 7, 1, szB_Z);

  s += szB_X;                           // vector
  s += " ";
  s += szB_Y;
  s += " ";
  s += szB_Z;
  s += "\n\n\n";

  s += "-1 ";                           // X = [-1..1]
  s += szB_Y;
  s += " ";
  s += szB_Z;
  s += "\n1 ";
  s += szB_Y;
  s += " ";
  s += szB_Z;
  s += "\n\n\n";

  s += szB_X;                           // Y = [-1..1]
  s += " -1 ";
  s += szB_Z;
  s += "\n";
  s += szB_X;
  s += " 1 ";
  s += szB_Z;
  s += "\n\n\n";

  s += szB_X;                           // Z = [-1..1]
  s += " ";
  s += szB_Y;
  s += " -1\n";
  s += szB_X;
  s += " ";
  s += szB_Y;
  s += " 1\n\n\n";

  Serial.println(s);

  //  Serial.print("  "); Serial.print(AcX);
  //  Serial.print("  "); Serial.print(AcY);
  //  Serial.print("  "); Serial.print(AcZ);
  //  Serial.print("  "); Serial.print(scale);
  //  Serial.println();
#endif

#if 0
  // display raw values
  Serial.print("AcX="); Serial.print(AcX);
  Serial.print(" | AcY="); Serial.print(AcY);
  Serial.print(" | AcZ="); Serial.print(AcZ);
  //equation for temperature in degrees C from datasheet
  Serial.print(" | Tmp="); Serial.print(Tmp / 340.00 + 36.53);
  Serial.print(" | GyX="); Serial.print(GyX);
  Serial.print(" | GyY="); Serial.print(GyY);
  Serial.print(" | GyZ="); Serial.print(GyZ);
  Serial.println();
#endif

  delay(nDelay);
}
