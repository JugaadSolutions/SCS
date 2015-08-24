
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/
#include "mutex.h"
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
{0 , 300,300},
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

MMD_Config mmdConfig = {0};

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

LOG log = {0}; // Useb by Modbus Master

ACTIVITY_SCHEDULE breaks[BREAKS_SUPPORTED+1] = {0};

UINT8 marquee[MARQUEE_SEGMENT_CHARS] = {0};
UINT8 time_backlight[TIME_SEGMENT_CHARS + BACKLIGHT_SEGMENT_CHARS] = {0};

CurrentActivitySegment currentActivitySegment[CURRENT_ACTIVITY_SEGMENTS] = {0};
PICKING_INFO pickingInfo ;
UINT8 activityParameterBuffer[ ACTIVITY_PARAMETER_BUFFER_SIZE] = {0};




SCHEDULE_STATUS scheduleStatus[TRUCKS_SUPPORTED+1][ACTIVITIES_SUPPORTED] = {0};

//UINT8 truck_statusIndicator[TRUCKS_SUPPORTED+1][8] = {0};

UINT8 truckNo[TRUCKS_SUPPORTED_BOARD * 2] = {0};		//buffer for truck nos
UINT8 truckStatus[2] = {0};	//buffer for truck status

UINT16 pickingStartTime[TRUCKS_SUPPORTED+1] = {0};
volatile STATUS activityStatus = 0;
UINT8 readTimeDateBuffer[6] = {0};
UINT8 writeTimeDateBuffer[] = {0X00, 0X00, 0X16, 0X03, 0x027, 0X12, 0X13};
UINT8 txBuffer[7] = {0};
UINT8 transmitTruncktime[30] = {0};
UINT8 activityTime[8];
UINT16 trucktime[6];

#pragma idata





/*
*------------------------------------------------------------------------------
* Private Functions	Prototypes
*------------------------------------------------------------------------------
*/
/*
void setSchedule(SCHEDULE_DATA *data);
*/
void resetActivitySegment(UINT8 i);
void getActivitySchedule(UINT8 truck, ACTIVITY activity, far ACTIVITY_SCHEDULE* activitySchedule);
BOOL processActivityTrigger( far ACTIVITY_TRIGGER_DATA* data, ACTIVITY_SCHEDULE as);
void loadActivityParameters(UINT8 segment,far ACTIVITY_SCHEDULE* scheduleData,far ACTIVITY_TRIGGER_DATA*data );

void APP_resetCounter_Buffer(void);
	// Function for manipulate Receive Data
void processReceivedData(void);
void updateTruckActivity(UINT8 truck ,UINT8 activity , UINT8 milestone);
	// Function for manipulate MMD (RTC & BREAK)
void updateTime(void);
void updateMarquee(void);
void APP_ASCIIconversion(void);
	//manipulate trucktime
void updateTruckTime(UINT8 truck , far UINT16* trucktime);
	//manipulate hooter
void updateAlarmIndication(ACTIVITY activity);
void resetAlarm(void);

	// Function for manupilate Truck Timing  (96 Latch Digit & 16 Scan Digit)
void updateShipmentScheduleIndication(far ACTIVITY_TRIGGER_DATA *data,far 	ACTIVITY_SCHEDULE *as);
void loadSchedule(UINT8 truck, UINT8 activity);
void updateSchedule(far  UINT8 *data);
void getScheduleTime(far ACTIVITY_SCHEDULE* as , far UINT8* activityTime);
void setSchedule(far SCHEDULE_DATA *data);
void resetSchedule(void);
void clearScheduleTime(void);

	//Modbus Master
void updateLog(far UINT8 *data,UINT8 slave);
void updateLog_Binary(far UINT8 *data,UINT8 slave,UINT8 length);

void updateCurrentActivityParameters(void);
void updateCurrentActivityIndication(void);

