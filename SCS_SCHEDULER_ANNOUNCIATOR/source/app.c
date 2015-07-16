
#include "mmd.h"
#include "string.h"
#include "app.h"
#include "typedefs.h"
#include "eep.h"
#include "mb.h"
#include "digit_driver.h"
#include "utilities.h"
#include "digitdisplay.h"

/*
*-----------------------------------------------------------
* shipment scheduler
*------------------------------------------------------------
*/

typedef struct _TRUCK_SCHEDULE
{
	ACTIVITY_SCHEDULE schedule[ACTIVITIES_SUPPORTED];
}TRUCK_SCHEDULE;

typedef struct _SCHEDULE_UPDATE_INFO
{
	UINT8 truck;
	ACTIVITY activity;
	MILESTONE milestone;
	STATUS status;
	UINT8 startMinute_MSB;
	UINT8 startMinute_LSB;
	UINT8 endMinute_MSB;
	UINT8 endMinute_LSB;
	UINT8 curMinute_MSB;
	UINT8 curMinute_LSB;
	UINT8 truckStatus[2];
}SCHEDULE_UPDATE_INFO;
 

typedef struct _SCHEDULE_DATA
{
	UINT8 truck;
	ACTIVITY_SCHEDULE schedule[ACTIVITIES_SUPPORTED];
}SCHEDULE_DATA;

typedef struct _SCHEDULE_STATUS
{
	ACTIVITY_STATUS activityStatus;
	STATUS status;
}SCHEDULE_STATUS;

typedef struct _TRUCK_INDICATOR_DATA
{
	UINT8 indicatorRed[4];
	UINT8 indicatorGreen[4];
}TRUCK_INDICATOR_DATA;



typedef struct _APP
{
	APP_STATE state;
	UINT16 curMinute;
	UINT16  prevMinute;
	UINT8 breakID;
	UINT8 delayPercentage;
	UINT8 hooterOnPercentage;
	UINT8 secON;
	//Modbus buffer
	UINT8 eMBdata[MAX_SIZE];
	BOOL DataReceived;
}APP;


/*
*-----------------------------------------------------------
* shipment announciator
*------------------------------------------------------------
*/
typedef struct _SEGMENT_DATA
{
	UINT8 truck;
	ACTIVITY activity;
	STATUS status;
	UINT8 planProgress;
	UINT8 planPercentage;
	UINT8 actualProgress;
	UINT8 actualPercentage;
}SEGMENT_DATA;

const rom UINT8 ACTIVITY_DATA[ACTIVITIES_SUPPORTED+1][PARAMETER_ACTIVITY_LENGTH+1] 
={
{" "},
{"PICKING "},
{"STAGING "},
{"LOADING "}
};

const rom UINT8 STATUS_DATA[3][2]
={ 
{' ',' '},
{SYM_ALL,' '},
{' ',SYM_ALL}
};


const rom UINT8 PROGRESS_DATA[37][6]
={ 
{' ',' ',' ', ' ',' ',' '},
{SYM_7_COL,' ',' ', ' ',' ',' '},
{SYM_76_COL,' ',' ', ' ',' ',' '},
{SYM_765_COL,' ',' ', ' ',' ',' '},
{SYM_7654_COL,' ',' ', ' ',' ',' '},
{SYM_76543_COL,' ',' ', ' ',' ',' '},
{SYM_ALL,' ',' ', ' ',' ',' '},

{SYM_ALL,SYM_7_COL, ' ',' ',' ',' '},
{SYM_ALL,SYM_76_COL,' ', ' ',' ',' '},
{SYM_ALL,SYM_765_COL,' ', ' ',' ',' '},
{SYM_ALL,SYM_7654_COL,' ', ' ',' ',' '},
{SYM_ALL,SYM_76543_COL,' ', ' ',' ',' '},
{SYM_ALL,SYM_ALL,' ', ' ',' ',' '},

{SYM_ALL,SYM_ALL,SYM_7_COL,' ',' ',' '},
{SYM_ALL,SYM_ALL,SYM_76_COL, ' ',' ',' '},
{SYM_ALL,SYM_ALL,SYM_765_COL, ' ',' ',' '},
{SYM_ALL,SYM_ALL,SYM_7654_COL, ' ',' ',' '},
{SYM_ALL,SYM_ALL,SYM_76543_COL, ' ',' ',' '},
{SYM_ALL,SYM_ALL,SYM_ALL,' ',' ',' '},

{SYM_ALL,SYM_ALL,SYM_ALL,SYM_7_COL,' ',' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_76_COL, ' ',' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_765_COL, ' ',' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_7654_COL, ' ',' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_76543_COL, ' ',' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,' ',' '},

{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_7_COL,' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_76_COL, ' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_765_COL, ' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_7654_COL, ' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_76543_COL, ' '},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,' '},

{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_7_COL},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_76_COL},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_765_COL},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_7654_COL},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_76543_COL},
{SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL,SYM_ALL}

};
	

