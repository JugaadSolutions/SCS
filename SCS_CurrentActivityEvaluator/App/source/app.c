
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
	UINT8 eMBdata[NO_OF_DIGITS];
	UINT8 Update;

	//Variables to handle dot matrix display
	UINT8 model[MSG_LENGTH];
	UINT8 eepUpdate;

	//Variables to handle seven segment display
    APP_STATE state;
    UINT16 Actual_Count;
    UINT8 Actual[NO_OF_DIGITS];
    UINT8 Plan[NO_OF_DIGITS];
	UINT8 Input_Recieved;
	UINT8 plan_eepUpdate;
	UINT8 plan_eepModify;
	UINT8 actual_eepUpdate;
}APP;

#pragma idata APP_DATA
APP app = {{0},0};
MMD_Config mmdConfig = {0};
#pragma idata



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
	UINT8 i = 0;

	//Dot matrix initialization
	do
	{
			app.model[i] = Read_b_eep(EEPROM_ADDRESS+i);
			Busy_eep();	
	}while(app.model[i++]);

	MMD_clearSegment(0);
	mmdConfig.startAddress = 0;
	mmdConfig.length = MMD_MAX_CHARS;
	mmdConfig.symbolCount = strlen(app.model);
	mmdConfig.symbolBuffer = app.model;
	mmdConfig.scrollSpeed = 0;//SCROLL_SPEED_LOW;
	MMD_configSegment( 0 , &mmdConfig);




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
void APP_resetCounter_Buffer(void)
{
	UINT8 i;
	for(i = 0; i < NO_OF_DIGITS; i++)			//reset all digits
	{
		app.Actual[i] = '0';
	}
}

UINT8 APP_comCallBack( far UINT8 *rxPacket, far UINT8* txCode,far UINT8** txPacket)
{

	UINT8 i;

	UINT8 rxCode = rxPacket[0];
	UINT8 length = 0;

	switch( rxCode )
	{
		case CMD_SET_MODEL:
			app.eepUpdate = TRUE;
			strcpy(app.model,&rxPacket[1]);
			MMD_clearSegment(0);
			mmdConfig.startAddress = 0;
			mmdConfig.length = MMD_MAX_CHARS;
			mmdConfig.symbolCount = strlen(app.model);
			mmdConfig.symbolBuffer = app.model;
			mmdConfig.scrollSpeed = 0;//SCROLL_SPEED_LOW;
			MMD_configSegment( 0 , &mmdConfig);

			*txCode = CMD_SET_MODEL;
			break;

	  	case SET_PLAN:
			app.plan_eepUpdate= TRUE;
			*txCode = SET_PLAN;
		   
		break;
	

        case MODIFY_PLAN:

			app.plan_eepModify = TRUE;

			*txCode = MODIFY_PLAN;
		break;

		default:
		break;

	}

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
