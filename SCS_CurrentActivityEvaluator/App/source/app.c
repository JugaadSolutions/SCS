
#include "app.h"





/*
*------------------------------------------------------------------------------
* app - the app structure. 
*------------------------------------------------------------------------------
*/

typedef struct _APP
{

	//Modbus buffer
	UINT8 eMBdata[NO_OF_DATA];
	UINT8 Update;

	APP_STATE state;
	UINT16 curMinute;
	UINT16 prevMinute;
	UINT8 breakID;
	UINT8 delayPercentage;
	UINT8 alarmPercentage;
	UINT8 secON;

	UINT8 timeBuffer[6];
	


}APP;


UINT8 txBuffer[7] = {0};
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

#pragma udata APP_DATA

APP app ;

ACTIVITY_SCHEDULE breaks[BREAKS_SUPPORTED+1];

UINT8 marquee[MARQUEE_SEGMENT_CHARS];
UINT8 time_backlight[TIME_SEGMENT_CHARS + BACKLIGHT_SEGMENT_CHARS];

//CurrentActivitySegment currentActivitySegment[CURRENT_ACTIVITY_SEGMENTS];

UINT8 activityParameterBuffer[ ACTIVITY_PARAMETER_BUFFER_SIZE];

MMD_Config mmdConfig;

ACTIVITY_STATUS scheduleTable[TRUCKS_SUPPORTED][ACTIVITIES_SUPPORTED];


#pragma udata

UINT8 readTimeDateBuffer[6] = {0};
UINT8 writeTimeDateBuffer[] = {0X00, 0X15, 0X18, 0X03, 0x027, 0X12, 0X13};



/*
*------------------------------------------------------------------------------
* Private Functions	Prototypes
*------------------------------------------------------------------------------
*/

void updateTime(void);
void updateReceivedData(void);
void updateMarquee(void);
void APP_ASCIIconversion(void);
void APP_resetCounter_Buffer(void);
void APP_conversion(void);


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
	UINT8 i;

	UINT16 sbaudrate , saddress;

//	updateTime();
	eMBErrorCode    eStatus;

	WriteRtcTimeAndDate(writeTimeDateBuffer);	
	sbaudrate = 19200;	//set baudrate
	saddress = DEVICE_ADDRESS;		//slave address

	//modbus configuration
	eStatus = eMBInit( MB_RTU, ( UCHAR )saddress, 0, sbaudrate, MB_PAR_NONE);
	eStatus = eMBEnable(  );	/* Enable the Modbus Protocol Stack. */


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
	
	updateTime();
	DISABLE_UART_RX_INTERRUPT();

	if(app.Update == TRUE)
	{
		ENABLE_UART_RX_INTERRUPT();

		updateReceivedData();

		DISABLE_UART_RX_INTERRUPT();
		app.Update = FALSE;	
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
					updateMarquee();
					break;
				}
		
			}

			break;

		case APP_STATE_INACTIVE:

			if( app.curMinute >= breaks[app.breakID].endMinute)
			{
				app.breakID =00;
				app.state = APP_STATE_ACTIVE;
				updateMarquee();
				
				
			}
			break;

		default:
			return;
	}



}


/*
*------------------------------------------------------------------------------
* void APP_resetCounter_Buffer(void)
*
* Summary	: 
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/


UINT8 APP_comCallBack( far UINT8 *rxPacket, far UINT8* txCode,far UINT8** txPacket)
{

	UINT8 i;

	UINT8 rxCode = rxPacket[0];
	UINT8 length = 0;


	return length;

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
	
	 DISABLE_UART_RX_INTERRUPT();
		app.Update = TRUE;
	ENABLE_UART_RX_INTERRUPT();
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


void resetAlarm()
{
//	HOOTER = SWITCH_OFF;
}

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


	mmdConfig.startAddress = 0;//TIME_SEGMENT_START_ADDRESS ;
	mmdConfig.length = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.symbolBuffer = time_backlight;
	mmdConfig.symbolCount = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;

	MMD_configSegment(0, &mmdConfig);


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

	

	MMD_configSegment(1, &mmdConfig);

}	



void updateReceivedData (void)
{
	UINT8 cmd = app.eMBdata[0];

	switch(cmd)
	{
		case CMD_RTC	:

			writeTimeDateBuffer[0] = 0;
			writeTimeDateBuffer[2] = ((app.eMBdata[1] - '0' ) << 4) | (app.eMBdata[2] - '0' ); //store minutes
			writeTimeDateBuffer[1] = ((app.eMBdata[3] - '0') << 4) | (app.eMBdata[4] - '0'); //store houres
		
			WriteRtcTimeAndDate(writeTimeDateBuffer);  //update RTC
			
		break;

		case CMD_HOOTER_OFF	:
			
		//	resetAlarm();
		break;
/*
		case CMD_PICKING_START:
			
			LAMP1 = 0;
			DelayMs(500);
			LAMP1 = 1;
		break;

		case CMD_PICKING_END:

			LAMP2 = 0;
			DelayMs(500);
			LAMP2 = 1;			

		break;

		case CMD_STAGING_START:

			LAMP3 = 0;
			DelayMs(500);
			LAMP3 = 1;

		break;

		case CMD_STAGING_END:

			LAMP4 = 0;
			DelayMs(500);
			LAMP4 = 1;
			

		break;

		case CMD_LOADING_START	:

			LAMP5 = 0;
			DelayMs(500);
			LAMP5 = 1;
			

		break;
		case CMD_LOADING_END:
			
			LAMP6 = 0;
			DelayMs(200);
			LAMP6 = 1;

		break;

		case CMD_TRUCK_TIMINGS	:
			
			LAMP7 = 0;
			DelayMs(500);
			LAMP7 = 1;

		break;

		case CMD_CANCEL_TRUCK	:
			
			LAMP8 = 0;
			DelayMs(500);
			LAMP8 = 1;
		break;
*/

		default:
		break;
	}
}

void APP_conversion(void)
{
			
	app.timeBuffer[0] = (readTimeDateBuffer[0] & 0X0F) + '0';        //Seconds LSB
	app.timeBuffer[1] = ((readTimeDateBuffer[0] & 0XF0) >> 4) + '0'; //Seconds MSB
	app.timeBuffer[2] = (readTimeDateBuffer[1] & 0X0F) + '0';        //Minute LSB
	app.timeBuffer[3] = ((readTimeDateBuffer[1] & 0XF0) >> 4) + '0' ; 		//Minute MSB
	app.timeBuffer[4] = (readTimeDateBuffer[2] & 0X0F) + '0';        //Minute LSB
	app.timeBuffer[5] = ((readTimeDateBuffer[2] & 0X30) >> 4)  + '0'; 		//Minute MSB

}
