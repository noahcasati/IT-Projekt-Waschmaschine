/*! \mainpage Controller

Simple fabrication controller.

The Controller running on an Arduino will transfer manual commands from the 
USB/COM port to the physical ports or to the I²C port and displays the outcome from a connected plant.

The goal is to control the bottle filling machine from this program.
Initially this controller program is just a dummy to get you started.

The controller uses a simple non-preemptive multitasking.

Sections:
- \link Controller.ino Controller \endlink
- \link Commands Manual Commands \endlink
- \link I2C_Master I2C_Master \endlink

Arduinos for Plant (top) and controller (below).

![Plant and controller below](../../UNO2UNO_I2C+IO.png)

When using multiple I2C slaves note that
I2CSendRequest(...)
serves only one slave at a time.

<pre>
(c)2013-2020 Ingenieurbüro Dr. Friedrich Haase
             Consulting - Automatisierungstechnik
</pre>

<pre>
Versions
July 2014 Dr. Haase  created
Sept 2015 Dr. Haase  converted for IT project
Aug 2016  Dr. Haase  modified using direct IO
Sept 2017 Dr. Haase  temperature and some minor changes
Aug 2018  Dr. Haase  minor improvements
Jan 2020  Dr. Haase  redesigned for better C/C++ coding
Aug 2020  Dr. Haase  complete redesign
</pre>
*/

/*! \page Commands Manual Commands

Manual Commands to physical (PLC) outputs:
<table border="0" width="80%">
<tr><td> F=x </td><td> start/stop feed belt, port D2 </td></tr>
<tr><td> O=x </td><td> start/stop outlet belt, port D3 </td></tr>
<tr><td> P=x </td><td> move pusher cylinder out/in, port D4 </td></tr>
<tr><td> S=x </td><td> move stopper cylinder out/in, port D5 </td></tr>
<tr><td> L=x </td><td> move filler cylinder out/in, port D6 </td></tr>
<tr><td> C=x </td><td> move closing cylinder out/in, port D8 </td></tr>
<tr><td> H=x </td><td> switch heating on/off, port D9 </td></tr>
<tr><td> N=x </td><td> open/close nozzle valve, port D7 </td></tr>
<tr><td> c=x </td><td> switch cooling on/off, port A0 </td></tr>
</table>
with x either 1 or 0.

All other commands will be transmitted to the simulated plant over I²C.
<table border="0" width="80%">
<tr><td> M=x </td><td> set manual flag </td></tr>
<tr><td> V=x </td><td> set verbose flag </td></tr>
<tr><td> T?  </td><td> get simulation time </td></tr>
<tr><td> W?  </td><td> get warning flags </td></tr>
<tr><td> V?  </td><td> get verbose flag </td></tr>
<tr><td> t?  </td><td> get temperature </td></tr>
<tr><td> M?  </td><td> get manual flag </td></tr>
<tr><td> Z?  </td><td> get combined cylinder sensors </td></tr>
<tr><td> R   </td><td> (re)init </td></tr>
</table>
with x either 1 or 0.
*/

// include standard Arduino library
#include "Arduino.h"
// include standard I²C library
#include "Wire.h"
// include I²C connection
#include "I2C_Master.h"

                                                // time management
//! for 10 msec detection
unsigned long   msecPreviousMillis = 0;
//! counter for 10 msec up to 100 msec
int             nCount10msec = 0;
//! counter for 100 msec up to 1sec
int             nCount100msec = 0;

                                                // static const PLC IO output numbers
const int       nFeedBelt = 2;                  ///< feed belt drive
const int       nOutletBelt = 3;                ///< outlet belt drive
const int       nPusher = 4;                    ///< pusher cylinder
const int       nStopper = 5;                   ///< stopper cylinder
const int       nFiller = 6;                    ///< filler cylinder
const int       nNozzleValve = 7;               ///< nozzle valve
const int       nClosing = 8;                   ///< closing cylinder
const int       nHeating = 9;                   ///< heaters
const int       nCooling = 14/*A0*/;            ///< cooling

                                                // static const PLC IO input numbers
const int       nFeedBeltSensor = 10;           ///< feed belt bottle sensor
const int       nOutletBeltSensor = 11;         ///< feed outlet bottle sensor
const int       nBottleFullSensor = 12;         ///< bottle filled sensor

                                                // I2C
