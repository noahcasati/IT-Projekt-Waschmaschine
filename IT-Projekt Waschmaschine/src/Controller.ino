/*! \mainpage Controller

//Test123

Simple washing machine controller.

The goal is to control the washing machine from this program.
Initially this controller program is just a dummy to get you started.

The controller uses a simple non-preemptive multitasking.

Sections:
- \link Controller.ino Controller \endlink
- \link Commands Manual Commands \endlink
- \link I2C_Master I2C_Master \endlink

Arduinos for Plant (top) and controller (below).

![Plant and controller below](../../UNO2UNO_I2C+IO.png)

When using multiple I2C slaves note that
Master.SendRequest(...)
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
Sept 2017 Dr. Haase  minor improvements
Aug 2018  Dr. Haase  door sensor reversed
Jan 2020  Dr. Haase  redesigned for better C/C++ coding
May 2020  Dr. Haase  manual mode removed, drum speed only over I2C
Aug 2020  Dr. Haase  complete redesign
</pre>
*/

/*! \page Commands Manual Commands

The washing machine running on an Arduino has following manual commands using the USB COM port:

<table border="0" width="80%">
<tr><td> t=x </td><td> time lapse mode on/off </td></tr>
<tr><td> D=x </td><td> door open/closed </td></tr>
<tr><td> L=n </td><td> laundry amount in kg (integer) </td></tr>
<tr><td> I=x </td><td> water valve open closed </td></tr>
<tr><td> H=x </td><td> heating on/off </td></tr>
<tr><td> P=x </td><td> pump on/off </td></tr>
<tr><td> O=n </td><td> washing detergent (integer) </td></tr>
<tr><td> o=n </td><td> softener (integer) </td></tr>
<tr><td> r=n </td><td> drum rotation speed </td></tr>
<tr><td> T? </td><td> get simulation time </td></tr>
<tr><td> t? </td><td> time lapse mode </td></tr>
<tr><td> C? </td><td> temperature </td></tr>
<tr><td> D? </td><td> door state </td></tr>
<tr><td> L? </td><td> laundry amount </td></tr>
<tr><td> I? </td><td> water valve state </td></tr>
<tr><td> A? </td><td> water level in kg </td></tr>
<tr><td> H? </td><td> heating </td></tr>
<tr><td> w? </td><td> amount of water in laundry </td></tr>
<tr><td> k? </td><td> consumed heating energy </td></tr>
<tr><td> P? </td><td> water pump state </td></tr>
<tr><td> o? </td><td> remaining amount of detergent or softener</td></tr>
<tr><td> r? </td><td> drum rotation speed </td></tr>
<tr><td> W? </td><td> warning </td></tr>
<tr><td> V=x </td><td> verbose on/off </td></tr>
<tr><td> R </td><td> (re)init </td></tr>
</table>
with x either 1 or 0.

All other commands will be transmitted to the simulated plant over I²C.
<table border="0" width="80%">
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
// include Metro header for "Multitasking"
#include "Metro.h"
//include SoftwareSerial for additional serial communication
#include "SoftwareSerial.h"

                                                // time management
//! for 10 msec detection
unsigned long   msecPreviousMillis = 0;
//! counter for 10 msec up to 100 msec
int             nCount10msec = 0;
//! counter for 100 msec up to 1sec
int             nCount100msec = 0;
// enable waschprogramm
bool          isRunning = true;               


                                                // I2C
//! Address for I²C slave
const int       I2C_PLANT_ADDR = 10;
//! standard 100KHz for I²C
const long      I2C_FREQUENCY = 100000L;

//SoftwareSerial init
SoftwareSerial esp_uno (10 , 11); // RX, TX

char rMessage;


                                                // static const PLC IO output numbers
const int       nWaterIntake = 2;               ///< water intake valve
const int       nWaterPump = 3;                 ///< water pump
const int       nHeating = 4;                   ///< heating
const int       doorSwitch = 9;                 ///< pin doorswitch
bool            bDoorClose = 0;                 ///< Türvariable


                                                // static const PLC IO input numbers
const int       nDoorClosed = 8;                ///< door closed sensor (Input)

                                                // live data from plant over I²C bus
double          dTime = 0.0;                    ///< simulation time in min
double          dTemperature = 0.0;             ///< temperature
double          dWaterLevel = 0.0;              ///< water level
int             nWarnings = 0;                  ///< warning bits
int          drpm = 0;                     ///< rpm
int          dWaeschemenge = 0;            ///< Wäschemenge
int          dWaschmittelmenge = 0;        ///< Waschmittelmange

// Timing Variablen fürs "Multitasking" (Metro)
Metro hundert = Metro(100);
Metro fuenfhundert = Metro(500);
Metro sekunde = Metro(1000);

Metro temp100 = Metro(100);
Metro akt_data = Metro(100);
Metro send_data_plant = Metro(100);
Metro send_data_esp = Metro(500);
Metro receive_data_esp = Metro(500);
Metro display_data = Metro(1000);
Metro wp1_duration = Metro(60000);


//! Banner and version number
const char     szBanner[] = "# Washing Machine Controller V3.04";
//! usual Arduino pin 13 LED
const int       LEDpin = 13;

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
  pinMode(LEDpin, OUTPUT);                      // init arduino LED pin as an output

  Serial.begin(115200);                         // set up serial port for 115200 Baud
  esp_uno.begin(9600);                          // set up SoftwareSerial port 9600 Baud

#if defined (__AVR_ATmega32U4__)
  while (! esp_uno)
  {
    ;
  }
#endif

#if defined (__AVR_ATmega32U4__)
  while ( ! Serial )                            // wait for serial port, ATmega32U4 chips only
    ;
#endif
  Serial.println(szBanner);                     // show banner and version

  pinMode(10, INPUT);
  pinMode(11, OUTPUT);

  // initialize IO, PLC outputs
  pinMode(nWaterIntake, OUTPUT);                // water intake valve
  pinMode(nWaterPump, OUTPUT);                  // water pump
  pinMode(nHeating, OUTPUT);                    // heaters

  // initialize IO, PLC inputs
  pinMode(nDoorClosed, INPUT);                  // door closed sensor
  pinMode(doorSwitch, INPUT_PULLUP);            // door mechanism

  msecPreviousMillis = millis();                // init global timing

  I2C_Master_Setup(I2C_FREQUENCY);              // start I²C master
}

//! reset initial IO positions
void ResetIO()
{
  digitalWrite(nWaterIntake, false);            // water intake valve
  digitalWrite(nWaterPump, false);              // water pump
  digitalWrite(nHeating, false);                // heater
  digitalWrite(nDoorClosed, false);             // Door
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
  case 2:                                       // request temperature
    strcpy(szCommand, "C?");                    // build command
    break;
  case 3:                                       // request water level
    strcpy(szCommand, "A?");                    // build command
    break;
  case 4:                                       // request warnings
    strcpy(szCommand, "W?");                    // build command
    break;
  case 5:                                       // request door state
    if(bDoorClose == true)
      strcpy(szCommand, "D=1");                    // Tür zu
    else
      strcpy(szCommand, "D=0");                // Tür auf
    break;
  case 6:
      strcpy(szCommand, "r?");                    // rpm Abfrage
  case 7:
    strcpy(szCommand, "L?");
    break;
  case 8:
    strcpy(szCommand, "o?");
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
      case 'I':
        digitalWrite(nWaterIntake, bValue);
        return true;                            // done
      case 'P':
        digitalWrite(nWaterPump, bValue);
        return true;                            // done
      case 'H':
        digitalWrite(nHeating, bValue);
        return true;                            // done
      //case'D':
         //digitalWrite(nDoorOpen, bValue);
         //return true;
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
    case 'C':                                   // got a fresh dTemperature value
      dTemperature = atof(szResponse+2);        // convert response part after '=' to double
      return true;                              // done
    case 'A':                                   // got a fresh dWaterLevel value
      dWaterLevel = atof(szResponse+2);         // convert response part after '=' to double
      return true;                              // done
    case 'W':                                   // got a fresh warning bits value
      nWarnings = atoi(szResponse+2);           // convert response part after '=' to integer
      return true;                              // done
    ///*!
    case 'D':
      bDoorClose = atoi(szResponse+2);
      return true;
    //*/
    case 'r':
      drpm = atoi(szResponse+2);              // convert response part after '=' to integer
      return true;
    case 'L':
      dWaeschemenge = atof(szResponse+2);
      return true;
    case 'o':
      dWaschmittelmenge = atof(szResponse+2);
      return true;
    // more cases may follow
    }
  }
  return false;                                 // response not handled
}