void displayTruckNumber(UINT8* buffer);
void updateBackLightIndication(void);
BOOL updatePickingInfo(void);
void updatePickingIndication(void);

void COM_txBuffer(UINT8 *txData, UINT8 length);

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
	UINT16 timeStart = 0, timeEnd = 0;
	UINT8 test[] = "ABCDEF";

	ACTIVITY_SCHEDULE as;

	eMBErrorCode    eStatus;

//	WriteRtcTimeAndDate(writeTimeDateBuffer);
/*
	//store the truck timings in the shipment schedule structure 
	for(k = 0 ; k < TRUCKS_SUPPORTED ; k++)
	{
		truck = k;
		for(i = 0 ; i < 3 ; i++)
		{
			for(j = 0; j < 2 ; j++ )
			{
				timeStart <<= 8 ;
				timeStart |=	Read_b_eep(( (truck) * 12) + ((4 * i ) + j));	
				Busy_eep();
				timeEnd <<= 8 ;
				timeEnd |=	Read_b_eep(( (truck) * 12) + (((4 * i ) + j) +2));	
				Busy_eep();
	
			}
			shipmentSchedule[truck + 1 ][i].startMinute = timeStart ;
			shipmentSchedule[truck + 1 ][i].endMinute = timeEnd ;
			shipmentSchedule[truck + 1 ][i].duration = timeEnd - timeStart;
	
		}
	}

*/


	//modbus configuration
	eStatus = eMBInit( MB_RTU, ( UCHAR )DEVICE_ADDRESS, 0, UART1_BAUD, MB_PAR_NONE);
	eStatus = eMBEnable(  );	/* Enable the Modbus Protocol Stack. */

	//modbus master initialization
	MB_init(BAUD_RATE, TIMEOUT, POLLING, RETRY_COUNT, packets, TOTAL_NO_OF_PACKETS, regs);



#ifdef __FACTORY_CONFIGURATION__


	
	for( i = 1 ; i < TRUCKS_SUPPORTED  ; i++)
	{
		for(j = 0 ; j < ACTIVITIES_SUPPORTED ; j++)
		{
		//	as = shipmentSchedule[i][j];
		//	scheduleTable[i][j] = as;
			if( j ==0 )
			{
				pickingStartTime[i] = as.startMinute;
			}
			ClrWdt();
		}
	}


	for( i = 0; i < BREAKS_SUPPORTED+1 ; i++)
	{
		as = breakSchedule[i];
		breaks[i] = as;

	}
	
	app.delayPercentage = DELAY_PERCENTAGE;
	app.alarmPercentage = ALARM_PERCENTAGE;

	
#else
	
	for( i = 0 ; i < TRUCKS_SUPPORTED + 1 ; i++)
	{
		for(j = 0 ; j < ACTIVITIES_SUPPORTED ; j++)
		{
			ReadBytesEEP(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + i*(sizeof(TRUCK_SCHEDULE))+ j*sizeof(ACTIVITY_SCHEDULE)
									, (UINT8*)&as,sizeof(ACTIVITY_SCHEDULE));

			if( j == 0 )
			{
				pickingStartTime[i] = as.startMinute;
			}
			ClrWdt();
		}
	}


	for( i = 0; i < BREAKS_SUPPORTED+1 ; i++)
	{
		
		ReadBytesEEP(EEP_BREAK_SCHEDULE_BASE_ADDRESS + i*(sizeof(ACTIVITY_SCHEDULE))
									, (UINT8*)&as,sizeof(ACTIVITY_SCHEDULE));
		breaks[i] = as;

	}

	app.delayPercentage = ReadByteEEP(EEP_DELAY_PERCENTAGE);
	app.alarmPercentage = ReadByteEEP(EEP_ALARM_PERCENTAGE);

#endif

	//set the value of CurrentActivitySegment structure parameter
	for(i= 0; i < ACTIVITIES_SUPPORTED; i++)
	{
		resetActivitySegment(i);
	}
	
	app.state = APP_STATE_ACTIVE;
	app.breakID = 0;
	app.secON = TRUE;