//! Address for I²C slave
const int       I2C_PLANT_ADDR = 10;
//! standard 100KHz for I²C
const long      I2C_FREQUENCY = 100000L;


//! Banner and version number
const char      szBanner[] = "# Fabrication Controller V4.04";
//! usual arduino pin 13 LED
const int       LEDpin = 13;


                                                // Bits of live data from plant over I²C bus
#define PUSHER_SENSOR_OUT       0x0001          ///< pusher reeled-out
#define PUSHER_SENSOR_HALF      0x0002          ///< pusher reeled-out half
#define PUSHER_SENSOR_IN        0x0004          ///< pusher retracted
#define STOPPER_SENSOR_OUT      0x0010          ///< stopper reeled-out
#define STOPPER_SENSOR_IN       0x0020          ///< stopper retracted
#define FILLER_SENSOR_OUT       0x0040          ///< filler reeled-out
#define FILLER_SENSOR_IN        0x0080          ///< filler retracted
#define CLOSING_SENSOR_OUT      0x0100          ///< closing reeled-out
#define CLOSING_SENSOR_IN       0x0200          ///< closing retracted

                                                // live data from plant over I²C bus
double          dTime = 0.0;                    ///< simulation time in sec
int             nZylinderSensors = 0;           ///< cylinder sensors
double          dTemperature = 0.0;             ///< temperature
int             nWarnings = 0;                  ///< warning bits

//! Toggle digital IO port
/*!
Toggle digital IO port.

\param nPin IO pin number
*/
void ToggleDigitalIOPort(int nPin)
{
  //// read, invert, write
  digitalWrite(nPin, ! digitalRead(nPin));      // read, invert, write
}

//! usual arduino init function
void setup()
{
  Serial.begin(115200);                         // set up serial port for 115200 Baud
#if defined (__AVR_ATmega32U4__)
  while ( ! Serial )                            // wait for serial port, ATmega32U4 chips only
    ;
#endif
  Serial.println(szBanner);                     // show banner and version

  pinMode(LEDpin, OUTPUT);                      // init arduino LED pin as an output

  // initialize IO, PLC outputs
  pinMode(nFeedBelt, OUTPUT);                   // feed belt drive
  pinMode(nOutletBelt, OUTPUT);                 // outlet belt drive
  pinMode(nPusher, OUTPUT);                     // pusher cylinder
  pinMode(nStopper, OUTPUT);                    // stopper cylinder
  pinMode(nFiller, OUTPUT);                     // filler cylinder
  pinMode(nNozzleValve, OUTPUT);                // nozzle valve
  pinMode(nClosing, OUTPUT);                    // closing cylinder
  pinMode(nHeating, OUTPUT);                    // heaters
  pinMode(nCooling, OUTPUT);                    // cooling

  // initialize IO, PLC inputs
  pinMode(nFeedBeltSensor, INPUT);              // feed belt bottle sensor
  pinMode(nOutletBeltSensor, INPUT);            // feed outlet bottle sensor
  pinMode(nBottleFullSensor, INPUT);            // bottle filled sensor

  msecPreviousMillis = millis();                // init global timing

  I2C_Master_Setup(I2C_FREQUENCY);              // start I²C master
}

//! reset initial IO positions
void ResetIO()
{
  digitalWrite(nFeedBelt, false);
  digitalWrite(nOutletBelt, false);
  digitalWrite(nPusher, false);
  digitalWrite(nStopper, false);
  digitalWrite(nFiller, false);
  digitalWrite(nNozzleValve, false);
  digitalWrite(nClosing, false);
  digitalWrite(nHeating, false);                // heater
  digitalWrite(nCooling, false);
}

//! Check if a command has been typed
/*!
Check if a command has been typed over COM-Port.

\param szCommand storage for a typed command
\param nCommandLengthMax size of szCommand
\returns true if a command has been typed
*/
bool CheckIfTypedAvailable(char szCommand[], int nCommandLengthMax)
{
  *szCommand = 0;                               // initially empty result
  if ( Serial.available() > 0 )
  {                                             // work on manual command
    int   ch;
    int   nCommandLen = 0;
    while ( ( ch = Serial.read() ) > 0 )
    {
      if ( nCommandLen < nCommandLengthMax )
      {
        szCommand[nCommandLen++] = ch;          // collect characters
        szCommand[nCommandLen] = 0;             // append a trailing zero byte
      }
    }
    return true;                                // command complete
  }
  else
    return false;
}

