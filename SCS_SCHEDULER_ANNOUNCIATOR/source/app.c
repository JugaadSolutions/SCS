#include "app.h"
#include "mmd.h"
#include "eeprom_interface.h"
#include "utilities.h"
#include "mb.h"
#include "digit_driver.h"
#include <string.h>
#include "eep.h"
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
	BOOL MBdataReceived;
}APP;

typedef struct _DISPLAY_SCANNING
{
	//stores ascii of truck number
	UINT8 truckNoBuff[TRUCKS_SUPPORTED*2];
	
	//used to store status of truck
	UINT8 truckStatus[TRUCKS_SUPPORTED*2];
}DISPLAY_SCANNING;

static rom ACTIVITY_SCHEDULE shipmentSchedule[TRUCKS_SUPPORTED*4+1][ACTIVITIES_SUPPORTED]
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


const rom TRUCK_INDICATOR_DATA truckIndicators[TRUCKS_SUPPORTED*4 + 1]={
{{' ',' ',' ',' '},{' ',' ',' ',' '}},
{{'0',' ','1',' '},{' ','0',' ','1'}},
{{'0',' ','2',' '},{' ','0',' ','2'}},
{{'0',' ','3',' '},{' ','0',' ','3'}},
{{'0',' ','4',' '},{' ','0',' ','4'}},
{{'0',' ','5',' '},{' ','0',' ','5'}},
{{'0',' ','6',' '},{' ','0',' ','6'}},
{{'0',' ','7',' '},{' ','0',' ','7'}},

{{'0',' ','8',' '},{' ','0',' ','8'}},
{{'0',' ','9',' '},{' ','0',' ','9'}},
{{'1',' ','0',' '},{' ','1',' ','0'}},
{{'1',' ','1',' '},{' ','1',' ','1'}},
{{'1',' ','2',' '},{' ','1',' ','2'}},
{{'1',' ','3',' '},{' ','1',' ','3'}},
{{'1',' ','4',' '},{' ','1',' ','4'}},
{{'1',' ','5',' '},{' ','1',' ','5'}},
{{'1',' ','6',' '},{' ','1',' ','6'}},
};


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

UINT8 truck_statusIndicator[TRUCKS_SUPPORTED+1][8] = {0};
UINT8 activityTime[8 ]= {0};

#pragma idata



void resetSegment(void);

void copySrcToDst(const rom UINT8*src, UINT8* dst , UINT8 length);

void resetSchedule(UINT8 i);
void getSchedule(UINT8 truck, ACTIVITY activity, ACTIVITY_SCHEDULE* activitySchedule);

void loadSchedule(UINT8 truck, UINT8 activity);

void getScheduleTime(ACTIVITY_SCHEDULE* as , UINT8* activityTime);
void setSchedule(SCHEDULE_DATA *data);

void updateSchedule(SCHEDULE_UPDATE_INFO *info, UINT8 command);

void resetSchedule(UINT8 truck);

void clearScheduleTime(void);

//used to update truck number
void displayTruckNumber(UINT8* buffer);

