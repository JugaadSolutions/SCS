
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/
#include "app.h"

/*
*------------------------------------------------------------------------------
* modbus master
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
* app - the app structure. 
*------------------------------------------------------------------------------
*/

typedef struct _APP
{

	//Modbus Slave buffer
	UINT8 eMBdata[NO_OF_DATA];     //store modbus receive data
	UINT8 DataReceived;				//flag for check modbus receive complete 
	//Modbus Master
	UINT8 regCount[MAX_LOG_ENTRIES];     // Buffer used to hold the number of 16bits counts in data pack

	APP_STATE state;
	UINT16 curMinute;
	UINT16 prevMinute;
	UINT8 breakID;
	UINT8 delayPercentage;
	UINT8 alarmPercentage;
	UINT8 secON;

	UINT8 updated;
	


}APP;


static rom ACTIVITY_SCHEDULE  breakSchedule[BREAKS_SUPPORTED+1]={ 
{0 , 0,0},
{550, 560,10},
{670 , 700,30},
{830 , 840,10},
{900,910 , 10 },
{1000 , 1010,10},
{1170 , 1200,30},
{1300 , 1310,10},
{00 , 300,300},
};


const rom UINT8 marqueeData[MARQUEES_SUPPORTED+1][MARQUEE_SEGMENT_CHARS]={
{""},
{"TEA BREAK"},
{"LUNCH BREAK"},
{"TEA BREAK"},
{"MEETING"},
{"TEA BREAK"},
{"LUNCH BREAK"},
{""},
{""}
};

#pragma idata APP_DATA

 ACTIVITY_SCHEDULE shipmentSchedule[TRUCKS_SUPPORTED+1][ACTIVITIES_SUPPORTED]
={
{{0, 0,0},{0 , 0,0},{0 , 0, 0}},
{{(UINT16)1340, (UINT16)1400,(UINT16)60},{(UINT16)1350, (UINT16)1410,(UINT16)60},{(UINT16)350 ,(UINT16)385 ,(UINT16)35}},
{{(UINT16)330 , (UINT16)380 ,(UINT16)50},{(UINT16)340 , (UINT16)390 ,(UINT16)50},{(UINT16)390 ,(UINT16)425 ,(UINT16)35}},
{{(UINT16)380 , (UINT16)460 ,(UINT16)80},{(UINT16)390 , (UINT16)470 ,(UINT16)80},{(UINT16)485 ,(UINT16)520 ,(UINT16)35}},
{{(UINT16)460 , (UINT16)540 ,(UINT16)80},{(UINT16)470 , (UINT16)550 ,(UINT16)80},{(UINT16)565 ,(UINT16)600 ,(UINT16)35}},
{{(UINT16)540 , (UINT16)610 ,(UINT16)70},{(UINT16)550 , (UINT16)620 ,(UINT16)70},{(UINT16)630 ,(UINT16)665 ,(UINT16)35}},
{{(UINT16)610 , (UINT16)670 ,(UINT16)60},{(UINT16)620 , (UINT16)680 ,(UINT16)60},{(UINT16)685 ,(UINT16)720 ,(UINT16)35}},
{{(UINT16)670 , (UINT16)760 ,(UINT16)90},{(UINT16)680 , (UINT16)770 ,(UINT16)90},{(UINT16)770 ,(UINT16)805 ,(UINT16)35}},
{{(UINT16)760 , (UINT16)820 ,(UINT16)60},{(UINT16)770 , (UINT16)830 ,(UINT16)60},{(UINT16)835 ,(UINT16)870 ,(UINT16)35}},
{{(UINT16)820 , (UINT16)880 ,(UINT16)60},{(UINT16)830 , (UINT16)890 ,(UINT16)60},{(UINT16)905 ,(UINT16)940 ,(UINT16)35}},
{{(UINT16)880 , (UINT16)950 ,(UINT16)70},{(UINT16)890 , (UINT16)960 ,(UINT16)70},{(UINT16)965 ,(UINT16)1000 ,(UINT16)35}},
{{(UINT16)950 , (UINT16)1020 ,(UINT16)70},{(UINT16)960 , (UINT16)1030 ,(UINT16)70},{(UINT16)1030 ,(UINT16)1065,(UINT16)35}},
{{(UINT16)1020 , (UINT16)1080,(UINT16)60},{(UINT16)1030 , (UINT16)1090,(UINT16)60},{(UINT16)1105,(UINT16)1140,(UINT16)35}},
{{(UINT16)1080, (UINT16)1140,(UINT16)60},{(UINT16)1090, (UINT16)1150,(UINT16)60},{(UINT16)1175,(UINT16)1210,(UINT16)35}},
{{(UINT16)1140, (UINT16)1220,(UINT16)80},{(UINT16)1150, (UINT16)1230,(UINT16)80},{(UINT16)1235,(UINT16)1270,(UINT16)35}},
{{(UINT16)1220, (UINT16)1280,(UINT16)60},{(UINT16)1230, (UINT16)1290,(UINT16)60},{(UINT16)1305,(UINT16)1340,(UINT16)35}},
{{(UINT16)1280, (UINT16)1340,(UINT16)60},{(UINT16)1290, (UINT16)1350,(UINT16)60},{(UINT16)1365,(UINT16)1400,(UINT16)35}},
};

APP app = {0};
//Modbus Master
LOG log = {0};

ACTIVITY_SCHEDULE breaks[BREAKS_SUPPORTED+1] = {0};

UINT8 marquee[MARQUEE_SEGMENT_CHARS];
UINT8 time_backlight[TIME_SEGMENT_CHARS + BACKLIGHT_SEGMENT_CHARS];

CurrentActivitySegment currentActivitySegment[CURRENT_ACTIVITY_SEGMENTS];
PICKING_INFO pickingInfo ;
UINT8 activityParameterBuffer[ ACTIVITY_PARAMETER_BUFFER_SIZE];