/*
*-----------------------------------------------------------
* shipment scheduler
*------------------------------------------------------------
*/


#pragma idata app_data

APP app = {0} ;
UINT8 segmentBuffer[28];

UINT8 activityParameterBuffer[ ACTIVITY_PARAMETER_BUFFER_SIZE] = {0};

MMD_Config mmdConfig= {0};
UINT8 line[10] ="LINE "; 

ACTIVITY_SCHEDULE scheduleTable[TRUCKS_SUPPORTED+1][ACTIVITIES_SUPPORTED] = {0};

SCHEDULE_STATUS scheduleStatus[TRUCKS_SUPPORTED+1][ACTIVITIES_SUPPORTED] = {0};

UINT8 truckNo[TRUCKS_SUPPORTED * 2] = {0};		//buffer for truck nos
UINT8 truckStatus[2] = {0};	//buffer for truck status
UINT8 activityTime[8 ]= {0};


ACTIVITY_SCHEDULE shipmentSchedule[TRUCKS_SUPPORTED+1][ACTIVITIES_SUPPORTED]
={
{{0, 0,0},{0 , 0,0},{0 , 0, 0}},
#if DEVICE_ADDRESS==1
{{(UINT16)540 , (UINT16)610 ,(UINT16)70},{(UINT16)550 , (UINT16)620 ,(UINT16)70},{(UINT16)630 ,(UINT16)665 ,(UINT16)35}},
{{(UINT16)610 , (UINT16)670 ,(UINT16)60},{(UINT16)620 , (UINT16)680 ,(UINT16)60},{(UINT16)685 ,(UINT16)720 ,(UINT16)35}},
{{(UINT16)670 , (UINT16)760 ,(UINT16)90},{(UINT16)680 , (UINT16)770 ,(UINT16)90},{(UINT16)770 ,(UINT16)805 ,(UINT16)35}},
{{(UINT16)760 , (UINT16)820 ,(UINT16)60},{(UINT16)770 , (UINT16)830 ,(UINT16)60},{(UINT16)835 ,(UINT16)870 ,(UINT16)35}},

#elif DEVICE_ADDRESS==2
{{(UINT16)820 , (UINT16)880 ,(UINT16)60},{(UINT16)830 , (UINT16)890 ,(UINT16)60},{(UINT16)905 ,(UINT16)940 ,(UINT16)35}},
{{(UINT16)880 , (UINT16)950 ,(UINT16)70},{(UINT16)890 , (UINT16)960 ,(UINT16)70},{(UINT16)965 ,(UINT16)1000 ,(UINT16)35}},
{{(UINT16)950 , (UINT16)1020 ,(UINT16)70},{(UINT16)960 , (UINT16)1030 ,(UINT16)70},{(UINT16)1030 ,(UINT16)1065,(UINT16)35}},
{{(UINT16)1020 , (UINT16)1080,(UINT16)60},{(UINT16)1030 , (UINT16)1090,(UINT16)60},{(UINT16)1105,(UINT16)1140,(UINT16)35}},

#elif DEVICE_ADDRESS==3
{{(UINT16)1080, (UINT16)1140,(UINT16)60},{(UINT16)1090, (UINT16)1150,(UINT16)60},{(UINT16)1175,(UINT16)1210,(UINT16)35}},
{{(UINT16)1140, (UINT16)1220,(UINT16)80},{(UINT16)1150, (UINT16)1230,(UINT16)80},{(UINT16)1235,(UINT16)1270,(UINT16)35}},
{{(UINT16)1220, (UINT16)1280,(UINT16)60},{(UINT16)1230, (UINT16)1290,(UINT16)60},{(UINT16)1305,(UINT16)1340,(UINT16)35}},
{{(UINT16)1280, (UINT16)1340,(UINT16)60},{(UINT16)1290, (UINT16)1350,(UINT16)60},{(UINT16)1365,(UINT16)1400,(UINT16)35}},

#endif
};

