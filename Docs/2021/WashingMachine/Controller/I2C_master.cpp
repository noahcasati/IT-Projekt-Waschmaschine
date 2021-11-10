
/* I²C Master
(c)2014..2020 Ingenieurbüro Dr. Friedrich Haase
*/

// include standard Arduino library
#include <Arduino.h>
// include standard I²C library
#include <Wire.h>
// include packer library
#include <WirePacker.h>
// include slave request header
#include <WireSlaveRequest.h>
// include I²C master
#include "I2C_Master.h"


                                                // I²C communication data
//! last used slave
int             nSlaveNoLast = 0;
//! response storage
uint8_t         uiResponse[I2C_DATA_MAX+1];
//! response storage length
int             nResponseLength = 0;
//! last request time in usec
unsigned long   nRequestTime = 0;
//! response timeout in usec
unsigned long   nResponseTimeOut = 100000;
//! last response time/duration in usec
unsigned long   nResponseTime = 0;
//! state machine states
enum I2C_State { I2C_READY, I2C_BUSY, I2C_DONE, I2C_TIMEOUT };
//! current I2C state
I2C_State       nI2CState = I2C_READY;

// I²C master setup
void I2C_Master_Setup(long nFrequency, int nSDA, int nSCL)
{
  nI2CState = I2C_READY;
  Wire.setClock(nFrequency);
  Wire.begin(nSDA, nSCL);                       // start I²C bus as master
}

// I²C master steady call
void I2C_Master_Steady()
{
  switch ( nI2CState )
  {
  case I2C_READY:
    break;
  case I2C_BUSY:
    {
      WireSlaveRequest  SlaveReq(Wire, nSlaveNoLast, I2C_DATA_MAX);
      SlaveReq.setRetryDelay(5/*msec*/);
      if ( SlaveReq.request() )
      {
        int   nLength = 0;
        while ( SlaveReq.available() )          // loop through all
        {
          uiResponse[nLength++] = SlaveReq.read(); // receive byte
        }
        uiResponse[nLength] = 0;                // append a trailing 0, just in case
        nResponseLength = nLength;
        nResponseTime = micros() - nRequestTime;
        nI2CState = I2C_DONE;
      }
      else if (  ( nI2CState == I2C_BUSY ) 
              && ( ( micros() - nRequestTime ) >= nResponseTimeOut ) )
      {                                         // timeout
        nResponseLength = -2;
        nI2CState = I2C_TIMEOUT;
      }
    }
    break;
  case I2C_DONE:
    break;
  case I2C_TIMEOUT:
    break;
  }
}

// Send a binary message to a slave
int I2C_SendMessage(int nSlaveNo, const uint8_t * pData, int nDataLength)
{
  if ( nI2CState != I2C_READY )
    return -1;                                  // fail if busy or still keeps some data
  if ( nDataLength > I2C_DATA_MAX )
    return -2;                                  // fail, too long

  nSlaveNoLast = nSlaveNo;
  nI2CState = I2C_BUSY;                         // busy now

  WirePacker  packer;
  for ( int i=0; i<nDataLength; ++i )
    packer.write(pData[i]);
  packer.end();
  Wire.beginTransmission(nSlaveNo);             // transmit to slave device
  while ( packer.available() )                  // send all data
    Wire.write(packer.read());
  Wire.endTransmission();                       // ends transmitting

  return nDataLength;                           // request done
}

// Send a text message to a slave
int I2C_SendMessage(int nSlaveNo, const char * const pszMessage)
{
  int  nLen = strlen(pszMessage);               // message length
  nLen = I2C_SendMessage(nSlaveNo, (uint8_t*)pszMessage, nLen+1); // send string including trailing 0
  if ( nLen > 0 )
    --nLen;
  return  nLen;                                 // request done
}

// Send request to slave and receive response
bool I2C_SendRequest(int nSlaveNo, const char * const pszRequest)
{
  int   res = I2C_SendMessage(nSlaveNo, pszRequest);
  if ( res < 0 )
    return false;                               // send request failed
  nRequestTime = micros();
  return true;
}

// Get binary response for last request
int I2C_GetResponse(int * pnSlaveNo, uint8_t * puiResponse)
{
  if ( nI2CState == I2C_BUSY )
    return -1;                                  // fail if busy
  if ( nI2CState == I2C_READY )
    return -3;                                  // fail if no outstanding response
  if ( nI2CState == I2C_TIMEOUT )
  {                                             // fail if timed out
    *puiResponse = 0;
    *pnSlaveNo = nSlaveNoLast;
    nI2CState = I2C_READY;
    return -2;
  }

  memcpy(puiResponse, uiResponse, nResponseLength); // return received answer
  *pnSlaveNo = nSlaveNoLast;
  nI2CState = I2C_READY;
  return nResponseLength;
}

// Get text response for last request
int I2C_GetResponse(int * pnSlaveNo, char * pszResponse)
{
  int   nLength = I2C_GetResponse(pnSlaveNo, (uint8_t*)pszResponse);
  if ( nLength < 0 )
    return nLength;                             // failed
//pszResponse[nLength] = 0;                     // make sure there is a trailing 0, done in I2C_Master_Steady()
  return strlen(pszResponse);                   // possibly nLength - 1
}

// Get time/duration from last request to last response in microseconds
unsigned long I2C_GetResponseTime()
{
  return nResponseTime;
}

// See if I2C is ready for a request
bool I2C_IsReady()
{
  return nI2CState == I2C_READY;
}

// not used so far, possibly not required
#if 0
//! See if request in progress
/*!
See if request in progress.
\return true if I2C is busy on a previous command
*/
bool I2C_IsBusy()
{
  return nI2CState == I2C_BUSY;
}

//! See if request has been fulfilled
/*!
See if request has been fulfilled.
\return true if I2C previous command got a reply
*/
bool I2C_HasReply()
{
  return nI2CState == I2C_DONE;
}

//! See if request timed out
/*!
See if request timed out.
\return true if I2C previous command got no reply in due time
*/
bool I2C_HasTimeout()
{
  return nI2CState == I2C_TIMEOUT;
}
#endif