MMD_Config mmdConfig;

ACTIVITY_STATUS scheduleTable[TRUCKS_SUPPORTED][ACTIVITIES_SUPPORTED];
SCHEDULE_STATUS scheduleStatus[TRUCKS_SUPPORTED+1][ACTIVITIES_SUPPORTED];

UINT8 truck_statusIndicator[TRUCKS_SUPPORTED+1][8];

#pragma idata
volatile STATUS activityStatus;


UINT8 readTimeDateBuffer[6] = {0};
UINT8 writeTimeDateBuffer[] = {0X00, 0X30, 0X17, 0X03, 0x027, 0X12, 0X13};
UINT8 txBuffer[7] = {0};


/*
*------------------------------------------------------------------------------
* Private Functions	Prototypes
*------------------------------------------------------------------------------
*/
/*
void setSchedule(SCHEDULE_DATA *data);
*/
void resetActivitySegment(UINT8 i);
void getActivitySchedule(UINT8 truck, ACTIVITY activity, ACTIVITY_SCHEDULE* activitySchedule);
BOOL processActivityTrigger( ACTIVITY_TRIGGER_DATA* data, ACTIVITY_SCHEDULE as);
void loadActivityParameters(UINT8 segment,ACTIVITY_SCHEDULE* scheduleData,ACTIVITY_TRIGGER_DATA*data );

void APP_resetCounter_Buffer(void);
	// Function for manipulate Receive Data
void processReceivedData(void);
void updateTruckActivity(UINT8 truck ,UINT8 activity , UINT8 milestone);
	// Function for manipulate MMD (RTC & BREAK)
void updateTime(void);
void updateMarquee(void);
void APP_ASCIIconversion(void);
	//manipulate trucktime
void updateTruckTime(UINT8 truck , UINT8* trucktime);
	//manipulate hooter
void updateAlarmIndication(ACTIVITY activity);
void resetAlarm(void);
	// Function for manupilate Truck Timing  (96 Latch Digit & 16 Scan Digit)
void updateShipmentScheduleIndication(ACTIVITY_TRIGGER_DATA *data,	ACTIVITY_SCHEDULE *as);
void loadSchedule(UINT8 truck, UINT8 activity);
void updateSchedule(UINT8 *data);

	//Modbus Master
void updateLog(far UINT8 *data);

void updateCurrentActivityParameters(void);
void updateCurrentActivityIndication(void);
//void resetSchedule(UINT8 i);

/*
void updateBackLightIndication(void);

BOOL updatePickingInfo(void);
void updatePickingIndication(void);
*/

/*
*------------------------------------------------------------------------------
* void APP_init(void)
*
* Summary	: Initialize application
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/

void APP_init(void)
{
	UINT8 i, j, k, truck;
	UINT16 timeStart, timeEnd;

	UINT16 sbaudrate , saddress;

//	updateTime();
	eMBErrorCode    eStatus;

	WriteRtcTimeAndDate(writeTimeDateBuffer);

	//modbus configuration
	eStatus = eMBInit( MB_RTU, ( UCHAR )DEVICE_ADDRESS, 0, UART1_BAUD, MB_PAR_NONE);
	eStatus = eMBEnable(  );	/* Enable the Modbus Protocol Stack. */

	//modbus master initialization
	MB_init(BAUD_RATE, TIMEOUT, POLLING, RETRY_COUNT, packets, TOTAL_NO_OF_PACKETS, regs);

	//store the truck timings in the shipment schedule structure 
	for(k = 0 ; k < TRUCKS_SUPPORTED ; k++)
	{
		truck = k;
		for(i = 0 ; i < 3 ; i++)
		{
			for(j = 0; j < 2 ; j++ )
			{
				timeStart <<= 8 ;
				timeStart |=	Read_b_eep(( (truck - 1) * 12) + ((4 * i ) + j));	
				Busy_eep();
				timeEnd <<= 8 ;
				timeEnd |=	Read_b_eep(( (truck - 1) * 12) + (((4 * i ) + j) +2));	
				Busy_eep();
	
			}
			shipmentSchedule[truck + 1 ][i].startMinute = timeStart ;
			shipmentSchedule[truck + 1 ][i].endMinute = timeEnd ;
			shipmentSchedule[truck + 1 ][i].duration = timeEnd - timeStart;
	
		}
	}

	//set the value of CurrentActivitySegment structure parameter
	for(i= 0; i < ACTIVITIES_SUPPORTED; i++)
	{
		resetActivitySegment(i);
	}
	
	app.state = APP_STATE_ACTIVE;
	app.breakID = 0;
	app.secON = TRUE;

	mmdConfig.startAddress = 0;
	mmdConfig.length = 0;
	mmdConfig.symbolBuffer = 0;
	mmdConfig.symbolCount = 0;
	mmdConfig.scrollSpeed = 0;

	memset((UINT8*)scheduleTable,0,ACTIVITIES_SUPPORTED*TRUCKS_SUPPORTED);
	memset((UINT8*)&pickingInfo,0,sizeof(PICKING_INFO));
	
	activityStatus = RESET ;
	
//	updateMarquee();
	updateTime();
//	updateBackLightIndication();


}