#pragma idata



void resetSegment(void);

void copySrcToDst(const rom UINT8*src, UINT8* dst , UINT8 length);

void resetSchedule( void);
void getSchedule(UINT8 truck, ACTIVITY activity, far ACTIVITY_SCHEDULE* activitySchedule);

void loadSchedule(UINT8 truck, UINT8 activity);

void getScheduleTime(far ACTIVITY_SCHEDULE* as , far UINT8* activityTime);
void setSchedule(far SCHEDULE_DATA *data);

void updateSchedule(far SCHEDULE_UPDATE_INFO *info);

void clearScheduleTime(void);

//used to update truck number
void displayTruckNumber(far UINT8* buffer);

void processMBdata(void);

//function used to update the truck timings in the array of the structure
void updateTruckTime(UINT8 truck , UINT8* trucktime); 

void getActivitySchedule(UINT8 truck, ACTIVITY activity,far ACTIVITY_SCHEDULE* activitySchedule);




/*
*------------------------------------------------------------------------------
* APP init
*------------------------------------------------------------------------------
*/

void APP_init(void)
{
	UINT8 i = 0,j, k;
	UINT16 timeStart, timeEnd;
	UINT8 truck;
	UINT8 buffer[4];


#ifndef __FACTORY_CONFIGURATION__
	//load truck timings from EEPROM into the shipment schedule array of struct
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
#endif

	for( i = 1 ; i < TRUCKS_SUPPORTED + 1 ; i++)
	{
		for(j = 0 ; j < ACTIVITIES_SUPPORTED ; j++)
		{
			getActivitySchedule(i , j, &scheduleTable[i][j]);
		}
	}
	
/*	
	mmdConfig.startAddress = 0;
	mmdConfig.length = 0;
	mmdConfig.symbolBuffer = 0;
	mmdConfig.symbolCount = 0;
	mmdConfig.scrollSpeed = 0;


*/	
	resetSchedule();
	

	resetSegment();
}


/*
*------------------------------------------------------------------------------
* APP TASK
*------------------------------------------------------------------------------
*/

void APP_task(void)
{

	UINT8 i;
 	DISABLE_UART_RX_INTERRUPT();
	if(app.DataReceived == TRUE )
	{
		ENABLE_UART_RX_INTERRUPT();


		processMBdata();

		DISABLE_UART_RX_INTERRUPT();
		app.DataReceived = FALSE;
		ENABLE_UART_RX_INTERRUPT();

	}
	
	ENABLE_UART_RX_INTERRUPT();


}

/*
*------------------------------------------------------------------------------
* Function to process the data received form modbus
*------------------------------------------------------------------------------
*/

