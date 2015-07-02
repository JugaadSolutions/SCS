#include "board.h"
#include "config.h"
#include "Keypad.h"
#include "lcd.h"
#include "string.h"
#include "ui.h"
#include "app.h"



typedef struct _UI
{
	UI_STATE state;
	UI_STATE prevState;
	UINT8 buffer[MAX_INPUT_CHARS+1];
	UINT8 bufferIndex;
	UINT8 prevcode;
	UINT8 keyIndex;
	UINT8 input[MAX_INPUT_CHARS+1];
	UINT8 inputIndex;
}UI;





const rom UINT8 *UI_MSG[] = {
								{"TRUCK NO:"},
								{"ACTIVITY:"},
								{"PICKING START"},
								{"STAGING START"},
								{"LOADING START"},

								{"PICKING END"},
								{"STAGING END"},
								{"LOADING END"},

								{"PASSWORD:"},

								{"ADMIN ACTIVITY:"},
			
								{"SCH:"},
								{"HOOTER OFF"},
								{"CANCEL TRUCK:"},
								{"SET RTC:"},

								{"COMPLETED"},

							};


const rom UINT8 keyMap[MAX_KEYS] = { '1','2','3','\x0A',
									 '4','5','6','\x0B',
									 '7','8','9','\x0C',
									 '*','0','\x08','\x0E' } ;




#pragma idata UI_DATA
UI ui = {0,0,{0},0,0xFF,0,0};
//#pragma idata



UINT8 mapKey(UINT8 scancode, UINT8 duration);
UINT8 getStation(void);
void getData(void);
void clearUIBuffer(void);
void putUImsg(UINT8 msgIndex);
void setUImsg( UINT8 msgIndex );
void clearUIInput(void);
void showUImsg( UINT8* msg );

void storeCMDinBuffer(UINT8 *buffer, UINT8 command);


void UI_init(void)
{
	UINT8 i = 0;

	LCD_setBackSpace('\x08');	//Indicates LCD driver "\x08" is the symbol for backspace

	setUImsg(UI_MSG_TRUCK_NO);

	ui.state = UI_GET_TRUCK_NO;

	clearUIBuffer();
	clearUIInput();
	//storeCMDinBuffer(ui.buffer,0x82);

}