//Sets Door status based on closed circuit on Pin 

void door()
{
  if(digitalRead(doorSwitch) == false)
  {
    //Serial.print("funktionirt bis hier hin!");
    digitalWrite(nDoorClosed, true);             // Door
  } 
  else
  {
    //Serial.print("Läuft nícht!");
    //WorkOnCommandsForDigitalIO(D=0)  
    //Serial.println("Ist offen");
    digitalWrite(nDoorClosed, false);             // Door
    }
}

void charArrOut(char arr[])
{

}

int handleTemp(double tTemperature)
{

  bool i = true;

  while(i == true) 
  {

  if(dTemperature < tTemperature)
  {
      WorkOnCommandsForDigitalIO("H=1");
      Task_100ms();
      I2C_Master_Steady();                          // give background processing a chance
      Serial.println("Heizung An");
      Serial.println(" C=");
      Serial.print(dTemperature);
  }

  else if(dTemperature >= tTemperature && dTemperature < tTemperature + 2)
  {
    WorkOnCommandsForDigitalIO("H=0");
    Task_100ms();
    I2C_Master_Steady();                          // give background processing a chance
    Serial.println("Temperatur ist im Rahmen");
    Serial.print(" C=");
    Serial.print(dTemperature);
    i = false;
  }

  else 
  {
      WorkOnCommandsForDigitalIO("H=0");
      Task_100ms();
      I2C_Master_Steady();                          // give background processing a chance
      Serial.println("Heizung aus");
      Serial.println(" C=");
      Serial.print(dTemperature);
  }
      Serial.println("Temp Schleife läuft noch!!!");
  }
  return 0;
}

