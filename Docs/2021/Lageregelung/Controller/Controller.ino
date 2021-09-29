/*! \mainpage Controller

A simple starter for a controller.

This sample does not control anything.
It just shows how to execute some code at regular spaced intervals.

It also includes the libraries for I2C and servos but does not use them in any way.

Prepared for doxygen documentaion.

<pre>
(c)2014-2020 Ingenieurbüro Dr. Friedrich Haase
             Consulting - Automatisierungstechnik
</pre>

<pre>
Date        Name       Action
Sept 2016   Dr. Haase  starter sample
Sept 2017   Dr. Haase  minor updates
Aug 2018    Dr. Haase  minor updates
June 2020   Dr. Haase  better C++ and better doxygen support
</pre>
*/

#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

const int       LED = 13;                     ///< standard Arduino LED at pin 13

unsigned long   nTime = 0;                    ///< timer tick counter in msec

char            szLine[40];                   ///< manual command line storage

//! usual arduino init function
/*!
Usual arduino init function.

Does some initialisation and nothing more.
*/
void setup()
{
  Serial.begin(115000);                       // init arduino serial
#if defined (__AVR_ATmega32U4__)
  while ( ! Serial )
    ;                                         // wait for serial port, ATmega 32U4 only
#endif

  pinMode(LED, OUTPUT);

  Serial.println("# Controller, empty starter V2.03");
}

//! Command line fetcher
/*!
Get a command line if available.
\param pszCommand command line storage
\param nSize command line size
\returns true on success
*/
bool HasCommand(char* pszCommand, size_t nSize)
{
  if ( Serial.available() > 0 )
  {                                           // work on manual command
    int          ch;
    size_t       nLen = 0;
    while ( ( ch = Serial.read() ) > 0 )
    {
      if ( nLen < nSize )
      {
        pszCommand[nLen++] = ch;              // collect command characters
        pszCommand[nLen] = 0;                 // append an EOL byte
      }
    }
    return true;                              // command received
  }
  return false;                               // no command
}

//! command interpreter
/*!
Command interpreter.
\param pszCommand command line storage
*/
void DoCommand(char* pszCommand)
{
  Serial.println(pszCommand);                 // display command
  // work on command
  *pszCommand = 0;                            // clear command
}

//! usual arduino steadily called function
/*!
Usual arduino steadily called function.

This function is called as fast and as often as possible by
the underlying Arduino system software.
*/
void loop()
{
  nTime = millis();                           // update time

  if ( HasCommand(szLine, sizeof(szLine) ) )  // anything to do?
    DoCommand(szLine);                        // yes, do it

  static unsigned long    nTimeLast = 0;      // storage to compare nTime with
  if ( ( nTime - nTimeLast ) >= 1000 )        // 1000 ticks for 1 sec
  {
    nTimeLast = nTime;                        // save for next second
    digitalWrite(LED, ! digitalRead(LED));    // toggle LED every second
//  Serial.println(nTime);
  }
}
