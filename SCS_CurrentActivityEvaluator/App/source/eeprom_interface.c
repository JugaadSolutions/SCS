
/*
*------------------------------------------------------------------------------
* eeprom_interface.c
*
* Module provides funcions to interface with the internal/external eeprom.
*
* (C)2010 Sam's Logic.
*
* The copyright notice above does not evidence any
* actual or intended publication of such source code.
*
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* File				: eeprom_interface.c
* Created by		: Sam
* Last changed by	: Sam
* Last changed		: 02/06/2010
*------------------------------------------------------------------------------
*
* Revision 1.1 07/07/2010 Sam
* First Release
* Revision 1.0 02/06/2010 Sam
* Initial revision
*
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include "board.h"
#include "i2c_driver.h"
#include "eeprom_interface.h"

/*
*------------------------------------------------------------------------------
* Private Defines
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Macros
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* Private Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Variables (static)
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* Public Constants
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Constants (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Function Prototypes (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Functions
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* void WriteByteEEP(UINT16 address, UINT8 data)
*
* Summary	: Write one byte data to internal/external EEPROM
*
* Input		: UINT16 address - memory loaction (address)
*			  UINT8 data - data byte to write
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
void WriteByteEEP(UINT16 address, UINT8 data)
{
	ENTER_CRITICAL_SECTION();
#ifdef EEP_EXTERRNAL
	StartI2C();
	if(address > 255)	// above byte address
	{
		WriteI2C(EEP_DEV_ADDR | EEP_NEXT_PAGE);
		WriteI2C((UINT8)address);
	}
	else
	{
		WriteI2C(EEP_DEV_ADDR);
		WriteI2C(address);
	}
	WriteI2C(data);
	StopI2C();
	DelayMs(5);
#else
	// Write single byte to Internal EEP
	Write_b_eep (address,data);
	// Checks & waits the status of ER bit in EECON1 register
	Busy_eep ();
#endif
	EXIT_CRITICAL_SECTION();
}

/*
*------------------------------------------------------------------------------
* UINT8 ReadByteEEP(UINT16 address)
*
* Summary	: Read one byte data from internal/external EEPROM
*
* Input		: UINT16 address - memory loaction (address)
*
* Output	: UINT8 data - data byte read
*
*------------------------------------------------------------------------------
*/
UINT8 ReadByteEEP(UINT16 address)
{
	UINT8 temp;
#ifdef EEP_EXTERRNAL
	ENTER_CRITICAL_SECTION();
	StartI2C();
	if(address > 255)	// above byte address
	{
		WriteI2C(EEP_DEV_ADDR | EEP_NEXT_PAGE);
		WriteI2C((UINT8)address);
	}
	else
	{
		WriteI2C(EEP_DEV_ADDR);
		WriteI2C(address);
	}
	StartI2C();
	if(address > 255)	// above byte address
	{
		WriteI2C(EEP_DEV_ADDR | EEP_NEXT_PAGE | EEP_READ);
	}
	else
	{
		WriteI2C(EEP_DEV_ADDR | EEP_READ);
	}
	temp = ReadI2C(NO_ACK);
	StopI2C();
	DelayMs(1);
	EXIT_CRITICAL_SECTION();
	return temp;
#else
	ENTER_CRITICAL_SECTION();
	//Read single byte from Internal EEP
	temp = Read_b_eep (address);
	EXIT_CRITICAL_SECTION();
	return temp;
#endif
}

/*
*------------------------------------------------------------------------------
* void WriteBytesEEP(UINT16 address, UINT8 *dataBuff,UINT8 numOfBytes)
*
* Summary	: Write multiple bytes of data to internal/external EEPROM
*
* Input		: UINT16 address - starting memory loaction (address)
*			  UINT8 *dataBuff - pointer to data bytes
*			  UINT8 numOfBytes - number of bytes to write (Max 256)
* Output	: None
*
*------------------------------------------------------------------------------
*/
void WriteBytesEEP(UINT16 address, UINT8 *dataBuff,UINT8 numOfBytes)
{
	UINT8 index;
	for(index = 0; index < numOfBytes; index++, dataBuff++)
	{
		WriteByteEEP((address + index),*dataBuff);
	}
}

/*
*------------------------------------------------------------------------------
* void ReadBytesEEP(UINT16 address,UINT8 *dataBuff,INT8 numOfBytes)
*
* Summary	: Read multiple bytes from internal/external EEPROM
*
* Input		: UINT16 address - starting memory loaction (address)
*			  UINT8 *dataBuff - pointer to data buffer
*			  UINT8 numOfBytes - number of bytes to read
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
void ReadBytesEEP(UINT16 address,UINT8 *dataBuff,INT8 numOfBytes)
{
	UINT8 index;
	for(index = 0; index < numOfBytes; index++, dataBuff++)
	{
		*dataBuff = ReadByteEEP(address + index);
	}
}
/*
*  End of eeprom_interface.c
*/