void APP_init(void)
{
	UINT8 i = 0,j, k;

	eMBErrorCode    eStatus;

	UINT8 buffer[4];


#ifdef __FACTORY_CONFIGURATION__

	ACTIVITY_SCHEDULE as;
	void *ptr = &as;

	for( i = 1 ; i < TRUCKS_SUPPORTED + 1 ; i++)
	{
		for(j = 0 ; j < ACTIVITIES_SUPPORTED ; j++)
		{
			as = shipmentSchedule[(i+((DEVICE_ADDRESS-1)*4))][j];
			scheduleTable[i][j] = as;
			for( i = 0; i < sizeof(ACTIVITY_SCHEDULE); i++)
			{
				Write_b_eep(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + i*(sizeof(TRUCK_SCHEDULE))+ j*sizeof(ACTIVITY_SCHEDULE), *(UINT8*)(ptr+i) );
				Busy_eep();
			}
//			WriteBytesEEP(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + i*(sizeof(TRUCK_SCHEDULE))+ j*sizeof(ACTIVITY_SCHEDULE)
//									, (UINT8*)&as,sizeof(ACTIVITY_SCHEDULE));
			ClrWdt();
		}
	}

#else

	for( i = 1 ; i < TRUCKS_SUPPORTED + 1 ; i++)
	{
		for(j = 0 ; j < ACTIVITIES_SUPPORTED ; j++)
		{
			getActivitySchedule(i + ((DEVICE_ADDRESS -1) *4), j, &scheduleTable[i][j]);
		}
	}


#endif


	//modbus configuration
	eStatus = eMBInit( MB_RTU, ( UCHAR )DEVICE_ADDRESS, 0, UART1_BAUD, MB_PAR_NONE);
	eStatus = eMBEnable(  );	/* Enable the Modbus Protocol Stack. */

	//buffer used to store truck number
	for( i = 0; i < 4; i++ )
		buffer[i] = ((((DEVICE_ADDRESS-1)*4)+i) + 1);

	//update the truck number 
	displayTruckNumber(buffer);

	
	
	mmdConfig.startAddress = 0;
	mmdConfig.length = 0;
	mmdConfig.symbolBuffer = 0;
	mmdConfig.symbolCount = 0;
	mmdConfig.scrollSpeed = 0;

/*
	MMD_clearSegment(0);
	mmdConfig.startAddress = 0;
	mmdConfig.length = 6;
	mmdConfig.symbolCount = 5;
	mmdConfig.symbolBuffer = line;
	mmdConfig.scrollSpeed = SCROLL_SPEED_LOW;
			
	MMD_configSegment( 0 , &mmdConfig);

	MMD_clearSegment(1);
	mmdConfig.startAddress = 6;
	mmdConfig.length = 6;
	mmdConfig.symbolCount = 5;
	mmdConfig.symbolBuffer = line;
	mmdConfig.scrollSpeed = SCROLL_SPEED_LOW;
			
	MMD_configSegment( 1 , &mmdConfig);

	MMD_clearSegment(2);
	mmdConfig.startAddress = 12;
	mmdConfig.length = 6;
	mmdConfig.symbolCount = 5;
	mmdConfig.symbolBuffer = line;
	mmdConfig.scrollSpeed = SCROLL_SPEED_LOW;
			
	MMD_configSegment( 2 , &mmdConfig);

	MMD_clearSegment(3);
	mmdConfig.startAddress = 18;
	mmdConfig.length = 6;
	mmdConfig.symbolCount = 5;
	mmdConfig.symbolBuffer = line;
	mmdConfig.scrollSpeed = SCROLL_SPEED_LOW;
			
	MMD_configSegment( 3 , &mmdConfig);
*/

	for(i= 1; i < TRUCKS_SUPPORTED+1 ; i++)
	{
		resetSchedule(i);
	}	

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
 	DISABLE_UART1_RX_INTERRUPT();
	if(app.MBdataReceived == TRUE )
	{
		ENABLE_UART1_RX_INTERRUPT();




		DISABLE_UART1_RX_INTERRUPT();
		app.MBdataReceived = FALSE;
		ENABLE_UART1_RX_INTERRUPT();

	}
	
	ENABLE_UART1_RX_INTERRUPT();


}

UINT8 APP_comCallBack( far UINT8 *rxPacket, far UINT8* txCode,far UINT8** txPacket)
{
	UINT8 i;

	UINT8 rxCode = rxPacket[0];
	UINT8 length = 0;
	
	SCHEDULE_UPDATE_INFO *data = (SCHEDULE_UPDATE_INFO*) ((UINT8*)rxPacket+1);
	updateSchedule(data, rxCode);


/*		case CMD_GET_COMM_STATUS:



			break;




		case CMD_RESET:

		for(i= 1; i < TRUCKS_SUPPORTED+1 ; i++)
		{
			resetSchedule(i);
		}

		break;
*/


	return length;

}
	