/*
*------------------------------------------------------------------------------
* void APP_task(void)
*
* Summary	: 
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/

void APP_task(void)
{
	UINT8 i,j;
	MBErrorCode status;
	
	updateTime();

	status = MB_getStatus();
//Modubus Master 
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

////Modubus Slave Packet REceived
	DISABLE_UART_RX_INTERRUPT();

	if(app.DataReceived == TRUE)
	{
		ENABLE_UART_RX_INTERRUPT();

		processReceivedData();

		DISABLE_UART_RX_INTERRUPT();
		app.DataReceived = FALSE;	
		ENABLE_UART_RX_INTERRUPT();	
	}

	ENABLE_UART_RX_INTERRUPT();	

#if defined (RTC_DATA_ON_UART)
				for(i = 0; i < 7; i++)			
				{
					txBuffer[i] = readTimeDateBuffer[i];  //store time and date 
				}
				
				COM_txBuffer(txBuffer, 7);
#endif


	switch( app.state)
	{
		case APP_STATE_ACTIVE:


			for( i = 1 ; i < BREAKS_SUPPORTED+1 ; i++)
			{ 
				UINT16 temp;
				if((app.curMinute >= breaks[i].startMinute) && (app.curMinute < breaks[i].endMinute)) 
				{
				
					app.state = APP_STATE_INACTIVE;
					app.breakID = i;
				//	updateMarquee();
					break;
				}
		
			}

			break;

		case APP_STATE_INACTIVE:

			if( app.curMinute >= breaks[app.breakID].endMinute)
			{
				app.breakID =00;
				app.state = APP_STATE_ACTIVE;
			//	updateMarquee();
				
				
			}
			break;

		default:
			return;
	}
/*
	if( app.curMinute != app.prevMinute)
	{

		if( app.curMinute == 120  )
		{
			memset((UINT8*)scheduleTable,0,ACTIVITIES_SUPPORTED*TRUCKS_SUPPORTED);
			hdr.deviceAddress = BROADCAST_ADDRESS;
			hdr.length = 0;
			hdr.cmdID = CMD_RESET;
			COM_sendCommand(&hdr,activityParameterBuffer);

			for( i = 0 ; i < ACTIVITIES_SUPPORTED ; i++)
			{
				resetActivitySegment(i);
			}
		}

		if( updatePickingInfo() == TRUE )
		{
			updatePickingIndication();
		}

		updateCurrentActivityParameters();
		ClrWdt();
		updateCurrentActivityIndication();
		app.prevMinute = app.curMinute;
	}
*/

}

/*
*------------------------------------------------------------------------------
* MODBUS CALLBACK
*------------------------------------------------------------------------------
*/

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{

    eMBErrorCode    eStatus = MB_ENOERR;


    return eStatus;

}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{

	UINT8	starting_add = usAddress;
	UINT8	no_regs		 = usNRegs * 2;
	eMBErrorCode    eStatus = MB_ENOERR;
	UINT8 i = 0;

	switch(eMode)
	{
		case MB_REG_WRITE:
	
	    
		while( no_regs > 0)
		{
	
			app.eMBdata[i++] = * pucRegBuffer++;
	
			starting_add++;
			no_regs	--;
		}
	
//	 DISABLE_UART_RX_INTERRUPT();
		app.DataReceived = TRUE;
//	ENABLE_UART_RX_INTERRUPT();
	//	app.valueBuffer[i++] = 0;
	    break;
	
	 	case MB_REG_READ: 
	
		while(no_regs > 0)
		{
	
				* pucRegBuffer++ =	'A';
				* pucRegBuffer++ =	'B';		
				
				* pucRegBuffer++ = 'C';
				* pucRegBuffer++ = 'D';

	
			starting_add++;
			no_regs	--;	
		}
	   	 break;
	}

	return eStatus;
  }


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}

