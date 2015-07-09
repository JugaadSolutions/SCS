
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

/*
*------------------------------------------------------------------------------
* modbus 
*------------------------------------------------------------------------------
*/

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  PACKET1,
  //PACKET2,
  TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS];

// Masters register array
unsigned int regs[TOTAL_NO_OF_REGISTERS];

MBErrorCode PACKET_SENT = MB_TXDONE;
MBErrorCode RETRIES_DONE = MB_TIMEDOUT;

/*
*------------------------------------------------------------------------------
* Structures
*------------------------------------------------------------------------------
*/
																			//Object to store data of raised issue

typedef struct _LOG
{
	UINT8 prevIndex;
	UINT8 writeIndex;
	UINT8 readIndex;
	UINT16 entries[MAX_LOG_ENTRIES][LOG_BUFF_SIZE];
}LOG;																			//Object to store log entries

typedef struct _APP
{
	UINT8 password[5];
	UINT8 logonPassword[5];
	UINT8 truckState[MAX_NO_OF_TRUCKS][MAX_STATES];	//used to store the state of each truck
	UINT8 regCount[MAX_LOG_ENTRIES];     // Buffer used to hold the number of 16bits counts in data pack
	UINT8 cancelTruck[MAX_NO_OF_TRUCKS]; // Buffer used to hold the truck status
}APP;																			//This object contains all the varibles used in this application




/*
*------------------------------------------------------------------------------
* Variables
*------------------------------------------------------------------------------
*/
#pragma idata APP_DATA
APP app = {0};
#pragma idata

#pragma idata LOG_DATA
LOG log = {0};
#pragma idata

UINT8 buffer[] = "01234567";

static rom UINT16 timeout = (UINT16)150;

/*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/




/*
*------------------------------------------------------------------------------
* void APP-init(void)
*------------------------------------------------------------------------------
*/

void APP_init(void)
{

	UINT16 i;
	UINT8 regCount;
	UINT8 ackStatus = 0;

	app.password[0] = '1';
	app.password[1] = '0';
	app.password[2] = '0';
	app.password[3] = '3';
	app.password[4] = '\0';

	regCount = strlen(buffer);

//	MB_construct(&packets[PACKET1], SLAVE_ID, PRESET_MULTIPLE_REGISTERS, 
//							STARTING_ADDRESS, regCount/2, buffer);	
		
	//modbus master initialization
	MB_init(BAUD_RATE, TIMEOUT, POLLING, RETRY_COUNT, packets, TOTAL_NO_OF_PACKETS, regs);

}




/*
*------------------------------------------------------------------------------
* void APP-task(void)
*------------------------------------------------------------------------------
*/
void APP_task(void)
{
	MBErrorCode status;
	UINT8 regCount;

	status = MB_getStatus();

	if( (status == PACKET_SENT) || (status == RETRIES_DONE) )
	{
		
		//check for log entry, if yes write it to modbus			
		if(log.readIndex != log.writeIndex)
		{			
			MB_construct(&packets[PACKET1], SLAVE_ID, PRESET_MULTIPLE_REGISTERS, 
								STARTING_ADDRESS, app.regCount[log.readIndex], log.entries[log.readIndex]);	

			log.readIndex++;
		
			// check for the overflow
			if( log.readIndex >= MAX_LOG_ENTRIES )
				log.readIndex = 0;
								
	
		}
	}
	else
		return;



}



/*---------------------------------------------------------------------------------------------------------------
*	void updateLog(void)
*----------------------------------------------------------------------------------------------------------------
*/
void App_updateLog(far UINT8 *data)
{
	UINT8 i = 0;
	while( *data != '\0')
	{
		log.entries[log.writeIndex][i] = (UINT16)*data << 8;
		data++;
		log.entries[log.writeIndex][i] |= (UINT16)*data;
		data++;
		i++;
	}
	log.entries[log.writeIndex][i]= '\0';
	app.regCount[log.writeIndex] = i;
	log.writeIndex++;
	if( log.writeIndex >= MAX_LOG_ENTRIES)
		log.writeIndex = 0;
}

	