//! Create next steady transmitted command
/*!
Create next of steadily transmitted requests or commands to the plant.

Assume the buffer is large enough for all automatically created commands.

The state machine here may be expanded to support more commands.
Commands are created to cyclically transmit and/or request values to or from the plant.

Please note the state value nIndex has to be static to remain in existence after leaving the function.
The index is incremented in every call and reset to zero after all commands have been generated once.

\param szCommand storage for a typed command
\returns true if a command has been created
*/
bool CreateNextSteadyCommand(char szCommand[])
{
  *szCommand = 0;                               // initially empty

  static int   nIndex = 0;                      // index walking through requests
  switch ( ++nIndex )
  {
  case 1:                                       // request time
    strcpy(szCommand, "T?");                    // build command
    break;
  case 2:                                       // request cylinder sensors
    strcpy(szCommand, "Z?");                    // build command
    break;
  case 3:                                       // request temperature
    strcpy(szCommand, "t?");                    // build command
    break;
  case 4:                                       // request warnings
    strcpy(szCommand, "W?");                    // build command
    break;
  // more cases for other requests or settings
  default:
    nIndex = 0;                                 // start over
    break;
  }
  return ( *szCommand != 0 );                   // return true if command buffer not empty
}

//! Handle all commands which use digital IO
/*!
Handle all commands which use digital IO.

Several commands are not transmitted over I²C but set or reset digital outputs.

\param szCommand storage for a typed command
\returns true if command has been done
*/
bool WorkOnCommandsForDigitalIO(char szCommand[])
{
  if ( szCommand[1] == '=' )                    // test all assignments
  {
    if (   (   ( szCommand[2] == '0' )
            || ( szCommand[2] == '1' ) )
        && (   ( szCommand[3] == 0 )
            || ( szCommand[3] == '\r' )
            || ( szCommand[3] == '\n' ) ) )
    {
      bool  bValue = ( szCommand[2] == '1' );
      switch ( szCommand[0] )
      {
      case 'F':
        digitalWrite(nFeedBelt, bValue);
        return true;                            // done
      case 'O':
        digitalWrite(nOutletBelt, bValue);
        return true;                            // done
      case 'P':
        digitalWrite(nPusher, bValue);
        return true;                            // done
      case 'S':
        digitalWrite(nStopper, bValue);
        return true;                            // done
      case 'L':
        digitalWrite(nFiller, bValue);
        return true;                            // done
      case 'C':
        digitalWrite(nClosing, bValue);
        return true;                            // done
      case 'H':
        digitalWrite(nHeating, bValue);
        return true;                            // done
      case 'N':
        digitalWrite(nNozzleValve, bValue);
        return true;                            // done
      case 'c':
        digitalWrite(nCooling, bValue);
        return true;                            // done
      default:
        break;                                  // expect an I²C a command, see below
      }
    }
  }
  // else if ( szCommand[1] == '?' )            // value requests not yet implemented, not required so far

  return false;                                 // not done
}

//! Interpret an I²C response from the plant
/*!
Interpret an I²C response from the plant.

If more command types have been issued in function CreateNextSteadyCommand() the responses from the plant will return here.

\param szResponse storage for a typed command
\returns true if response has been used
*/
bool InterpreteResponse(char szResponse[])
{
  if ( szResponse[1] == '=' )
  {                                             // seems to be a valid response
    switch ( szResponse[0] )
    {
    case 'T':                                   // got a fresh dTime value
      dTime = atof(szResponse+2);               // convert response part after '=' to double
      return true;                              // done
    case 'Z':                                   // got fresh cylinder sensor values
      nZylinderSensors = atoi(szResponse+2);    // convert response part after '=' to integer
      return true;                              // done
    case 't':                                   // got a fresh dTemperature value
      dTemperature = atof(szResponse+2);        // convert response part after '=' to double
      return true;                              // done
    case 'W':                                   // got a fresh warning bits value
      nWarnings = atoi(szResponse+2);           // convert response part after '=' to integer
      return true;                              // done
    // more cases may follow
    }
  }
  return false;                                 // response not handled
}

