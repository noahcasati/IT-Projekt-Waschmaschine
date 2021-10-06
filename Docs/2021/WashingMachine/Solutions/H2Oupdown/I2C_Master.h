/*! \page I2C_Master I2C Master
Arduino I²C Master.

<pre>
(c)2015-2020 Ingenieurbüro Dr. Friedrich Haase
             Consulting - Automatisierungstechnik
</pre>

<pre>
<b>Versions</b>
July 2015  Dr. Haase  created
Sept 2015  Dr. Haase  converted for IT project
Aug 2020   Dr. F. Haase  complete redesign
</pre>
*/

// include standard Arduino library
#include <Arduino.h>

                                                // I²C attributes
//! max request and response length
const int       I2C_DATA_MAX = 32;

                                                // I²C prototypes
//! I²C master setup
/*!
I²C master setup
\param nFrequency I2C frequency
\param nSDA SDA pin
\param nSCL SCL pin
*/
extern void I2C_Master_Setup(long nFrequency, int nSDA, int nSCL);

//! I²C master steady call
/*!
I²C master steady call
*/
extern void I2C_Master_Steady();

//! Send a binary message to a slave
/*!
Send a binary message to a slave.
Does not expect a response.
\param nSlaveNo target slave number for the message
\param Data binary message to send
\param nDataLength message length
\return >=0 on success, -1 if busy, -2 if too long
*/
extern int I2C_SendMessage(int nSlaveNo, const uint8_t * pData, int nDataLength);

//! Send a text message to a slave
/*!
Send a text message to a slave.
Does not expect a response.
\param nSlaveNo target slave number for the message
\param pszMessage text message to send
\return >=0 on success, -1 if busy, -2 if too long
*/
extern int I2C_SendMessage(int nSlaveNo, const char * const pszMessage);

//! Send request to slave
/*!
Send request to slave.
Will not necesarily get a response.
\param nSlaveNo target slave number for the request
\param pszRequest request text
\return true on success
*/
extern bool I2C_SendRequest(int nSlaveNo, const char * const pszRequest);

//! Get binary response for last request
/*!
Gets a received binary response.
\param pnSlaveNo slave number the request was targeted
\param puiResponse storage for received response
\returns >=0 length on success, -1 if busy, -2 if timed out, -3 if no outstanding response
*/
int I2C_GetResponse(int * pnSlaveNo, uint8_t * puiResponse);

//! Get text response for last request
/*!
Gets a received text response.
\param pnSlaveNo slave number the request was targeted
\param pszResponse storage for received response
\returns >=0 length on success, -1 if busy, -2 if timed out, -3 if no outstanding response
*/
extern int I2C_GetResponse(int * pnSlaveNo, char * pszResponse);

//! Get time/duration from last request to last response in microseconds
/*!
Get time/duration from last request to last response in microseconds.
*/
extern unsigned long I2C_GetResponseTime();

//! See if I2C is ready for a request
/*!
See if I2C is ready for a request
\return true if I2C is ready
*/
bool I2C_IsReady();
