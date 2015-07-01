
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

MBErrorCode PACKET_SENT = 1;
MBErrorCode RETRIES_DONE = 4;

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
	UINT32 preAppTime,curAppTime;
	UINT8 stationCount[2];
	UINT8 password[5];
	UINT8 logonPassword[5];
	UINT8 logonStatus;
	UINT8 openIssue;
	UINT8 truckState[MAX_NO_OF_TRUCKS][MAX_STATES];	//used to store the state of each truck
}APP;																			//This object contains all the varibles used in this application




/*
*------------------------------------------------------------------------------
* Variables
*------------------------------------------------------------------------------
*/
#pragma idata APP_DATA
APP ias = {0};
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


UINT8 getRegCount(void);


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

	ias.password[0] = '1';
	ias.password[1] = '0';
	ias.password[2] = '0';
	ias.password[3] = '3';
	ias.password[4] = '\0';

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
			regCount = getRegCount();

			if(regCount == 0)
				return;
			
			MB_construct(&packets[PACKET1], SLAVE_ID, PRESET_MULTIPLE_REGISTERS, 
								STARTING_ADDRESS, regCount, log.entries[log.readIndex]);	

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
	log.entries[log.writeIndex][i]= (UINT16)'\0';
	log.writeIndex++;
	if( log.writeIndex >= MAX_LOG_ENTRIES)
		log.writeIndex = 0;
}

	

BOOL APP_checkPassword(UINT8 *password)
{

	if( strcmp(ias.password , password) )
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

	if(ias.truckState[truckNo][PICKING_START] == TRUE)
	{
		ias.truckState[truckNo][PICKING_END] = TRUE;
		buffer[0] = CMD_PICKING_END;
	}
	else
	{
		ias.truckState[truckNo][PICKING_START] = TRUE;
		buffer[0] = CMD_PICKING_START;
	}
}

UINT8 APP_validatePicking(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	if	(ias.truckState[truckNo][PICKING_END] == TRUE)
		return FALSE;

	if	(ias.truckState[truckNo][PICKING_START] == TRUE)
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

	if(ias.truckState[truckNo][STAGING_START] == TRUE)
	{
		ias.truckState[truckNo][STAGING_END] = TRUE;
		buffer[0] = CMD_STAGING_END;
	}
	else
	{
		ias.truckState[truckNo][STAGING_START] = TRUE;
		buffer[0] = CMD_STAGING_START;
	}
}

UINT8 APP_validateStaging(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	if	(ias.truckState[truckNo][STAGING_END] == TRUE)
		return FALSE;

	if	(ias.truckState[truckNo][STAGING_START] == TRUE)
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

	if(ias.truckState[truckNo][LOADING_START] == TRUE)
	{
		ias.truckState[truckNo][LOADING_END] = TRUE;
		buffer[0] = CMD_LOADING_END;
		
	}
	else
	{
		ias.truckState[truckNo][LOADING_START] = TRUE;
		buffer[0] = CMD_LOADING_START;
	}
}

UINT8 APP_validateLoading(UINT8 *buffer)
{
	UINT8 truckNo = atoi(buffer);

	if	(ias.truckState[truckNo][LOADING_END] == TRUE)
		return FALSE;

	if	(ias.truckState[truckNo][LOADING_START] == TRUE)
		return CMD_LOADING_END;
	else
		return CMD_LOADING_START;
}
		
/*---------------------------------------------------------------------------------------------------------------
*	UINT8 APP_manageLoading(UINT8 *buffer)
*	used to calculate data count in the buffer.
* 	If the data count is odd, add+1 to it.
*----------------------------------------------------------------------------------------------------------------
*/
UINT8 getRegCount(void) 
{
	UINT8 i,regCount = 0;
	UINT16 hooterOff;

	regCount = strlen(log.entries[log.readIndex]);

	if(regCount == 0)
	{
		hooterOff = log.entries[log.readIndex][0];
		hooterOff >>= 8;
		
		if(hooterOff == (UINT16)CMD_HOOTER_OFF)
			regCount = 1;
	}
	
	return regCount;
}
	

