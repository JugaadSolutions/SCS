
#include "app.h"




void APP_ASCIIconversion(void);
void APP_resetCounter_Buffer(void);


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

	UINT16 curMinute;
	UINT16 prevMinute;
	UINT8 breakID;
	UINT8 delayPercentage;
	UINT8 alarmPercentage;
	UINT8 secON;
	


}APP;



#pragma idata APP_DATA

APP app = {{0}, 0} ;
MMD_Config mmdConfig = {0};
#pragma idata


UINT8 readTimeDateBuffer[6] = {0};
UINT8 writeTimeDateBuffer[] = {0X00, 0X02, 0X06, 0X03, 0x027, 0X12, 0X13};


/*
*------------------------------------------------------------------------------
* Private Functions	Prototypes
*------------------------------------------------------------------------------
*/

void updateTime(void);
void updateReceivedData(void);
UINT8 time_backlight[TIME_SEGMENT_CHARS + BACKLIGHT_SEGMENT_CHARS];


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
	HOOTER = SWITCH_OFF;
}

void updateTime(void)
{
	UINT8 i;
	UINT16 hour ,minute;
	hour= RTC_getHour();
	minute = RTC_getMinute();
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


	mmdConfig.startAddress =TIME_SEGMENT_START_ADDRESS ;
	mmdConfig.length = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.symbolBuffer = time_backlight;
	mmdConfig.symbolCount = TIME_SEGMENT_CHARS+BACKLIGHT_SEGMENT_CHARS;
	mmdConfig.scrollSpeed = SCROLL_SPEED_NONE;

	MMD_configSegment(1, &mmdConfig);


}



void updateReceivedData (void)
{
	int cmd = app.eMBdata[0];
	
	switch(cmd)
	{
		case CMD_RTC	:
			writeTimeDateBuffer[0] = 0;
			writeTimeDateBuffer[1] = ((app.eMBdata[1] - '0' ) << 4) | (app.eMBdata[2] - '0' ); //store minutes
			writeTimeDateBuffer[2] = ((app.eMBdata[3] - '0') << 4) | (app.eMBdata[4] - '0'); //store houres
		
			WriteRtcTimeAndDate(writeTimeDateBuffer);  //update RTC

		case CMD_HOOTER_OFF	:
			
			resetAlarm();
		default:
		break;
	}
}

