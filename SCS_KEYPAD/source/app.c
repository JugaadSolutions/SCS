
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/
#include "config.h"
#include "board.h"
#include "timer.h"
#include "modbusMaster.h"
#include "app.h"
#include "keypad.h"
#include "lcd.h"
#include "string.h"
#include "eep.h"
#include "ui.h"


//#define SIMULATION
#define STARTING_ADDRESS 0

/*
*------------------------------------------------------------------------------
* Modbus master related data
*------------------------------------------------------------------------------
*/

//////////////////// Port information ///////////////////
#define BAUD_RATE 	9600
#define TIMEOUT	 	1000
#define POLLING		200 // the scan rate
#define RETRY_COUNT 10

// The total amount of available memory on the master to store data
#define TOTAL_NO_OF_REGISTERS 1

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  PACKET1,
  PACKET2,
  TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS];

// Masters register array
unsigned int regs[TOTAL_NO_OF_REGISTERS];
/*
*------------------------------------------------------------------------------
* Structures
*------------------------------------------------------------------------------
*/
typedef struct _APP
{
	UINT8 count;

}APP;

/*
*------------------------------------------------------------------------------
* Variables
*------------------------------------------------------------------------------
*/


/*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

void login(void);


/*
*------------------------------------------------------------------------------
* void APP-init(void)
*------------------------------------------------------------------------------
*/

void APP_init(void)
{


 // Initialize the Modbus Finite State Machine
  modbus_configure(BAUD_RATE, TIMEOUT, POLLING, RETRY_COUNT, packets, TOTAL_NO_OF_PACKETS, regs);
}


void login()
{
	
}

BOOL APP_login(far UINT8 *password,far UINT8 *data)
{
	

	return TRUE;
}


BOOL APP_logout(far UINT8 *password,far UINT8 *data)
{
	UINT8 grn =0 , org = 0, red = 0, buz = 0; 
	UINT8 i;
	//if( strcmp(ias.logonPassword , password) )
		return FALSE;

	return TRUE;
}




/*
*------------------------------------------------------------------------------
* void APP-task(void)
*------------------------------------------------------------------------------
*/
void APP_task(void)
{
	UINT8 i,*ptr, data;

	UINT32 addr;
	UINT8 resetBuzzer = TRUE;

}

/*
*------------------------------------------------------------------------------
* void APP_TxDataOnModubs(void)
*------------------------------------------------------------------------------
*/

void APP_TxDataOnModubs(UINT16 id, UINT8 state, UINT8 command)
{
	UINT8 buffer[4] = {0};
	
	buffer[0] = command;
	buffer[1] = (UINT8)id >> 8;
	buffer[2] = (UINT8)id & 0xFF;
	buffer[3] = state;
	
	// Initialize each packet
  	modbus_construct(&packets[PACKET1], 1, PRESET_MULTIPLE_REGISTERS, STARTING_ADDRESS, 2, buffer);
}
	
		

		
	