//! Show some data values
/*!
Show some data values

Remember, each character at 115200 Baud requires about 0.1 msec.
*/
void ShowData()
{                                               // just to show a result
  Serial.print("T=");
  Serial.print(dTime);                          // time in sec
  Serial.print(" Z=0x");
  Serial.print(nZylinderSensors, HEX);
  Serial.print(" W=0x");
  Serial.print(nWarnings, HEX);
  Serial.println("");
}

//! Function Task_10ms called every 10 msec
void Task_10ms()
{
  ;							                                // nothing to do so far
}

//! Function Task_100ms called every 100 msec
/*!
I²C communication and keyboard input.
 */
void Task_100ms()
{
  static char  szCommand[I2C_DATA_MAX+1];       // buffer for commands
  static char  szResponse[I2C_DATA_MAX+1];      // buffer for responses
  static bool  bOperatesCommand = false;        // flag tells if request is under way

  if ( ! bOperatesCommand )                     // if not busy at working on a current command
  {
    if ( CheckIfTypedAvailable(szCommand, I2C_DATA_MAX+1) )
    {
      bOperatesCommand = true;                  // we have a new manual command to work on
    }
    else if ( CreateNextSteadyCommand(szCommand) )
    {
      bOperatesCommand = true;                  // we have a new generated command to work on
    }
  }

  // check for direct IO commands to physical ports
  if (   bOperatesCommand                       // working on a current command
      && ( szCommand[0] != 0 ) )                // and a new command is available
  {
    if ( szCommand[0] == 'R' )                  // check special case first
    {
      ResetIO();                                // the reset command
    }
    else if ( WorkOnCommandsForDigitalIO(szCommand) ) // check if command for digital IO
    {
      bOperatesCommand = false;                 // command has been done by IO assignment
    }
    // all remaining commands go to I²C, see below
  }

  if ( bOperatesCommand )                       // working on a current command
  {
    *szResponse = 0;                            // clean response
    int           nRes;
    nRes = I2C_SendRequest(I2C_PLANT_ADDR, szCommand); // request something from I²C slave
    if ( nRes == -2 )
      Serial.println("request too long");
    if ( *szCommand == 'R' )                    // handle special "reset" request
      bOperatesCommand = false;                 // no response expected after reset
  }

  long          nResponseTime;
  int           nSlaveNo;
  int           nResult = I2C_GetResponse(&nSlaveNo, szResponse, &nResponseTime);
  if ( nResult >= 0 )
  {
    if ( ! InterpreteResponse(szResponse) )     // use response we got
    {
      Serial.print(szCommand);                  // show not handled command and response
      Serial.print(" -> ");
      Serial.println(szResponse);
    }
    bOperatesCommand = false;                   // note command complete done
  }
  else if ( nResult == -1 )
    ;//Serial.println("busy");
  else if ( nResult == -3 )
  {
    Serial.print(nSlaveNo);
    Serial.println(":timeout");
    bOperatesCommand = false;                   // note command complete done
  }
  else if ( nResult == -4 )
    ;//Serial.println("no response yet");
}

//! Function Task_1s called every 1 sec
/*!
Use communication verbose flag (-v) to remove all but pure values.
This allows to use the integrated Arduino serial plotter or an external software like gnuplot.
*/
void Task_1s()
{
  ToggleDigitalIOPort(LEDpin);                  // toggle output to LED

  ShowData();                                   // possibly remove later
}

//! Usual arduino steadily called function
/*!
Usual arduino steadily called function.

This function will be called again and again as fast as possible.

It will dispatch the CPU power between tasks which are expected to be executed in some regular intervals.
Such intervals are often called sampling time.
*/
void loop()
{
  long  msecCurrentMillis = millis();
  if ( ( msecCurrentMillis - msecPreviousMillis ) < 10 )
    return;
  msecPreviousMillis = msecCurrentMillis;

  Task_10ms();                                  // call user 10 msec function
  if ( ++nCount10msec >= 10 )
  {
    nCount10msec = 0;
    Task_100ms();                               // call user 100 msec function
    if ( ++nCount100msec >= 10 )
    {
      nCount100msec = 0;
      Task_1s();                                // call user 1 sec function
    }
  }
 
  I2C_Master_Steady();                          // give background processing a chance
  delay(1);
}