/*
*------------------------------------------------------------------------------
* void processReceivedData (void)
*
* Summary	: After received 
*
* Input		: None
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
void processReceivedData (void)
{
	UINT8 i;
	UINT8 cmd = app.eMBdata[0];
	UINT8 truck ;
	UINT8 activity;
	UINT8 milestone;
	UINT16 trucktime[6];


	switch(cmd)
	{
		case CMD_PICKING_START:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_PICKING;
			truck = (app.eMBdata[1]* 10) + app.eMBdata[2];
			milestone = MILESTONE_START ;
			updateTruckActivity(truck ,activity , milestone);
			
		break;

		case CMD_PICKING_END:
		
			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_PICKING;
			truck = (app.eMBdata[1]* 10) + app.eMBdata[2];
			milestone = MILESTONE_END;
			updateTruckActivity(truck ,activity , milestone);			

		break;

		case CMD_STAGING_START:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_STAGING;
			truck = (app.eMBdata[1]* 10) + app.eMBdata[2];
			milestone = MILESTONE_START ;
			updateTruckActivity(truck ,activity , milestone);

		break;

		case CMD_STAGING_END:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_STAGING;
			truck = (app.eMBdata[1]* 10) + app.eMBdata[2];
			milestone = MILESTONE_END;
			updateTruckActivity(truck ,activity , milestone);

		break;

		case CMD_LOADING_START	:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_LOADING;
			truck = (app.eMBdata[1]* 10) + app.eMBdata[2];
			milestone = MILESTONE_START ;
			updateTruckActivity(truck ,activity , milestone);			

		break;
		case CMD_LOADING_END:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_LOADING;
			truck = (app.eMBdata[1]* 10) + app.eMBdata[2];
			milestone = MILESTONE_END;
			updateTruckActivity(truck ,activity , milestone);			

		break;

		case CMD_TRUCK_TIMINGS	:

			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			for(i = 0 ; i < 6 ; i++)
			{
				trucktime[i] = (UINT16)( ( (app.eMBdata[3 + (i * 4)] - '0' )* 10 ) + (app.eMBdata[4 + (i * 4)] - '0' ) ) * 60
				 + ( (app.eMBdata[5 + (i * 4)] - '0' )* 10 ) + (app.eMBdata[6 + (i * 4)] - '0' );

			}

			updateTruckTime( truck , trucktime);

		break;

		case CMD_RTC	:

			writeTimeDateBuffer[0] = 0;
			writeTimeDateBuffer[2] = ((app.eMBdata[1] - '0' ) << 4) | (app.eMBdata[2] - '0' ); //store minutes
			writeTimeDateBuffer[1] = ((app.eMBdata[3] - '0') << 4) | (app.eMBdata[4] - '0'); //store houres
		
			WriteRtcTimeAndDate(writeTimeDateBuffer);  //update RTC
			
		break;

		case CMD_CANCEL_TRUCK	:
		{

			activity = ACTIVITY_CANCEL;
			truck = (app.eMBdata[1]* 10) + app.eMBdata[2];
			milestone = MILESTONE_NONE;
			updateTruckActivity(truck ,activity , milestone);

		}	
		break;


		case CMD_HOOTER_OFF	:
			
			resetAlarm();
		break;


		default:
		break;
	}
}


/*
*------------------------------------------------------------------------------
* void updateTruckActivity(UINT8 truck ,UINT8 activity , UINT8 milestone);
*
* Summary	: 
*
* Input		: None
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
void updateTruckActivity(UINT8 truck ,UINT8 activity , UINT8 milestone)
{
	ACTIVITY_SCHEDULE as;
	ACTIVITY_TRIGGER_DATA *data ;

	data->truck = truck;
	data->activity = activity;
	data->mileStone = milestone;
	
	if( (data->activity == ACTIVITY_CANCEL) && (data->mileStone == MILESTONE_NONE) )
	{
		updateShipmentScheduleIndication(data , 0);
		ClrWdt();
	}
	else
	{
		getActivitySchedule(data->truck ,data->activity, &as);
	
		if( processActivityTrigger(data,as ) == TRUE)
		{
	
			updateCurrentActivityParameters();
			ClrWdt();
	
			updateCurrentActivityIndication();
			ClrWdt();

			updateShipmentScheduleIndication(data,&as);
			ClrWdt();
	
			if( data->mileStone == MILESTONE_START)
			{
				updateAlarmIndication(data->activity);
				if( data->truck == pickingInfo.truck )
				{
					pickingInfo.truck = 0;
					pickingInfo.state = 0;
					pickingInfo.timeout = 0;		
				//	updatePickingIndication();	
				}
			}
				
		//	updateBackLightIndication();
			ClrWdt();
	
		}
	}
}

/*
*------------------------------------------------------------------------------
* void updateTruckTime(UINT8 truck , UINT8* trucktime);
*
* Summary	: 
*
* Input		: truck , ps ,pe , ss , se , ls, le
*			  
*
* Output	: None
*
*
*
*
*------------------------------------------------------------------------------
*/
void updateTruckTime(UINT8 truck , UINT8* trucktime)
{
	UINT8 i , j ,k;
	UINT16 timeStart,timeEnd ;

	for(i = 0 ; i < 6 ; i++)
	{
		for(j = 0 ; j < 2 ; j++)
		{
			Write_b_eep(( (truck - 1) * 12) + ((2 * i ) + j)  , *(trucktime +((2 * i) + (1 - j) ) ) );	
			Busy_eep();
		}
	}

	for(i = 0 ; i < 3 ; i++)
	{
		for(j = 0 , k  = 0 ; j < 2 ; j++ , k++)
		{
			timeStart <<= 8 ;
			timeStart |=	Read_b_eep(( (truck - 1) * 12) + ((4 * i ) + j));	
			Busy_eep();
			timeEnd <<= 8 ;
			timeEnd |=	Read_b_eep(( (truck - 1) * 12) + (((4 * i ) + j) +2));	
			Busy_eep();

		}
		shipmentSchedule[truck ][i].startMinute = timeStart ;
		shipmentSchedule[truck ][i].endMinute = timeEnd ;
		shipmentSchedule[truck ][i].duration = timeEnd - timeStart;

	}

}
/*
*------------------------------------------------------------------------------
*void resetAlarm(void);
*
* Summary	: Trun OFF Hooter
*
* Input		: None
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
void resetAlarm()
{
	HOOTER = SWITCH_OFF;
}

/*
*------------------------------------------------------------------------------
*void updateAlarmIndication(ACTIVITY activity)
*
* Summary	: Trun ON Hooter based on Activity Status
*
* Input		: None
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/

void updateAlarmIndication(ACTIVITY activity)
{
	UINT8 i;
	UINT32 actualPercentage, planPercentage;
	actualPercentage = currentActivitySegment[activity-1].actualPercentage;
	planPercentage = currentActivitySegment[activity-1].planPercentage;
	if((planPercentage - actualPercentage)/100 >= app.alarmPercentage)
	{
		HOOTER = SWITCH_ON;
		return;
	}

}


/*
*------------------------------------------------------------------------------
*void resetActivitySegment(UINT8 i);
*
* Summary	: 
*
* Input		: activity
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
void resetActivitySegment(UINT8 i)
{
	currentActivitySegment[i].no = 0;
	currentActivitySegment[i].status = RESET;
	currentActivitySegment[i].activity = ACTIVITY_SCHEDULED;
	currentActivitySegment[i].planProgress =0;
	currentActivitySegment[i].planPercentage=0;
	currentActivitySegment[i].actualProgress =0;
	currentActivitySegment[i].actualPercentage =0;
	currentActivitySegment[i].planSchedule = shipmentSchedule[0][0];
	currentActivitySegment[i].actualSchedule = shipmentSchedule[0][0];

	currentActivitySegment[i].free = TRUE; 


}

/*
*------------------------------------------------------------------------------
*void getActivitySchedule(UINT8 truck, ACTIVITY activity, ACTIVITY_SCHEDULE* activitySchedule)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
void getActivitySchedule(UINT8 truck, ACTIVITY activity, ACTIVITY_SCHEDULE* activitySchedule)
{
#ifdef __FACTORY_CONFIGURATION__

	*activitySchedule = shipmentSchedule[truck][activity-1];
#else
	ReadBytesEEP(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + truck * sizeof(TRUCK_SCHEDULE) + (sizeof(ACTIVITY_SCHEDULE) * activity-1)
							 ,(UINT8 *)activitySchedule,sizeof(ACTIVITY_SCHEDULE));
#endif
}

/*
*------------------------------------------------------------------------------
*BOOL processActivityTrigger( ACTIVITY_TRIGGER_DATA* data, ACTIVITY_SCHEDULE as)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
BOOL processActivityTrigger( ACTIVITY_TRIGGER_DATA* data, ACTIVITY_SCHEDULE as)
{
	UINT8 i , j;
	BOOL result= FALSE;
	if( data->truck > TRUCKS_SUPPORTED )
	{
		return result;
	}

	switch( data->mileStone )
	{
		case MILESTONE_END:
			
		if( data->truck  != currentActivitySegment[data->activity-1].no )			//if truck numbers don't match ignore
			return FALSE;
		
		if ( scheduleTable[data->truck -1][data->activity - 1] != ACTIVITY_ONGOING) 	//if the particular activity has not started ignore
			return FALSE;

		if( currentActivitySegment[data->activity-1].planSchedule.endMinute <= app.curMinute )
		{
			activityStatus = DELAYED;
		}
		else
		{
			activityStatus = ON_TIME;
		}
		resetActivitySegment(data->activity -1 );										//reset the activity segment
		scheduleTable[data->truck-1][data->activity-1] = ACTIVITY_COMPLETED;			//update scheduleTable

		result = TRUE;								
				
		break;

		case MILESTONE_START:
			if(currentActivitySegment[data->activity-1].free != TRUE )			// if activity segment not free ignore
					return FALSE;
	
			if( scheduleTable[data->truck -1][data->activity - 1] != ACTIVITY_SCHEDULED	)	
					return FALSE;

			loadActivityParameters(data->activity-1, &as,data );

			scheduleTable[data->truck-1][data->activity-1] = ACTIVITY_ONGOING;		//update scheduleTable 



			result = TRUE;
			
		break;
	}

	return result;
}
	
/*
*------------------------------------------------------------------------------
*void loadActivityParameters(UINT8 segment,ACTIVITY_SCHEDULE* scheduleData,ACTIVITY_TRIGGER_DATA*data );
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
void loadActivityParameters(UINT8 segment,ACTIVITY_SCHEDULE* scheduleData,ACTIVITY_TRIGGER_DATA*data )
{
	currentActivitySegment[segment].no = data->truck;
	currentActivitySegment[segment].activity = data->activity;
	currentActivitySegment[segment].planSchedule = *scheduleData;
	currentActivitySegment[segment].actualSchedule.startMinute = app.curMinute;
	currentActivitySegment[segment].free = FALSE;

}

/*
*------------------------------------------------------------------------------
*void updateTime(void);
*
* Summary	: Read time from RTC and Display on MMD
*
* Input		: None
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
void updateTime(void)
{


	UINT8 time[6];

	UINT8 i;
	UINT16 hour ,minute;
	ReadRtcTimeAndDate(readTimeDateBuffer);	//Read RTC data and store it in buffer
	hour= (UINT16)readTimeDateBuffer[2];
	minute = (UINT16)readTimeDateBuffer[1];
#ifdef TIME_DEBUG
	app.curMinute = hour*60 + minute;
	UTL_binaryToBCDASCII(hour , &time_backlight[TIME_HOUR_INDEX]);
	UTL_binaryToBCDASCII(minute , &time_backlight[TIME_MINUTE_INDEX]);
#else
	app.curMinute = ((UINT16)BCDtoBin(hour))*60 + (UINT16)BCDtoBin(minute);
	UTL_binaryToBCDASCII(BCDtoBin(hour) , &time_backlight[TIME_HOUR_INDEX]);
	UTL_binaryToBCDASCII(BCDtoBin(minute) , &time_backlight[TIME_MINUTE_INDEX]);
#endif
	
	if( app.secON == TRUE)
	{
		time_backlight[TIME_SECOND_INDEX] = SYM_SEC_LEFT;
		time_backlight[TIME_SECOND_INDEX+1] = SYM_SEC_RIGHT;
		app.secON = FALSE;
	}
	else
	{
		time_backlight[TIME_SECOND_INDEX] = ' ';
		time_backlight[TIME_SECOND_INDEX+1] = ' ';
		app.secON = TRUE;
	}

	time_backlight[BACKLIGHT_TRUCK_INDEX] = SYM_ALL;
	time_backlight[BACKLIGHT_STATUS_INDEX] = SYM_ALL;
	time_backlight[BACKLIGHT_PICKING_INDEX] = SYM_ALL;
	time_backlight[BACKLIGHT_STAGING_INDEX] = SYM_ALL;
	time_backlight[BACKLIGHT_LOADING_INDEX] = SYM_ALL;

	mmdConfig.startAddress = TIME_SEGMENT_START_ADDRESS ;
	mmdConfig.length = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.symbolBuffer = time_backlight;
	mmdConfig.symbolCount = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;

	MMD_configSegment(1, &mmdConfig);


}
/*

void updateBackLightIndication(void)
{
	UINT8 i;
	BOOL picking, staging , loading;

	time_backlight[BACKLIGHT_TRUCK_INDEX] = SYM_ALL;
	time_backlight[BACKLIGHT_STATUS_INDEX] = SYM_ALL;

	for( i  = 0 ; i < ACTIVITIES_SUPPORTED ; i++)
	{
		switch(currentActivitySegment[i].activity)
		{
			case ACTIVITY_PICKING:
				picking = TRUE;
			break;

			case ACTIVITY_STAGING:
				staging = TRUE;
			break;			

			case ACTIVITY_LOADING:
				loading = TRUE;
			break;	

			default:
			break;
		}
	}
	if( picking == TRUE )
		time_backlight[BACKLIGHT_PICKING_INDEX] = SYM_ALL;
	else
		time_backlight[BACKLIGHT_PICKING_INDEX] = ' ';
	
	if( staging == TRUE )
		time_backlight[BACKLIGHT_STAGING_INDEX] = SYM_ALL;
	else
		time_backlight[BACKLIGHT_STAGING_INDEX] = ' ';		

	if( loading == TRUE )
		time_backlight[BACKLIGHT_LOADING_INDEX] = SYM_ALL;
	else
		time_backlight[BACKLIGHT_LOADING_INDEX] = ' ';	


	mmdConfig.startAddress =TIME_SEGMENT_START_ADDRESS ;
	mmdConfig.length = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.symbolBuffer = time_backlight;
	mmdConfig.symbolCount = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;

	MMD_configSegment(1, &mmdConfig);

}
void updateMarquee(void)
{
	UINT8 i;
	const rom UINT8* mData;	//marquee data

	MMD_clearSegment(0);

	for( i = 0; i < MARQUEE_SEGMENT_CHARS; i++)
	{
		marquee[i] = ' ';
	}

	i = 0;
	mData = marqueeData[app.breakID];
	while(*mData !='\0')
	{
		marquee[i] = *mData;
		mData++;
		i++;
	}
	if( i <= (MARQUEE_SEGMENT_LENGTH + 5 ))
	{
		marquee[i++] = ' ';
	}

	mmdConfig.startAddress = MARQUEE_SEGMENT_START_ADDRESS;
	mmdConfig.length = MARQUEE_SEGMENT_LENGTH;
	mmdConfig.symbolBuffer = marquee;
	mmdConfig.symbolCount = i;
	mmdConfig.scrollSpeed = SCROLL_SPEED_HIGH;

	

	MMD_configSegment(0, &mmdConfig);

}
*/	