BOOL APP_checkPassword(UINT8 *password)
{

	if( strcmp(app.password , password) )
		return FALSE;
	return TRUE;
}

/*---------------------------------------------------------------------------------------------------------------
*	BOOL APP_activityValid(UINT8 *buffer)
*----------------------------------------------------------------------------------------------------------------
*/


BOOL APP_activityValid(UINT8 *buffer)
{
	UINT8 result = FALSE;
	
	UINT8 truckNo = atoi(buffer);

	if(app.cancelTruck[truckNo] == CANCELLED)
		return result;

	if(truckNo <= MAX_NO_OF_TRUCKS && truckNo > 0)
		result = TRUE;

	return result;
}

/*---------------------------------------------------------------------------------------------------------------
*	UINT8 APP_managePicking(UINT8 *buffer)
*----------------------------------------------------------------------------------------------------------------
*/
void APP_managePicking(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	//shift truck number to store CMD
	buffer[3] = '\0';
	buffer[2] = buffer[1];
	buffer[1] = buffer[0];

	if(app.truckState[truckNo][PICKING_START] == TRUE)
	{
		app.truckState[truckNo][PICKING_END] = TRUE;
		buffer[0] = CMD_PICKING_END;
	}
	else
	{
		app.truckState[truckNo][PICKING_START] = TRUE;
		buffer[0] = CMD_PICKING_START;
	}
}

UINT8 APP_validatePicking(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	if	(app.truckState[truckNo][PICKING_END] == TRUE)
		return FALSE;

	if	(app.truckState[truckNo][PICKING_START] == TRUE)
		return CMD_PICKING_END;
	else
		return CMD_PICKING_START;
}

/*---------------------------------------------------------------------------------------------------------------
*	UINT8 APP_manageStaging(UINT8 *buffer)
*----------------------------------------------------------------------------------------------------------------
*/

void APP_manageStaging(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	//shift truck number to store CMD
	buffer[3] = '\0';
	buffer[2] = buffer[1];
	buffer[1] = buffer[0];

	if(app.truckState[truckNo][STAGING_START] == TRUE)
	{
		app.truckState[truckNo][STAGING_END] = TRUE;
		buffer[0] = CMD_STAGING_END;
	}
	else
	{
		app.truckState[truckNo][STAGING_START] = TRUE;
		buffer[0] = CMD_STAGING_START;
	}
}

UINT8 APP_validateStaging(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	if	(app.truckState[truckNo][STAGING_END] == TRUE)
		return FALSE;

	if	(app.truckState[truckNo][STAGING_START] == TRUE)
		return CMD_STAGING_END;
	else
		return CMD_STAGING_START;
}


/*---------------------------------------------------------------------------------------------------------------
*	UINT8 APP_manageLoading(UINT8 *buffer)
*----------------------------------------------------------------------------------------------------------------
*/

void APP_manageLoading(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	//shift truck number to store CMD
	buffer[3] = '\0';
	buffer[2] = buffer[1];
	buffer[1] = buffer[0];

	if(app.truckState[truckNo][LOADING_START] == TRUE)
	{
		app.truckState[truckNo][LOADING_END] = TRUE;
		buffer[0] = CMD_LOADING_END;
		
	}
	else
	{
		app.truckState[truckNo][LOADING_START] = TRUE;
		buffer[0] = CMD_LOADING_START;
	}
}

UINT8 APP_validateLoading(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	if	(app.truckState[truckNo][LOADING_END] == TRUE)
		return FALSE;

	if	(app.truckState[truckNo][LOADING_START] == TRUE)
		return CMD_LOADING_END;
	else
		return CMD_LOADING_START;
}
		


/*---------------------------------------------------------------------------------------------------------------
*	void APP_cancelTruck(UINT8 *buffer)
*	Used to store the status of truck i.e., running or cancelled
*----------------------------------------------------------------------------------------------------------------
*/

void APP_cancelTruck(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);
	
	app.cancelTruck[truckNo] = CANCELLED;

}