void processMBdata(void)
{
	UINT8 cmd = app.eMBdata[0];
	UINT8 truck;
	const rom UINT8* pData;
	UINT8 i;
	UINT16 trucktime[6];

	switch(cmd)
	{
		case CMD_GET_COMM_STATUS:
		{
			SCHEDULE_UPDATE_INFO *data = (SCHEDULE_UPDATE_INFO*) (((UINT8*)app.eMBdata+1) );
			if( (data->truck <= (DEVICE_ADDRESS-1)*4 ) ||(data->truck > (DEVICE_ADDRESS)*4 ))
				return;
			
			if(data->activity == ACTIVITY_NONE)
				return;
			

			else
			{
				updateSchedule(data);
			}
		}
			
		break;

		case CMD_RESET:
		break;

		case CMD_SET_SEGMENT:
		{
			SEGMENT_DATA *data = (SCHEDULE_UPDATE_INFO*) (((UINT8*)&app.eMBdata+1));

			UTL_binaryToBCDASCII( data->truck , &segmentBuffer[PARAMETER_TRUCK_INDEX] );

			pData = ACTIVITY_DATA[data->activity];
			copySrcToDst(pData, &segmentBuffer[ PARAMETER_ACTIVITY_INDEX], PARAMETER_ACTIVITY_LENGTH);

			pData = STATUS_DATA[data->status];
			copySrcToDst(pData, &segmentBuffer[ PARAMETER_STATUS_INDEX], PARAMETER_STATUS_LENGTH);

			pData = PROGRESS_DATA[data->planProgress];
			copySrcToDst(pData, &segmentBuffer[ PARAMETER_PLAN_PROGRESS_INDEX], PARAMETER_PLAN_PROGRESS_LENGTH);

			UTL_binaryToBCDASCII( data->planPercentage , &segmentBuffer[PARAMETER_PLAN_PERCENTAGE_INDEX] );

			pData = PROGRESS_DATA[data->actualProgress];
			copySrcToDst(pData, &segmentBuffer[ PARAMETER_ACTUAL_PROGRESS_INDEX], PARAMETER_ACTUAL_PROGRESS_LENGTH);			

			UTL_binaryToBCDASCII( data->actualPercentage , &segmentBuffer[PARAMETER_ACTUAL_PERCENTAGE_INDEX] );

			mmdConfig.startAddress = 0;
			mmdConfig.length = 28;
			mmdConfig.symbolCount = 28;
			mmdConfig.symbolBuffer = segmentBuffer;
			mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;
			
			MMD_configSegment( 0 , &mmdConfig);
			break;	
		}

		case CMD_CLEAR_SEGMENT:
			resetSegment();
		break;

		case CMD_TRUCK_TIMINGS	:

			truck = ( (app.eMBdata[1] - '0' )* 10 ) + (app.eMBdata[2] - '0' );
			truck = (truck) - (DEVICE_ADDRESS)*4 + 1;
			for(i = 0 ; i < 6 ; i++)
			{
				trucktime[i] = (UINT16)( ( (app.eMBdata[3 + (i * 4)] - '0' )* 10 ) + (app.eMBdata[4 + (i * 4)] - '0' ) ) * 60
				 + ( (app.eMBdata[5 + (i * 4)] - '0' )* 10 ) + (app.eMBdata[6 + (i * 4)] - '0' );

			}

			updateTruckTime( truck , trucktime);

		break;


		case CMD_UPDATE_SHIPMENT_SCHEDULE:
		{
			SCHEDULE_UPDATE_INFO *data = (SCHEDULE_UPDATE_INFO*) ((UINT8*)&app.eMBdata[1]);
			if( (data->truck <= (DEVICE_ADDRESS*4 +1) ) ||(data->truck > (DEVICE_ADDRESS+1)*4 ))
				break;	
			
			if(data->activity == ACTIVITY_NONE)
				break;
			

			else
			{
				updateSchedule(data);
			}

		}
		break;

		default:
		break;
	}
}