//	memset((UINT8*)scheduleTable,0,ACTIVITIES_SUPPORTED*TRUCKS_SUPPORTED);
	memset((UINT8*)&pickingInfo,0,sizeof(PICKING_INFO));
	
	activityStatus = RESET ;
	
	updateTime();
//	updateMarquee();
	updateBackLightIndication();

	resetSchedule();

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
	static UINT8 count = 0;

	status = MB_getStatus();

	updateTime();
	//Modubus Master 
	if( (status == PACKET_SENT) || (status == RETRIES_DONE) )
	{
		
		//check for new log entry, if yes write it to modbus			
		if(log.readIndex != log.writeIndex)
		{			
			MB_construct(&packets[PACKET1],log.slaveID[log.readIndex], PRESET_MULTIPLE_REGISTERS, 
 								STARTING_ADDRESS,log.regCount[log.readIndex], log.entries[log.readIndex]);	

			log.readIndex++;
		
			// check for the overflow
			if( log.readIndex >= MAX_LOG_ENTRIES )
				log.readIndex = 0;
								
	
		}
	}


//Modubus Slave Packet REceived
	if(app.DataReceived == TRUE)
	{
		ENABLE_UART_RX_INTERRUPT();

		processReceivedData();

	
		app.DataReceived = FALSE;	
	
	}

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
					updateMarquee();
					break;
				}
		
			}

			break;

		case APP_STATE_INACTIVE:

			if( app.curMinute >= breaks[app.breakID].endMinute)
			{
				app.breakID =0;
				app.state = APP_STATE_ACTIVE;
				updateMarquee();
				
				
			}
			break;

		default:
			return;
	}

	count++;
	
	if( count >= 30 )
	{
		if( app.curMinute != app.prevMinute)
		{
	
			if( updatePickingInfo() == TRUE )
			{
				updatePickingIndication();
			}
	
	
			updateCurrentActivityParameters();
			ClrWdt();
			updateCurrentActivityIndication();
			app.prevMinute = app.curMinute;
		}

		count = 0;
	}

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
	

		app.DataReceived = TRUE;

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
	UINT8 slaveID = 0;


	switch(cmd)
	{
		case CMD_PICKING_START:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_PICKING;
			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			milestone = MILESTONE_START ;
			updateTruckActivity(truck ,activity , milestone);
			
		break;

		case CMD_PICKING_END:
		
			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_PICKING;
			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			milestone = MILESTONE_END;
			updateTruckActivity(truck ,activity , milestone);			

		break;

		case CMD_STAGING_START:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_STAGING;
			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			milestone = MILESTONE_START ;
			updateTruckActivity(truck ,activity , milestone);

		break;

		case CMD_STAGING_END:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_STAGING;
			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			milestone = MILESTONE_END;
			updateTruckActivity(truck ,activity , milestone);

		break;

		case CMD_LOADING_START	:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_LOADING;
			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			milestone = MILESTONE_START ;
			updateTruckActivity(truck ,activity , milestone);			

		break;
		case CMD_LOADING_END:

			if( app.state == APP_STATE_INACTIVE )
				break;

			activity = ACTIVITY_LOADING;
			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			milestone = MILESTONE_END;
			updateTruckActivity(truck ,activity , milestone);			

		break;

		case CMD_TRUCK_TIMINGS	:

			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			slaveID = ((truck-1) /4);

			for(i = 0 ; i < 6 ; i++)
			{
				trucktime[i] = (UINT16)( ( (app.eMBdata[3 + (i * 4)] - '0' )* 10 ) + (app.eMBdata[4 + (i * 4)] - '0' ) ) * 60
				 + ( (app.eMBdata[5 + (i * 4)] - '0' )* 10 ) + (app.eMBdata[6 + (i * 4)] - '0' );

			}

			if( slaveID > 0)
			{
				for( i  = 0 ; i < 30 ; i++)
				{
					transmitTruncktime[i] = app.eMBdata[i]; 
				}
				transmitTruncktime[i] = '\0';
				updateLog_Binary (transmitTruncktime , slaveID,14 );
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
			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
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
	ACTIVITY_TRIGGER_DATA data ;

	data.truck = truck;
	data.activity = activity;
	data.mileStone = milestone;
	
	if( (data.activity == ACTIVITY_CANCEL) && (data.mileStone == MILESTONE_NONE) )
	{

		updateShipmentScheduleIndication(&data , 0);
		ClrWdt();
	}
	else
	{
		if( app.state == APP_STATE_INACTIVE )
			return;
		getActivitySchedule(data.truck ,data.activity, &as);
	
		if( processActivityTrigger(&data,as ) == TRUE)
		{
	
			updateCurrentActivityParameters();
			ClrWdt();

			updateShipmentScheduleIndication(&data,&as);
			ClrWdt();

			updateCurrentActivityIndication();
			ClrWdt();
	
			if( data.mileStone == MILESTONE_START)
			{
				updateAlarmIndication(data.activity);
				if( data.truck == pickingInfo.truck )
				{
					pickingInfo.truck = 0;
					pickingInfo.state = 0;
					pickingInfo.timeout = 0;		
					updatePickingIndication();	
				}
			}
				
			updateBackLightIndication();
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
* Input		: truck , 
*			  
*
 * Output	: None
*
*
*
*
*------------------------------------------------------------------------------
*/
void updateTruckTime(UINT8 truck , far UINT16* trucktime)
{
	UINT8 lock = 0;
	UINT8 i , j ,k;
	UINT16 timeStart,timeEnd ;
	UINT8* temp;
	temp = (UINT8*)trucktime;


	for(i = 0 ; i < 6 ; i++)
	{
		for(j = 0 ; j < 2 ; j++)
		{
			Write_b_eep(( (truck - 1) * 12) + ((2 * i ) + j)  , *(temp +((2 * i) + (1 - j) ) ) );	
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

		if(truck - 1 < TRUCKS_SUPPORTED_BOARD)
		{
			getScheduleTime(&shipmentSchedule[truck][i] ,activityTime);
			do
			{
				ENTER_CRITICAL_SECTION();
				//Aquire the lock
				lock = mutex_lock(  );
				EXIT_CRITICAL_SECTION();
			}while( lock == 0 );
			loadSchedule(truck,i+1);
			mutex_unlock(  );	
		}

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
void getActivitySchedule(UINT8 truck, ACTIVITY activity, far ACTIVITY_SCHEDULE* activitySchedule)
{

	*activitySchedule = shipmentSchedule[truck][activity-1];

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
BOOL processActivityTrigger( far ACTIVITY_TRIGGER_DATA* data, ACTIVITY_SCHEDULE as)
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
		
		if ( scheduleStatus[data->truck ][data->activity - 1].activityStatus != ACTIVITY_ONGOING) 	//if the particular activity has not started ignore
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
		scheduleStatus[data->truck][data->activity-1].activityStatus = ACTIVITY_COMPLETED;			//update scheduleTable

		result = TRUE;								
				
		break;

		case MILESTONE_START:
			if(currentActivitySegment[data->activity-1].free != TRUE )			// if activity segment not free ignore
					return FALSE;
	
			if( scheduleStatus[data->truck][data->activity - 1].activityStatus != ACTIVITY_SCHEDULED	)	
					return FALSE;

			loadActivityParameters(data->activity-1, &as,data );

			scheduleStatus[data->truck][data->activity-1].activityStatus = ACTIVITY_ONGOING;		//update scheduleTable 



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
void loadActivityParameters(UINT8 segment,far ACTIVITY_SCHEDULE* scheduleData,far ACTIVITY_TRIGGER_DATA*data )
{
	currentActivitySegment[segment].no = data->truck;
	currentActivitySegment[segment].activity = data->activity ;
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

#ifdef TIME_DEBUG
	hour = RTC_getHour();
	minute = RTC_getMinute();
	
	app.curMinute = hour*60 + minute;
	UTL_binaryToBCDASCII(hour , &time_backlight[TIME_HOUR_INDEX]);
	UTL_binaryToBCDASCII(minute , &time_backlight[TIME_MINUTE_INDEX]);
#else

	ReadRtcTimeAndDate(readTimeDateBuffer);	//Read RTC data and store it in buffer
	hour= (UINT16)readTimeDateBuffer[2];
	minute = (UINT16)readTimeDateBuffer[1];


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


	mmdConfig.startAddress =  TIME_SEGMENT_START_ADDRESS ;
	mmdConfig.length = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.symbolBuffer = time_backlight;
	mmdConfig.symbolCount = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;

	MMD_configSegment(1, &mmdConfig);



}


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

	if( app.breakID  == 0)
	{
		for( i = 0; i < MARQUEE_SEGMENT_LENGTH; i++)
		{
			marquee[i] = ' ';
		}
	}
	else
	{
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
	}

	mmdConfig.startAddress = MARQUEE_SEGMENT_START_ADDRESS;
	mmdConfig.length = MARQUEE_SEGMENT_LENGTH;
	mmdConfig.symbolBuffer = marquee;
	mmdConfig.symbolCount = i;
	mmdConfig.scrollSpeed = SCROLL_SPEED_HIGH;

	

	MMD_configSegment(0, &mmdConfig);

}
	

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
	UINT8 length;


	if( app.state == APP_STATE_ACTIVE )
		i = 0;
	else i = 2;


	for(; i < ACTIVITIES_SUPPORTED; i++)
	{		
		if( currentActivitySegment[i].free == TRUE)	
		{
			j = 0;
			activityParameterBuffer[j++] = CMD_CLEAR_SEGMENT;	
		}
		else
		{
			j = 0;
			activityParameterBuffer[j++] = CMD_SET_SEGMENT ;
			activityParameterBuffer[j++] = currentActivitySegment[i].no;
			activityParameterBuffer[j++] = currentActivitySegment[i].activity;
			activityParameterBuffer[j++] = currentActivitySegment[i].status;
			activityParameterBuffer[j++] = currentActivitySegment[i].planProgress;
	
			temp = (currentActivitySegment[i].planPercentage/100);
			activityParameterBuffer[j++] = (temp >= 99) ? (99) : temp;
	
			activityParameterBuffer[j++] = currentActivitySegment[i].actualProgress;
	
			temp = currentActivitySegment[i].actualPercentage/100;
			activityParameterBuffer[j++] = ( temp >= 99 ) ? (99) : temp;
		}
		length = (j%2 == 0 ? j/2 : j/2 + 1 );
		updateLog_Binary(activityParameterBuffer,i+1 , length);
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

void updateShipmentScheduleIndication(far ACTIVITY_TRIGGER_DATA *data,far 	ACTIVITY_SCHEDULE *as)
{
	UINT8 i = 0;
	UINT8 deviceAddress;
	UINT8 cmd;
	UINT8 length;
	if ( data->truck > 4 )
	{
		activityParameterBuffer[i++] = CMD_UPDATE_SHIPMENT_SCHEDULE;
	}
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

	if ( (data->truck) <= 4 )
	{
		updateSchedule(activityParameterBuffer);
	}
	else
	{
		length = (i% 2 == 0 ? i/2 : i/2 + 1 );
		deviceAddress = ( ((data->truck) - 1 ) /4);
		updateLog_Binary(activityParameterBuffer,deviceAddress,length);
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

void updateSchedule(far UINT8 *data)
{
	UINT8 i;
	UINT8 lock = 0;
	UINT8 truck,truckStatusIndex ,index;
	SCHEDULE_UPDATE_INFO *info; 
	UINT8 activityCompleteFlag = TRUE;
	INT8 delayedActivity = 0xFF;
	info = (SCHEDULE_UPDATE_INFO*) data;
	truck = info->truck ;
	truckStatusIndex =  (truck - 1) * 2;
	index = 0;

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

		truckStatus[index] = DIGIT_DASH;
		truckStatus[index + 1] = DIGIT_DASH;

		DigitDisplay_updateBufferBinaryPartial(truckStatus, TRUCK_STATUS_BASE + truckStatusIndex, 2);


	}

	else
	{
		switch( info->milestone)
		{
			case MILESTONE_START:


			//store the status of the truck
			truckStatus[index] = DIGIT_A;
			truckStatus[index + 1] = DIGIT_SPACE;

	
			//update it into display buffer
			DigitDisplay_updateBufferBinaryPartial(truckStatus, TRUCK_STATUS_BASE + truckStatusIndex, 2);

			getScheduleTime(&shipmentSchedule[truck][info->activity-1] , activityTime);	
						
			scheduleStatus[truck][info->activity - 1].activityStatus = ACTIVITY_ONGOING;
			scheduleStatus[truck][info->activity - 1].status = info->status;
			break;

			case MILESTONE_END:

			clearScheduleTime();

			do
			{
				ENTER_CRITICAL_SECTION();
				//Aquire the lock
				lock = mutex_lock(  );
				EXIT_CRITICAL_SECTION();
			}while( lock == 0 );
			loadSchedule(truck,info->activity);
			//release the lock
			mutex_unlock(  );				

			scheduleStatus[truck][info->activity - 1].activityStatus = ACTIVITY_COMPLETED;
			scheduleStatus[truck][info->activity - 1].status = info->status;

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

			if( delayedActivity < ACTIVITIES_SUPPORTED )
			{
					//store the status of the truck
				truckStatus[index] = DIGIT_C;
				switch(delayedActivity)
				{
					case 0 : //delayed due to picking
						truckStatus[index + 1] = DIGIT_P;
					break;
					case 1 : //delayed due to staging
						truckStatus[index + 1] = 5;//'S' in seven segment
					break;

					case 2 : //delayed due to loading
						truckStatus[index + 1] = DIGIT_L;
					break;
				}
				
	
			}
			else
			{
					//store the status of the truck
				truckStatus[index] = DIGIT_C;
				truckStatus[index + 1] = DIGIT_SPACE;
			}
			DigitDisplay_updateBufferBinaryPartial(truckStatus, TRUCK_STATUS_BASE + truckStatusIndex, 2);	
			
			break;

			default:
			break;
		}

		
	}


//	loadSchedule(truck,info->activity);
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

void resetSchedule(void)
{
	UINT8 i,j;
	UINT8 lock = 0;

	//buffer used to store truck number
	for( i = 0; i < TRUCKS_SUPPORTED_BOARD ; i++ )
	{

		truckNo[2 * i] = (i+1)/10;
		truckNo[ (2 *i) + 1] = (i+1) % 10;
	}

	DigitDisplay_updateBufferBinaryPartial(truckNo,0,TRUCKS_SUPPORTED_BOARD*2);

	for( i = 1 ; i < TRUCKS_SUPPORTED_BOARD+1 ; i++)
	{
		for( j= 0; j < ACTIVITIES_SUPPORTED ; j++)
		{
			
			scheduleStatus[i][j].activityStatus = ACTIVITY_SCHEDULED;
			scheduleStatus[i][j].status = ACTIVITY_NONE;
			
			getScheduleTime(&shipmentSchedule[i][j] ,activityTime);

			do
			{
				ENTER_CRITICAL_SECTION();
				//Aquire the lock
				lock = mutex_lock(  );
				EXIT_CRITICAL_SECTION();
			}while( lock == 0 );	
			loadSchedule(i,j+1);
			//release the lock
			mutex_unlock(  );	
	
		}	
	}
	
}

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
		DDR_loadDigit( ( (32 + ((truck - 1) * 24) ) + (activity - 1 ) * 8) + i,activityTime[i] );
		DelayMs(1);
	}

}

/*
*------------------------------------------------------------------------------
* void getScheduleTime(ACTIVITY_SCHEDULE* as , UINT8* activityTime)
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/


void getScheduleTime(far ACTIVITY_SCHEDULE* as ,far  UINT8* activityTime)
{
	UINT16 hour;
	UINT16 minute;
		
	hour = as->startMinute / 60 ;
	minute = (as->startMinute - (UINT16)(hour * 60 ) ) % 60;

	activityTime[0] = hour /10  ;
	activityTime[1] = hour % 10;

	activityTime[2] = minute/10;
	activityTime[3] = minute%10;

	hour = as->endMinute / 60 ;
	minute = (as->endMinute - (UINT16)(hour * 60 ) ) % 60;

	activityTime[4] = hour /10  ;
	activityTime[5] = hour % 10;

	activityTime[6] = minute/10;
	activityTime[7] = minute%10;
}

/*
*------------------------------------------------------------------------------
* void clearScheduleTime(void )
*
* Summary	: 
*
* Input		: 
*			  
*
* Output	: None
*------------------------------------------------------------------------------
*/
void clearScheduleTime()
{
	UINT8 i;
	for( i = 0; i < 8;i++)
	{
		activityTime[i] = DIGIT_CLEAR;
	}
}


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



}




/*---------------------------------------------------------------------------------------------------------------
*void updateLog(far UINT8 *data,UINT8 slave)
*----------------------------------------------------------------------------------------------------------------
*/
void updateLog(far UINT8 *data,UINT8 slave)
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
	log.regCount[log.writeIndex] = i;

	log.slaveID[log.writeIndex] = slave;

	log.writeIndex++;
	if( log.writeIndex >= MAX_LOG_ENTRIES)
		log.writeIndex = 0;
}


void updateLog_Binary(far UINT8 *data,UINT8 slave,UINT8 length)
{
	UINT8 i = 0;
	while(length-- > 0)
	{
		log.entries[log.writeIndex][i] = (UINT16)*data << 8;
		data++;
		log.entries[log.writeIndex][i] |= (UINT16)*data;
		data++;
		i++;
	}
	log.regCount[log.writeIndex] = i;

	log.slaveID[log.writeIndex] = slave;

	log.writeIndex++;
	if( log.writeIndex >= MAX_LOG_ENTRIES)
		log.writeIndex = 0;
}


void COM_txBuffer(UINT8 *txData, UINT8 length)
{
	UINT8 i;

	for( i = 0 ; i < length ; i++)	//store data
	{
		UART1_write (*txData);
		txData++;
	}

	UART1_transmit();

}

/*---------------------------------------------------------------------------------------------------------------
* void APP_writeModbus( void )
*----------------------------------------------------------------------------------------------------------------
*/

void APP_writeModbus( void )
{
//	status = MB_getStatus();

	//Modubus Master 
//	if( (status == PACKET_SENT) || (status == RETRIES_DONE) )
	{
		
		//check for new log entry, if yes write it to modbus			
		if(log.readIndex != log.writeIndex)
		{			
			MB_construct(&packets[PACKET1],log.slaveID[log.readIndex], PRESET_MULTIPLE_REGISTERS, 
 								STARTING_ADDRESS,log.regCount[log.readIndex], log.entries[log.readIndex]);	

			log.readIndex++;
		
			// check for the overflow
			if( log.readIndex >= MAX_LOG_ENTRIES )
				log.readIndex = 0;
								
	
		}
	}
}