/*
*------------------------------------------------------------------------------
* void updateCurrentActivityParameters(void)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/

void updateCurrentActivityParameters(void)
{
	UINT8 i,j;
	UINT32 temp;
	UINT8 progress = 0;
	UINT16 breakDuration = 0;

	if( app.state == APP_STATE_ACTIVE )			//loading active during breaks
		i = 0;
	else i = 2;

	for(; i< ACTIVITIES_SUPPORTED ; i++)
	{
		progress = 0;
		breakDuration = 0;
		

		if( currentActivitySegment[i].free == TRUE)	
			continue;

		if( i < 2 )																//only for picking and staging activity segments
		{
			for(j = 1; j < BREAKS_SUPPORTED; j++)
			{
				if( currentActivitySegment[i].planSchedule.startMinute < breaks[j].startMinute 
					&& (currentActivitySegment[i].planSchedule.endMinute >= breaks[j].endMinute ))
				{
					if( app.curMinute >= breaks[j].endMinute ) 
						breakDuration += breaks[j].duration;
				}
			}
		}

		if( app.curMinute < currentActivitySegment[i].planSchedule.startMinute ) 	//check for advance
		{
			currentActivitySegment[i].planPercentage = 0;
		}
		else
		{
				
			currentActivitySegment[i].planPercentage = 
				(((UINT32)app.curMinute - (UINT32)currentActivitySegment[i].planSchedule.startMinute - breakDuration ) * 100*100)/((UINT32)currentActivitySegment[i].planSchedule.duration);
		}

		currentActivitySegment[i].actualPercentage = 
			(((UINT32)app.curMinute - (UINT32)currentActivitySegment[i].actualSchedule.startMinute - breakDuration )*100*100)/((UINT32)currentActivitySegment[i].planSchedule.duration);

		
		if( (( currentActivitySegment[i].planPercentage - currentActivitySegment[i].actualPercentage))/100 >= app.delayPercentage )
		{
			currentActivitySegment[i].status = DELAYED;
		}
		else
			currentActivitySegment[i].status = ON_TIME;



		if( currentActivitySegment[i].planPercentage >= 9900)
			currentActivitySegment[i].planPercentage = 9900;

		temp = currentActivitySegment[i].planPercentage;
		while( temp >= 275)
		{
			
			progress++;
			temp-=275;
		}
		currentActivitySegment[i].planProgress = (progress>36)? 36 : progress;

		progress = 0;
			
		if( currentActivitySegment[i].actualPercentage >= 9900)
			currentActivitySegment[i].actualPercentage = 9900;
		temp = currentActivitySegment[i].actualPercentage;
		while( temp >= 275)
		{
			
			progress ++;
			temp-=275;
		}

		currentActivitySegment[i].actualProgress = (progress>36)? 36 : progress;
		
	}
}

/*
*------------------------------------------------------------------------------
* void updateCurrentActivityIndication(void)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/


void updateCurrentActivityIndication(void)
{
	UINT8 i,j,temp;
	


	if( app.state == APP_STATE_ACTIVE )
		i = 0;
	else i = 2;


	for(; i < ACTIVITIES_SUPPORTED; i++)
	{
		j = 0;
		activityParameterBuffer[j++] = currentActivitySegment[i].no;
		activityParameterBuffer[j++] = currentActivitySegment[i].activity;
		activityParameterBuffer[j++] = currentActivitySegment[i].status;
		activityParameterBuffer[j++] = currentActivitySegment[i].planProgress;

		temp = (currentActivitySegment[i].planPercentage/100);
		activityParameterBuffer[j++] = (temp >= 99) ? (99) : temp;

		activityParameterBuffer[j++] = currentActivitySegment[i].actualProgress;

		temp = currentActivitySegment[i].actualPercentage/100;
		activityParameterBuffer[j++] = ( temp >= 99 ) ? (99) : temp;

		//	hdr.deviceAddress = CURRENT_ACTIVITY_DEVICE_START_ADDRESS+i;
		//	hdr.length = j;
		//	hdr.cmdID = CMD_SET_SEGMENT;

	//	COM_sendCommand(&hdr,activityParameterBuffer);
		DelayMs(30);
	}
}

/*
*------------------------------------------------------------------------------
* void updateShipmentScheduleIndication(ACTIVITY_TRIGGER_DATA *data,	ACTIVITY_SCHEDULE *as)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/

void updateShipmentScheduleIndication(ACTIVITY_TRIGGER_DATA *data,	ACTIVITY_SCHEDULE *as)
{
	UINT8 i = 0;
	UINT8 deviceAddress;
	UINT8 cmd;
	UINT8 length;
	activityParameterBuffer[i++] = data->truck;
	activityParameterBuffer[i++] = data->activity;
	if( data->activity != ACTIVITY_CANCEL)
	{
		activityParameterBuffer[i++] = data->mileStone;
		activityParameterBuffer[i++] = (data->mileStone == MILESTONE_START)? currentActivitySegment[data->activity-1].status : activityStatus;
		activityParameterBuffer[i++] = (as->startMinute >> 8)&0xFF;
		activityParameterBuffer[i++] = (as->startMinute )&0xFF;
		activityParameterBuffer[i++] = (as->endMinute >> 8 ) & 0xFF;
		activityParameterBuffer[i++] = (as->endMinute) & 0xFF;
		activityParameterBuffer[i++] = (app.curMinute >> 8 )&0xFF;
		activityParameterBuffer[i++] = (app.curMinute )&0xFF;
	}

	if ( (data->truck-1) < 4 )
	{
		updateSchedule(activityParameterBuffer);
	}
	else
	{
		deviceAddress = ((data->truck-1) /4) + 1;
		length = i;
		//cmdID = CMD_UPDATE_SHIPMENT_SCHEDULE;
		//COM_txCMD_CHAN1( deviceAddress, cmd, activityParameterBuffer ,length)
	}
}


/*
*------------------------------------------------------------------------------
* void updateSchedule(SCHEDULE_UPDATE_INFO *info)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/

void updateSchedule(UINT8 *data)
{
	UINT8 i;
	UINT8 truck;
	SCHEDULE_UPDATE_INFO *info; 
	UINT8 activityCompleteFlag = TRUE;
	INT8 delayedActivity = 0xFF;
	info = (SCHEDULE_UPDATE_INFO*) data;
	truck = info->truck -((DEVICE_ADDRESS - 1) * 4);


	if( info->activity == ACTIVITY_CANCEL)
	{
		
		for(i = 0 ; i < ACTIVITIES_SUPPORTED;i++)
		{
			if( scheduleStatus[truck][info->activity - 1].activityStatus != ACTIVITY_SCHEDULED)	//if activity is not scheduled ignore cmd
				return ;
		}
		for(i = 0 ; i < ACTIVITIES_SUPPORTED;i++)
		{
			(scheduleStatus[truck][i]).activityStatus = ACTIVITY_CANCELLED;
		}

/*		truck_statusIndicator[truck][0] = truckIndicators[info->truck].indicatorRed[0];
		truck_statusIndicator[truck][1] = truckIndicators[info->truck].indicatorRed[1];
		truck_statusIndicator[truck][2] = truckIndicators[info->truck].indicatorRed[2];
		truck_statusIndicator[truck][3] = truckIndicators[info->truck].indicatorRed[3];

		truck_statusIndicator[truck][4] = SYM_CANCEL;
		truck_statusIndicator[truck][5] = SYM_CANCEL;
		truck_statusIndicator[truck][6] = ' ';
		truck_statusIndicator[truck][7] = ' ';
		clearScheduleTime();
*/

	}

	else
	{
		switch( info->milestone)
		{
			case MILESTONE_START:
			if( scheduleStatus[truck][info->activity - 1].activityStatus != ACTIVITY_SCHEDULED)				//if activity is not scheduled ignore cmd
				return ;
			
/*			truck_statusIndicator[truck][0] = truckIndicators[info->truck].indicatorGreen[0];
			truck_statusIndicator[truck][1] = truckIndicators[info->truck].indicatorGreen[1];
			truck_statusIndicator[truck][2] = truckIndicators[info->truck].indicatorGreen[2];
			truck_statusIndicator[truck][3] = truckIndicators[info->truck].indicatorGreen[3];

			truck_statusIndicator[truck][4] = ' ';
			truck_statusIndicator[truck][5] = SYM_ONGOING;
			truck_statusIndicator[truck][6] = ' ';
			truck_statusIndicator[truck][7] = ' ';
*/

//			getScheduleTime(&scheduleTable[truck][info->activity-1] , activityTime);
			
						
			scheduleStatus[truck][info->activity - 1].activityStatus = ACTIVITY_ONGOING;
			scheduleStatus[truck][info->activity - 1].status = info->status;
			break;

			case MILESTONE_END:
			if( scheduleStatus[truck][info->activity - 1].activityStatus != ACTIVITY_ONGOING)				//if activity is not scheduled ignore cmd
				return ;

/*			truck_statusIndicator[truck][0] = truckIndicators[info->truck].indicatorRed[0];
			truck_statusIndicator[truck][1] = truckIndicators[info->truck].indicatorRed[1];
			truck_statusIndicator[truck][2] = truckIndicators[info->truck].indicatorRed[2];
			truck_statusIndicator[truck][3] = truckIndicators[info->truck].indicatorRed[3];


			clearScheduleTime();
*/
			loadSchedule(truck,info->activity);

			scheduleStatus[truck][info->activity - 1].activityStatus = ACTIVITY_COMPLETED;
			scheduleStatus[truck][info->activity - 1].status = info->status;

			truck_statusIndicator[truck][5] = SYM_COMPLETE;

			for(i = 0; i  < ACTIVITIES_SUPPORTED ; i++)
			{
				if( scheduleStatus[truck][i].activityStatus == ACTIVITY_ONGOING )			
				{
					return;
				}
			}

			for(i = 0; i  < ACTIVITIES_SUPPORTED ; i++)
			{
				if( scheduleStatus[truck][i].status == DELAYED)			
				{
					delayedActivity = i	;
					break;
				}
			}

			if( i < ACTIVITIES_SUPPORTED )
			{
				truck_statusIndicator[truck][4] = SYM_COMPLETE;
				truck_statusIndicator[truck][6] = i+SYM_PICKING;
				truck_statusIndicator[truck][7] = i+SYM_PICKING;
			}
			else
			{
				truck_statusIndicator[truck][4] = ' ';
				truck_statusIndicator[truck][6] = ' ';
				truck_statusIndicator[truck][7] = ' ';
			}
			
			break;

			default:
			break;
		}

		
	}

	mmdConfig.startAddress = (truck - 1)*32;
	mmdConfig.length = 8;
	mmdConfig.symbolBuffer =truck_statusIndicator[truck] ;
	mmdConfig.symbolCount = 8;
	mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;



	MMD_configSegment(truck-1, &mmdConfig);


	loadSchedule(truck,info->activity);
}