int handleWater(double tWaterlevel)
{
  if(digitalRead(nDoorClosed) == true)
  {
    while(tWaterlevel > dWaterLevel) 
    {
      WorkOnCommandsForDigitalIO("I=1");
      Task_100ms();
      I2C_Master_Steady();                          // give background processing a chance
      Serial.println("Ventil Auf");
      Serial.println(" A=");
      Serial.print(dWaterLevel);
    }
    WorkOnCommandsForDigitalIO("I=0");
    Task_100ms();
    I2C_Master_Steady();                          // give background processing a chance
    Serial.println("Ventil Zu");
  }
}

void waschprogramm1()
{
  int     O = 3;                    // Maschpulvermenge
  int     o = 2;                    // Weichspüler
  int     rpm = 800;                // Trommel rpm
  double  targetTemperature = 60;   
  double  targetWaterLevel = 1.3;
  int     maxWaescheMenge = 6;
  bool    isFilled = false;

  

  //strcpy(szCommand, "I=1");
  while(isRunning == true && bDoorClose == true)
  {
    if(isFilled == false)
    {
      handleWater(targetWaterLevel);
      Serial.println("Handle Water wurde ausgeführt");
      isFilled = true;
    }

    // Wassertemperatur einstellen
    if(temp100.check())
    {
      handleTemp(targetTemperature);
      Serial.println("Handle Temp wurde ausgeführt");
    }

    // Daten aktualisieren
    if(akt_data.check())
    {
      Task_100ms();
      Serial.println("100ms Task wurde ausgeführt!");
    }

    if(display_data.check())
    {
      ShowData();
    }

    if(hundert.check())
    {
      door();
      Serial.println("Tür-Sensor aktualisiert");
    }

    if(wp1_duration.check())
    {
      isRunning = false;
      Serial.println("isRunning ist auf false gesetzt worden!!! (WP1 beendet!)");
    }
  }

}

//! Show some data values
/*!
Show some data values

Remember, each character at 115200 Baud requires about 0.1 msec.
*/
void ShowData()
{                                               // just to show a result
  Serial.print("T=");
  Serial.print(dTime);                          // time in min
  Serial.print(" C=");
  Serial.print(dTemperature);
  Serial.print(" A=");
  Serial.print(dWaterLevel);
  Serial.print(" D=");
  Serial.print(digitalRead(nDoorClosed));
  Serial.print(" RPM=");
  Serial.print(drpm);
  Serial.print(" L=");
  Serial.print(dWaeschemenge);
  Serial.print(" O=");
  Serial.print(dWaschmittelmenge);
  Serial.print(" W=0x");
  Serial.print(nWarnings, HEX);
  Serial.println("");
}

//! Function Task_10ms called every 10 msec
void Task_10ms()
{
  ;                                              // nothing to do so far
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
  if ( bOperatesCommand )                       // working on a current command
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
#if 1                                           // possibly disable
      Serial.print(" -> ");                     // show not handled command and response
      Serial.println(szResponse);
#endif
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
    
   //Serial.print("test test 123");
}

//! Function Task_1s called every 1 sec
/*!
Use communication verbose flag (-v) to remove all but pure values.
This allows to use the integrated Arduino serial plotter or an external software like gnuplot.
*/
void Task_1s()
{
 // zeit ++;                                  // Timer

  ToggleDigitalIOPort(LEDpin);                  // toggle output to LED

  ShowData();                                   // possibly remove later

   //door();                                      // Türzustand senden



}

/*!
==========================================================================================================================
==========================================================================================================================
==========================================================================================================================
==========================================================================================================================
*/
//! Usual arduino steadily called function
/*!
Usual arduino steadily called function.

This function will be called again and again as fast as possible.

It will dispatch the CPU power between tasks which are expected to be executed in some regular intervals.
Such intervals are often called sampling time.
*/
void loop()
{
  static char   szCommand[I2C_DATA_MAX+1];       // buffer for commands

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
  waschprogramm1();
/*!
  if(sekunde.check())
  {
  esp_uno.write("test test 123");               // send to esp via SoftwareSerial
  }

  if(Serial.available())
  {

    rMessage = Serial.read();
    esp_uno.print(rMessage);
    Serial.println(rMessage);

  }
  */

  I2C_Master_Steady();                          // give background processing a chance
  //delay(1);
}