void UI_task(void)
{

	UINT8 keypressed = 0xFF;
	UINT8 i = 0;
	UINT8 duration, scancode;
	UINT8 uimsg;
	UINT8 temp;

	if(KEYPAD_read(&scancode, &duration) == FALSE)			//Check whether key has been pressed
	{
		return;
	}

	
	keypressed = mapKey(scancode,duration);				//Map the key

	if( keypressed == 0xFF)
	{
		return;
	}


	switch(ui.state)
	{
		case UI_GET_TRUCK_NO:

		//If pressed key is backspace
		if( keypressed == '\x08')
		{
			if(ui.bufferIndex > 0 )
			{
				LCD_putChar(keypressed);
				ui.bufferIndex--;
				if( ui.inputIndex > 0 )
					ui.inputIndex--;
			}

		}

		//if the pressed key is enter
		else if( keypressed == '\x0E')
		{
			if(ui.bufferIndex > 0)
			{
				if(APP_activityValid(ui.buffer) == VALID)
				{
					setUImsg(UI_MSG_ACTIVITY);
					ui.state = UI_ACTIVITY;
				}
				else
				{
					setUImsg(UI_MSG_TRUCK_NO);
					clearUIBuffer();
					clearUIInput();
					ui.state = UI_GET_TRUCK_NO;
				}				
			}
		}

		//if the pressed key password
		else if( keypressed == '*')
		{


			setUImsg(UI_MSG_PASSWORD);

			clearUIBuffer();

			ui.state = UI_PASSWORD;
				
			
		}
		
		else 
		{
			if( ui.bufferIndex < 2)
			{
				ui.buffer[ui.bufferIndex] = keypressed;
				LCD_putChar(ui.buffer[ui.bufferIndex]);
				ui.bufferIndex++;
			}
		}

		break;

		case UI_ACTIVITY:

		if( keypressed == '\x08')
		{	
			setUImsg(UI_MSG_TRUCK_NO);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_GET_TRUCK_NO;
		}

		// If the pressed key is picking
		else if( keypressed == '\x0A')
		{
			UINT8 picking = INVALID;

			picking = APP_validatePicking(ui.buffer);
		
			if(picking == INVALID)
				return;
			else if(picking == CMD_PICKING_START)
			{
				putUImsg(UI_MSG_PICKING_START);
			}
			else if(picking == CMD_PICKING_END)
			{
				putUImsg(UI_MSG_PICKING_END);
			}
			
			ui.state = UI_PICKING;
		}

		// If the pressed key is staging
		else if( keypressed == '\x0B')
		{

			UINT8 staging = INVALID;

			staging = APP_validateStaging(ui.buffer);
		
			if(staging == INVALID)
				return;
			else if(staging == CMD_STAGING_START)
			{
				putUImsg(UI_MSG_STAGING_START);
			}
			else if(staging == CMD_STAGING_END)
			{
				putUImsg(UI_MSG_STAGING_END);
			}

			ui.state = UI_STAGING;

		}

		// If the pressed key is loading
		else if( keypressed == '\x0C')
		{

			UINT8 loading = INVALID;

			loading = APP_validateLoading(ui.buffer);
		
			if(loading == INVALID)
				return;
			else if(loading == CMD_LOADING_START)
			{
				putUImsg(UI_MSG_LOADING_START);
			}
			else if(loading == CMD_LOADING_END)
			{
				putUImsg(UI_MSG_LOADING_END);
			}
			ui.state = UI_LOADING;

		}
	
		break;

		case UI_PICKING:

		if(keypressed == '\x08')
		{
			setUImsg(UI_MSG_ACTIVITY);
			clearUIBuffer();
			ui.state = UI_ACTIVITY;
		}

		//If the pressed key is enter store the state
		//Send packet on modbus
		//and switch the state into IDLE
		else if(keypressed == '\x0E')
		{
			APP_managePicking(ui.buffer);

			//store into log
			App_updateLog(ui.buffer);

			setUImsg(UI_MSG_TRUCK_NO);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_GET_TRUCK_NO;

		}


		break;


		case UI_STAGING:

		if( keypressed == '\x08')
		{
			setUImsg(UI_MSG_ACTIVITY);
			clearUIBuffer();
			ui.state = UI_ACTIVITY;
		}

		//If the pressed key is enter store the state
		//Send packet on modbus
		//and switch the state into IDLE
		else if( keypressed == '\x0E')
		{
			APP_manageStaging(ui.buffer);

			//store into log
			App_updateLog(ui.buffer);

			setUImsg(UI_MSG_TRUCK_NO);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_GET_TRUCK_NO;

		}


		break;


		case UI_LOADING:

		if( keypressed == '\x08')
		{
			setUImsg(UI_MSG_ACTIVITY);
			clearUIBuffer();
			ui.state = UI_ACTIVITY;
		}

		//If the pressed key is enter store the state
		//Send packet on modbus
		//and switch the state into IDLE
		else if( keypressed == '\x0E')
		{

			APP_manageLoading(ui.buffer);

			//store into log
			App_updateLog(ui.buffer);

			setUImsg(UI_MSG_TRUCK_NO);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_GET_TRUCK_NO;

		}


		break;

		case UI_PASSWORD:
		if( keypressed == '\x08')
		{	
			if(ui.bufferIndex > 0 )
			{
				LCD_putChar(keypressed);
				ui.bufferIndex--;
			}

			else
			{	
				setUImsg(UI_MSG_TRUCK_NO);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_GET_TRUCK_NO;	
			}	
		}

		else if( keypressed == '\x0E')
		{
			BOOL result = FALSE;
			ui.buffer[ui.bufferIndex] = '\0';
	
		
			 
			result = APP_checkPassword(ui.buffer);	
			if( result == TRUE )
			{
				setUImsg(UI_MSG_ADMIN_ACTIVITY);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_ADMIN_ACTIVITY;
			}
			else
			{
				setUImsg(UI_MSG_TRUCK_NO);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_GET_TRUCK_NO;
			}
		
		
		}

		else 
		{
			if(ui.bufferIndex < 4)
			{
				ui.buffer[ui.bufferIndex] = keypressed;
				LCD_putChar('*');
				ui.bufferIndex++;
			}
		}
			
		break;		

		case UI_ADMIN_ACTIVITY:

		if( keypressed == '\x08')
		{
			setUImsg(UI_MSG_TRUCK_NO);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_GET_TRUCK_NO;
			
		}

		else if( keypressed == '0')
		{
			setUImsg(UI_MSG_SET_TIMINGS);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_SET_TIMINGS;
		}

		else if( keypressed == '1')
		{
			putUImsg(UI_MSG_HOOTER_OFF);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_HOOTER_OFF;
		}

		else if( keypressed == '2')
		{
			setUImsg(UI_MSG_CANCEL_TRUCK);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_CANCEL_TRUCK;
		}

		else if( keypressed == '3')
		{
			setUImsg(UI_MSG_SET_RTC);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_SET_RTC;
		}

		break;

		case UI_SET_TIMINGS:

		if( keypressed == '\x08')
		{
			if(ui.bufferIndex > 0 )
			{
				LCD_putChar(keypressed);
				ui.bufferIndex--;
				if( ui.inputIndex > 0 )
					ui.inputIndex--;
			}
			else
			{	
				setUImsg(UI_MSG_ADMIN_ACTIVITY);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_ADMIN_ACTIVITY;
			}
		}

		else if( keypressed == '\x0E')
		{
			if(ui.bufferIndex > 0)
			{
				ui.buffer[ui.bufferIndex] = '\0';	
	
				storeCMDinBuffer(ui.buffer, CMD_TRUCK_TIMINGS);
			
				//store into log
				App_updateLog(ui.buffer);			
	
				setUImsg(UI_MSG_TRUCK_NO);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_GET_TRUCK_NO;
			}
		}
		else 
		{
			if( ui.bufferIndex < 26)
			{
				ui.buffer[ui.bufferIndex] = keypressed;
				LCD_putChar(ui.buffer[ui.bufferIndex]);
				ui.bufferIndex++;
			}
		}

		break;

		case UI_HOOTER_OFF:

		if( keypressed == '\x08')
		{	
			setUImsg(UI_MSG_ADMIN_ACTIVITY);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_ADMIN_ACTIVITY;
		}
		else if( keypressed == '\x0E')
		{

			ui.buffer[0] = '\0';	

			storeCMDinBuffer(ui.buffer, CMD_HOOTER_OFF);
		
			//store into log
			App_updateLog(ui.buffer);	

			setUImsg(UI_MSG_TRUCK_NO);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_GET_TRUCK_NO;

		}


		break;

		case UI_CANCEL_TRUCK:

		if( keypressed == '\x08')
		{	
			setUImsg(UI_MSG_ADMIN_ACTIVITY);
			clearUIBuffer();
			clearUIInput();
			ui.state = UI_ADMIN_ACTIVITY;
		}
		else if( keypressed == '\x0E')
		{
			ui.buffer[ui.bufferIndex] = '\0';

			if(APP_activityValid(ui.buffer) == VALID)
			{	
				APP_cancelTruck(ui.buffer);
				storeCMDinBuffer(ui.buffer, CMD_CANCEL_TRUCK);
			
				//store into log
				App_updateLog(ui.buffer);	
	
				setUImsg(UI_MSG_TRUCK_NO);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_GET_TRUCK_NO;
			}
			else
			{
				setUImsg(UI_MSG_CANCEL_TRUCK);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_CANCEL_TRUCK;
			}
		}
		//store truck number
		else 
		{
			if( ui.bufferIndex < 2)
			{
				ui.buffer[ui.bufferIndex] = keypressed;
				LCD_putChar(ui.buffer[ui.bufferIndex]);
				ui.bufferIndex++;
			}
		}
		break;

		case UI_SET_RTC:

		if( keypressed == '\x08')
		{	
			if(ui.bufferIndex > 0 )
			{
				LCD_putChar(keypressed);
				ui.bufferIndex--;
				if( ui.inputIndex > 0 )
					ui.inputIndex--;
			}
			else
			{
				setUImsg(UI_MSG_ADMIN_ACTIVITY);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_ADMIN_ACTIVITY;
			}
		}

		else if( keypressed == '\x0E')
		{
			if(ui.bufferIndex > 0)
			{
				ui.buffer[ui.bufferIndex] = '\0';	
	
				storeCMDinBuffer(ui.buffer, CMD_SET_RTC);
			
				//store into log
				App_updateLog(ui.buffer);	
	
				setUImsg(UI_MSG_TRUCK_NO);
				clearUIBuffer();
				clearUIInput();
				ui.state = UI_GET_TRUCK_NO;
			}
		}
		//store RTC data
		else 
		{
			if( ui.bufferIndex < 4)
			{
				ui.buffer[ui.bufferIndex] = keypressed;
				LCD_putChar(ui.buffer[ui.bufferIndex]);
				ui.bufferIndex++;
			}
		}

		break;
		break;
	
		default:
		break;


	}



}


