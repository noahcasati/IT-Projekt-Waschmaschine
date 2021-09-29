/*! \mainpage MPU-6050, 6 DOF gyro and accelerometer

Read data from MPU-6050 gyro.

This sample shows how to fetch data from a MPU-6050.
Values are displayed either raw or PT1-filtered.

![Sketch](../Read_MPU_6050_Sketch.png)

![Schaltplan](../Read_MPU_6050_Schaltplan.png)

<pre>
(c)2014-2016 Ingenieurbüro Dr. Friedrich Haase
             Consulting - Automatisierungstechnik

Based on
http://playground.arduino.cc/Main/MPU-6050

date       name       action
Dec 2014   Dr. Haase  created
Sept 2016  Dr. Haase  minor updates
Sept 2017  Dr. Haase  minor updates
</pre>
*/

//! standard Arduino header file
#include <Arduino.h>
//! include wire library header file
#include <Wire.h>

int    LEDpin = 13;                     //! usual arduino pin 13 LED

int    MPU=0x68;                        //! I2C address of the MPU-6050

int    AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;     //! raw values

int    nDelay = 1000;                   //! loop delay in msec

//! read from MPU-6050 starting with register 0x3B (ACCEL_XOUT_H)
void Read_MPU6050()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);      // request a total of 14 registers
  AcX=Wire.read() << 8 | Wire.read();   // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read() << 8 | Wire.read();   // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read() << 8 | Wire.read();   // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read() << 8 | Wire.read();   // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read() << 8 | Wire.read();   // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read() << 8 | Wire.read();   // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read() << 8 | Wire.read();   // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

//! usual arduino init function
void setup()
{
  Serial.begin(9600);                   // init arduino serial
#if defined (__AVR_ATmega32U4__)
  while ( ! Serial )
    ;                                   // wait for serial port, Arduino Leonardo only
#endif

  pinMode(LEDpin, OUTPUT);              // init arduino LED pin as an output

  Serial.println("# Read MPU-6050 V2.02");

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
  delay(nDelay);

  Read_MPU6050();
                                        // display raw values
  Serial.print("AcX="); Serial.print(AcX);
  Serial.print(" | AcY="); Serial.print(AcY);
  Serial.print(" | AcZ="); Serial.print(AcZ);
  //equation for temperature in degrees C from datasheet
  Serial.print(" | Tmp="); Serial.print(Tmp/340.00+36.53);
  Serial.print(" | GyX="); Serial.print(GyX);
  Serial.print(" | GyY="); Serial.print(GyY);
  Serial.print(" | GyZ="); Serial.print(GyZ);
  Serial.println();
}
