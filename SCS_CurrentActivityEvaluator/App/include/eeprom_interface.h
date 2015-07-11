
#ifndef EEPROM_INTERFACE_H
#define EEPROM_INTERFACE_H


/*
*------------------------------------------------------------------------------
* eeprom_interface.h
*
* Include file for eeprom_interface module.
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
* File				: eeprom_interface.h
* Created by		: Sam
* Last changed by	: Sam
* Last changed		: 19/07/2010
*------------------------------------------------------------------------------
*
* Revision 1.2 19/07/2010 Sam
* allocated location in eeprom for "EEP_PREV_SHIFT_TYPE"
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

#include "typedefs.h"

/*
*------------------------------------------------------------------------------
* Public Defines
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Macros
*------------------------------------------------------------------------------
*/


// EEPROM address and commands
#define 	EEP_DEV_ADDR				(0xA0)
#define 	EEP_READ					(0x01)		// OR with device addres for read.
#define 	EEP_NEXT_PAGE				(0x02)		// for address above 255 bytes.



/*
*------------------------------------------------------------------------------
* Public Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables (extern)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Constants (extern)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Function Prototypes (extern)
*------------------------------------------------------------------------------
*/

extern void WriteByteEEP(UINT16 address, UINT8 data);
extern void WriteBytesEEP(UINT16 address, UINT8 *dataBuff,UINT8 numOfBytes);
extern UINT8 ReadByteEEP(UINT16 address);
extern void ReadBytesEEP(UINT16 address,UINT8 *dataBuff,INT8 numOfBytes);

#endif
/*
*  End of eeprom_interface.h
*/