/*
*------------------------------------------------------------------------------
* void resetSchedule(UINT8 truck)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
/*
void resetSchedule(UINT8 truck)
{
	UINT8 j;
	UINT8 truckNo;
	
	
	truckNo = truck+((DEVICE_ADDRESS - 1) * 4);
	for( j= 0; j < ACTIVITIES_SUPPORTED ; j++)
	{
		
		scheduleStatus[truck][j].activityStatus = ACTIVITY_SCHEDULED;
		scheduleStatus[truck][j].status = ACTIVITY_NONE;
		
		getScheduleTime(&scheduleTable[truck][j] ,activityTime);

		loadSchedule(truck,j+1);


	}	

	truck_statusIndicator[truck][0] = truckIndicators[truckNo].indicatorRed[0];
	truck_statusIndicator[truck][1] = truckIndicators[truckNo].indicatorRed[1];
	truck_statusIndicator[truck][2] = truckIndicators[truckNo].indicatorRed[2];
	truck_statusIndicator[truck][3] = truckIndicators[truckNo].indicatorRed[3];

	truck_statusIndicator[truck][4] = 'F';
	truck_statusIndicator[truck][5] = ' ';
	truck_statusIndicator[truck][6] = 'G';
	truck_statusIndicator[truck][7] = ' ';

	mmdConfig.startAddress = (truck-1)*32;
	mmdConfig.length = 8;
	mmdConfig.symbolBuffer =truck_statusIndicator[truck] ;
	mmdConfig.symbolCount = 8;
	mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;

	MMD_configSegment(truck-1, &mmdConfig);

	
}
*/