UINT8 mapKey(UINT8 scancode, UINT8 duration)
{
	UINT8 keypressed = 0xFF;
	switch(ui.state)
	{

		case UI_GET_TRUCK_NO:
		keypressed = keyMap[scancode];
		
		if( (ui.bufferIndex == 0 ))
		{
		
			if( (keypressed == '\xA') || (keypressed =='\x0B')
				|| (keypressed =='\x0C') || (keypressed =='\x0E') )
				keypressed = 0xFF;

		}

		break;


		case UI_ACTIVITY:

		keypressed = keyMap[scancode];

		if( (keypressed == '\x8') || (keypressed =='\x0A') 
			|| (keypressed =='\x0B') || (keypressed =='\x0C') )
			break;

		else 
			keypressed = 0xFF;


		break;


		case UI_PICKING:
		case UI_STAGING:
		case UI_LOADING:

		keypressed = keyMap[scancode];

		if( (keypressed == '\x8') || (keypressed =='\x0E') )
			break;

		else 
			keypressed = 0xFF;

		break;


		case UI_PASSWORD:
			keypressed = keyMap[scancode];
		break;


		case UI_ADMIN_ACTIVITY:
		keypressed = keyMap[scancode];
		if( (keypressed != '0') && (keypressed !='\x08') && (keypressed != '\x0A')
			&& (keypressed != '1') && (keypressed != '2') && (keypressed != '3') )
			keypressed = 0xFF;
		break;



		default: keypressed = keyMap[scancode];
		break;

	}

	return keypressed;
}