void updateSchedule(SCHEDULE_UPDATE_INFO *info, UINT8 command)
{
	UINT8 i;
	UINT8 truck;
	UINT8 activityCompleteFlag = TRUE;
	INT8 delayedActivity = 0xFF;
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

		truck_statusIndicator[truck][0] = truckIndicators[info->truck].indicatorRed[0];
		truck_statusIndicator[truck][1] = truckIndicators[info->truck].indicatorRed[1];
		truck_statusIndicator[truck][2] = truckIndicators[info->truck].indicatorRed[2];
		truck_statusIndicator[truck][3] = truckIndicators[info->truck].indicatorRed[3];

		truck_statusIndicator[truck][4] = SYM_CANCEL;
		truck_statusIndicator[truck][5] = SYM_CANCEL;
		truck_statusIndicator[truck][6] = ' ';
		truck_statusIndicator[truck][7] = ' ';

		clearScheduleTime();
	}	
	else
	{
		switch( info->milestone)
		{
			case MILESTONE_START:
			if( scheduleStatus[truck][info->activity - 1].activityStatus != ACTIVITY_SCHEDULED)				//if activity is not scheduled ignore cmd
				return ;
			
			truck_statusIndicator[truck][0] = truckIndicators[info->truck].indicatorGreen[0];
			truck_statusIndicator[truck][1] = truckIndicators[info->truck].indicatorGreen[1];
			truck_statusIndicator[truck][2] = truckIndicators[info->truck].indicatorGreen[2];
			truck_statusIndicator[truck][3] = truckIndicators[info->truck].indicatorGreen[3];

			truck_statusIndicator[truck][4] = ' ';
			truck_statusIndicator[truck][5] = SYM_ONGOING;
			truck_statusIndicator[truck][6] = ' ';
			truck_statusIndicator[truck][7] = ' ';


			getScheduleTime(&scheduleTable[truck][info->activity-1] , activityTime);
			
						
			scheduleStatus[truck][info->activity - 1].activityStatus = ACTIVITY_ONGOING;
			scheduleStatus[truck][info->activity - 1].status = info->status;
			break;

			case MILESTONE_END:
			if( scheduleStatus[truck][info->activity - 1].activityStatus != ACTIVITY_ONGOING)				//if activity is not scheduled ignore cmd
				return ;

			truck_statusIndicator[truck][0] = truckIndicators[info->truck].indicatorRed[0];
			truck_statusIndicator[truck][1] = truckIndicators[info->truck].indicatorRed[1];
			truck_statusIndicator[truck][2] = truckIndicators[info->truck].indicatorRed[2];
			truck_statusIndicator[truck][3] = truckIndicators[info->truck].indicatorRed[3];


			clearScheduleTime();
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


void getScheduleTime(ACTIVITY_SCHEDULE* as , UINT8* activityTime)
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



void loadSchedule(UINT8 truck, UINT8 activity)
{
	UINT8 i;
	for(i = 0; i < 8 ;i++)
	{
		DDR_loadDigit( ((truck-1)*32)+(activity*8)+ i,activityTime[i] );
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





void getActivitySchedule(UINT8 truck, ACTIVITY activity, ACTIVITY_SCHEDULE* activitySchedule)
{
#ifdef __FACTORY_CONFIGURATION__

	*activitySchedule = shipmentSchedule[truck][activity-1];
#else
	UINT8 i;

	for( i = 0; i < sizeof(ACTIVITY_SCHEDULE); i++)
	{
		*(activitySchedule+i) = Read_b_eep((EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + truck * sizeof(TRUCK_SCHEDULE) 
											+ (sizeof(ACTIVITY_SCHEDULE) * activity-1))+i);
		Busy_eep();
	}
/*	ReadBytesEEP(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + truck * sizeof(TRUCK_SCHEDULE) + (sizeof(ACTIVITY_SCHEDULE) * activity-1)
							 ,(UINT8 *)&activitySchedule,sizeof(ACTIVITY_SCHEDULE));
*/
#endif
}

void setSchedule(SCHEDULE_DATA *data)
{
	UINT8 i;
	UINT8 *ptr = &data;
//	WriteBytesEEP(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + data->truck*(sizeof(TRUCK_SCHEDULE))
//									, (UINT8*)&data,sizeof(ACTIVITY_SCHEDULE)*ACTIVITIES_SUPPORTED);

	for( i = 0; i < sizeof(ACTIVITY_SCHEDULE)*ACTIVITIES_SUPPORTED; i++)
	{
		Write_b_eep(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + data->truck*(sizeof(TRUCK_SCHEDULE)), *(ptr+i) );
		Busy_eep();
	}	
}

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

DISABLE_UART1_RX_INTERRUPT();
	app.MBdataReceived = TRUE;
ENABLE_UART1_RX_INTERRUPT();

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
* Function used to update truck number
*------------------------------------------------------------------------------
*/

void displayTruckNumber(UINT8* buffer)
{
	UINT8 i;
	UINT8 displayBuf[8] = {'0'};

	for( i = 0; i < TRUCKS_SUPPORTED; i++ )
	{
		displayBuf[i*2] = *(buffer+i)/10 + '0';
		displayBuf[(i*2)+1] = *(buffer+i)%10 + '0';
	}

	DigitDisplay_updateBufferPartial(displayBuf, 0, TRUCKS_SUPPORTED*2);		
}