/*
*------------------------------------------------------------------------------
* void loadSchedule(UINT8 truck, UINT8 activity)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/

void loadSchedule(UINT8 truck, UINT8 activity)
{
	UINT8 i;
	for(i = 0; i < 8 ;i++)
	{
//		DDR_loadDigit( ((truck-1)*32)+(activity*8)+ i,activityTime[i] );
		DelayMs(1);
	}
}



/*


void setSchedule(SCHEDULE_DATA *data)
{
	UINT8 i;
	WriteBytesEEP(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + data->truck*(sizeof(TRUCK_SCHEDULE))
									, (UINT8*)&data,sizeof(ACTIVITY_SCHEDULE)*ACTIVITIES_SUPPORTED);
}

*/


/*
BOOL updatePickingInfo()
{
	UINT8 i,result = FALSE;
	switch( pickingInfo.state)
	{
		case 0:
		for(i = 1; i < TRUCKS_SUPPORTED+1 ; i++)
		{
			if( app.curMinute != pickingStartTime[i] )
				continue;
			pickingInfo.truck = i;
			pickingInfo.state = 1;
			pickingInfo.timeout = 3;
			result = TRUE;
				
		}

		break;
		case 1:
		--pickingInfo.timeout;
		if( pickingInfo.timeout == 0 )
		{
			pickingInfo.timeout = 3;
			pickingInfo.state = 2;
			result = TRUE;
		}
		break;

		case 2:
		--pickingInfo.timeout;
		if( pickingInfo.timeout == 0 )
		{
			pickingInfo.state = 3;
			result = TRUE;

		}
		break;

		default:
		break;
	}
	return result;
}


void updatePickingIndication()
{
	UINT8 i = 0,result = FALSE;
	switch( pickingInfo.state)
	{
		case 0:
		activityParameterBuffer[i++] = 0;	
		activityParameterBuffer[i++] = 0;
		activityParameterBuffer[i++] = 0;
		activityParameterBuffer[i++] = 0;
				
		break;
		case 1:
		activityParameterBuffer[i++] = 1;	
		activityParameterBuffer[i++] = 0;
		activityParameterBuffer[i++] = 0;
		activityParameterBuffer[i++] = 0;
			
		break;

		case 2:
		activityParameterBuffer[i++] = 0;	
		activityParameterBuffer[i++] = 1;
		activityParameterBuffer[i++] = 0;
		activityParameterBuffer[i++] = 0;
		break;


		case 3:
		activityParameterBuffer[i++] = 0;	
		activityParameterBuffer[i++] = 0;
		activityParameterBuffer[i++] = 1;
		activityParameterBuffer[i++] = 1;
		break;

		default:
		break;
	}
		
	hdr.deviceAddress = 9;
	hdr.length = i;
	hdr.cmdID = CMD_UPDATE_PICKING_INDICATION;
	COM_sendCommand(&hdr,activityParameterBuffer);

}
*/



/*---------------------------------------------------------------------------------------------------------------
*	void updateLog(void)
*----------------------------------------------------------------------------------------------------------------
*/
void updateLog(far UINT8 *data)
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