UINT8 getStation(void)
{
	UINT8 i,station = 0;

	if( ui.bufferIndex == 1 )
	{
		ui.input[ui.inputIndex] = '0';
		ui.inputIndex++;
		ui.input[ui.inputIndex] = ui.buffer[0];
		ui.inputIndex++;
	}

	else
	{
		ui.input[ui.inputIndex] = ui.buffer[0];
		ui.inputIndex++;
		ui.input[ui.inputIndex] = ui.buffer[1];
		ui.inputIndex++;
	}

	station = (ui.input[0]-'0')*10 + (ui.input[1]-'0');

	return station;
}



void getData(void)
{
	UINT8 i;

	for( i = 0; i< ui.bufferIndex; i++)
	{
		ui.input[ui.inputIndex] = ui.buffer[i];
		ui.inputIndex++;
		
	}
	ui.input[ui.inputIndex] = '\0';
	ui.inputIndex++;

	if( ui.inputIndex >= MAX_INPUT_CHARS )
		ui.inputIndex = 0;
}


void clearUIBuffer(void)
{
	memset(ui.buffer,0, MAX_INPUT_CHARS);
	ui.bufferIndex = 0;
	ui.keyIndex = 0;
	ui.prevcode = 0xFF;

}


void clearUIInput(void)
{
	memset((UINT8*)ui.input,0, MAX_INPUT_CHARS);
	ui.inputIndex = 0;
}




void showUImsg( UINT8* msg )
{
	UINT8 i;

	
	LCD_clear();

	i = 0;
	while( msg[i] != '\0')
	{
		LCD_putChar(msg[i]);
		i++;
	}
}


void setUImsg( UINT8 msgIndex )
{
	UINT8 i;

	const rom UINT8 *msg;

	msg = UI_MSG[msgIndex] ;

	LCD_clear();

	i = 0;
	while( msg[i] != '\0')
	{
		LCD_putChar(msg[i]);
		i++;
	}
}


void putUImsg(UINT8 msgIndex)
{
	UINT8 i;

	const rom UINT8 *msg;

	msg = UI_MSG[msgIndex] ;

	i = 0;
	while( msg[i] != '\0')
	{
		LCD_putChar(msg[i]);
		i++;
	}
}


void storeCMDinBuffer(UINT8 *buffer, UINT8 command)
{
	INT8 i = 0;

	while(buffer[i++] != '\0');

	for(; i > 0; i--)
		buffer[i] = buffer[i-1];

	buffer[0] = command;	
}

		