void updateSchedule(far SCHEDULE_UPDATE_INFO *info)
{
	UINT8 i;
	UINT8 truck,truckStatusIndex,index;
	UINT8 activityCompleteFlag = TRUE;
	INT8 delayedActivity = 0xFF;

	truck = info->truck - (DEVICE_ADDRESS * 4);
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

		clearScheduleTime();
		loadSchedule(truck,info->activity);
	}

	else
	{
		switch( info->milestone)
		{
			case MILESTONE_START:
			if( scheduleStatus[truck][info->activity - 1].activityStatus != ACTIVITY_SCHEDULED)				//if activity is not scheduled ignore cmd
				return ;


			//store the status of the truck
			truckStatus[index] = DIGIT_A;
			truckStatus[index + 1] = DIGIT_SPACE;
	
			//update it into display buffer
			DigitDisplay_updateBufferBinaryPartial(truckStatus, TRUCK_STATUS_BASE + truckStatusIndex, 2);

			getScheduleTime(&scheduleTable[truck][info->activity-1] , activityTime);
			
						
			scheduleStatus[truck][info->activity - 1].activityStatus = ACTIVITY_ONGOING;
			scheduleStatus[truck][info->activity - 1].status = info->status;
			break;

			case MILESTONE_END:
			if( scheduleStatus[truck][info->activity - 1].activityStatus != ACTIVITY_ONGOING)				//if activity is not scheduled ignore cmd
				return ;


			
	
			
			clearScheduleTime();
			loadSchedule(truck,info->activity);

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

			if( i < ACTIVITIES_SUPPORTED )
			{
				//store the status of the truck
				truckStatus[index] = DIGIT_C;
				switch(i)
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

			//update it into display buffer
			DigitDisplay_updateBufferBinaryPartial(truckStatus, TRUCK_STATUS_BASE + truckStatusIndex, 2);	
		
			
			break;

			default:
			break;
		}

		
	}



	loadSchedule(truck,info->activity);
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

    int             iRegIndex;
/*
    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0         {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
 
*/
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
* RESET SEGMENT
*------------------------------------------------------------------------------
*/
void resetSegment()
{
	UINT8 i;

	for(i = 0;i < 28 ; i++)
	{
		segmentBuffer[i] = ' ';
	}

	mmdConfig.startAddress = 0;
	mmdConfig.length = MMD_MAX_CHARS;//28;
	mmdConfig.symbolCount = MMD_MAX_CHARS;//28;
	mmdConfig.symbolBuffer = segmentBuffer;
	mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;
	
	MMD_configSegment( 0 , &mmdConfig);
}


void copySrcToDst(const rom UINT8*src, UINT8* dst , UINT8 length)
{
	UINT8 i;
	for( i = 0 ; i < length; i++)
	{
		dst[i] = src[i];
	}
}

/*
*------------------------------------------------------------------------------
* Function used to update truck number
*------------------------------------------------------------------------------
*/
void resetSchedule( void )
{
	UINT8 i,j;

	
	//buffer used to store truck number
	for( i = 0; i < TRUCKS_SUPPORTED ; i++ )
	{

		truckNo[2 * i] = ((DEVICE_ADDRESS*4)+i+1)/10;
		truckNo[(2 *i) + 1] = ((DEVICE_ADDRESS*4)+i+1) % 10;
	}

	DigitDisplay_updateBufferBinaryPartial(truckNo,0,TRUCKS_SUPPORTED*2);


	for( i = 1 ; i < TRUCKS_SUPPORTED+1 ; i++)
	{
		for( j= 0; j < ACTIVITIES_SUPPORTED ; j++)
		{
			
			scheduleStatus[i][j].activityStatus = ACTIVITY_SCHEDULED;
			scheduleStatus[i][j].status = ACTIVITY_NONE;
			
			getScheduleTime(&scheduleTable[i][j] ,activityTime);
	
			loadSchedule(i,j+1);
	
		}	
	}
	
}


void loadSchedule(UINT8 truck, UINT8 activity)
{
	UINT8 i;
	for(i = 0; i < 8 ;i++)
	{
		DDR_loadDigit( ( (32 + ((truck - 1) * 24) ) + (activity - 1 ) * 8) + i,activityTime[i] );
		DelayMs(1);
	}
}

void clearScheduleTime()
{
	UINT8 i;
	for( i = 0; i < 8;i++)
	{
		activityTime[i] = DIGIT_CLEAR;
	}
}

void getScheduleTime(far ACTIVITY_SCHEDULE* as ,far UINT8* activityTime)
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
void updateTruckTime(UINT8 truck ,far UINT8* trucktime)
{
	UINT8 i , j ,k;
	UINT16 timeStart,timeEnd ;

	for(i = 0 ; i < 6 ; i++)
	{
		for(j = 0 ; j < 2 ; j++)
		{
			Write_b_eep(( (truck) * 12) + ((2 * i ) + j)  , *(trucktime +((2 * i) + (1 - j) ) ) );	
			Busy_eep();
		}
	}

	for(i = 0 ; i < 3 ; i++)
	{
		for(j = 0 , k  = 0 ; j < 2 ; j++ , k++)
		{
			timeStart <<= 8 ;
			timeStart |=	Read_b_eep(( (truck) * 12) + ((4 * i ) + j));	
			Busy_eep();
			timeEnd <<= 8 ;
			timeEnd |=	Read_b_eep(( (truck) * 12) + (((4 * i ) + j) +2));	
			Busy_eep();

		}
		shipmentSchedule[truck+1 ][i].startMinute = timeStart ;
		shipmentSchedule[truck+1 ][i].endMinute = timeEnd ;
		shipmentSchedule[truck+1 ][i].duration = timeEnd - timeStart;

	}

}

void getActivitySchedule(UINT8 truck, ACTIVITY activity,far ACTIVITY_SCHEDULE* activitySchedule)
{

	*activitySchedule = shipmentSchedule[truck][activity];

}