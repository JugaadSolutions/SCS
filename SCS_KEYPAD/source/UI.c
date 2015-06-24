#include "board.h"
#include "config.h"
#include "Keypad.h"
#include "lcd.h"
#include "string.h"
#include "ui.h"
#include "app.h"


				

/*----------------------------------------
* structure to handle truck informations
*----------------------------------------*/
typedef struct _TRUCK_INFO
{
	UINT16 ID;
	UINT8 state[MAX_STATES];
}TRUCK_INFO;

typedef struct _UI
{
	UI_STATE state;
	UI_STATE prevState;
	UINT8 buffer[MAX_INPUT_CHARS+1];
	UINT8 bufferIndex;
	UINT8 prevcode;
	UINT8 keyIndex;
	TRUCK_INFO truck[MAX_NUMBER_OF_TRUCKS];
	UINT8 truckIndex;
	UINT8 setActivity;
}UI;

const rom UINT8 UI_MSG[][20] = {
						"TRUCK NO:",
						"ACTIVITY:"	
						"PICKING START",
						"STAGING START",
						"LOADING START",
						"PICKING END",
						"STAGING END",
						"LOADING END",
						"CANCELLED",
						"HOOTER OFF",
						"SET RTC",
						"SET TRUCK TIMINGS:"
					   };			
			
const rom UINT8 keyMap[MAX_KEYS] = { '1','2','3','\x0P',
									 '4','5','6','\x0S',
									 '7','8','9','\x0L',
									 '*','0','\x08','\x0E' } ;




#pragma idata UI_DATA
UI ui = {0,0,{0},0,0xFF,0,0};
OpenIssue openIssue={{0},-1};
OpenIssue ackIssue={{0},-1};
//#pragma idata



UINT8 mapKey(UINT8 scancode, UINT8 duration);
UINT8 getStation(void);
void getData(void);
void clearUIBuffer(void);
void putUImsg(UINT8 msgIndex);
void setUImsg( UINT8 msgIndex );
void clearUIInput(void);
void showUImsg( UINT8* msg );
void set_UI_IdleState(void);

void UI_init(void)
{

	LCD_setBackSpace('\x08');	//Indicates LCD driver "\x08" is the symbol for backspace

	ui.state = UI_IDLE;
//	ui.inputIndex = 0;
	ui.truckIndex = 0;

	clearUIBuffer();
	clearUIInput();

	//Show truck number on the LCD
	showUImsg(UI_MSG[0]);

	
}



void UI_task(void)
{

	UINT8 keypressed = 0xFF;
	UINT8 i, j = 0;
	UINT8 duration, scancode;
	UINT8 uimsg;		
	UINT16 truckNumber = 0;

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
		case UI_IDLE:

		//code to handle backspace
		if(keypressed == '\x08')
		{
			if(ui.bufferIndex > 0)
				ui.bufferIndex--;

			ui.buffer[ui.bufferIndex] = '\0';
			LCD_putChar(keypressed);
			
		}
		//if the pressed key is other than backspace store it in the buffer and display
		else if (ui.bufferIndex < 3)
		{
			ui.buffer[ui.bufferIndex]  = keypressed;
			ui.bufferIndex++;

			LCD_putChar(keypressed);
		}
		
		break;

		case UI_PICKING:

		//Check for the limit, if exceeds make it to zero
		//Check for the empty activity
		if(ui.truckIndex > MAX_NUMBER_OF_TRUCKS)
		{
			set_UI_IdleState();
			break;
		}	

		truckNumber = atoi(ui.buffer);	

		//if truckNumber is invalid do nothing
		if(truckNumber == 0)
		{
			set_UI_IdleState();
			break;
		}	
		
		//If the entry is first then store truck number and its status, then return
		if(ui.truckIndex == 0)
		{
			ui.truck[ui.truckIndex].ID = truckNumber;
			ui.truck[ui.truckIndex].state[PICKING] = START;
			ui.truckIndex++;

			//call App_sendDataOnModbus
			//App_sendDataOnModbus(truckNumber, START);
	
			//call App_writeEEPROM
			break;
		}

		//Check if the truck is already exist in the list
		for(i = 0; i < ui.truckIndex; i++)
		{
			if(ui.truck[i].ID != truckNumber)
				continue;

			if(ui.truck[i].state[PICKING] == START)
			{
				ui.truck[i].state[PICKING] == END;

				//call App_sendDataOnModbus
				//App_sendDataOnModbus(ui.truck[i-1].Id, ui.truck.state);
	
				//call App_writeEEPROM
				break;
			}
		}
			
		//call App_sendDataOnModbus
		//App_sendDataOnModbus(ui.truck[i-1].Id, ui.truck.state);

		//call App_writeEEPROM

		set_UI_IdleState();
		break;


		case UI_STAGING:
		//Check for the limit, if exceeds make it to zero
		//Check for the empty activity
		if(ui.truckIndex > MAX_NUMBER_OF_TRUCKS)
		{
			set_UI_IdleState();
			break;
		}	

		truckNumber = atoi(ui.buffer);	

		//if truckNumber is invalid do nothing
		if(truckNumber == 0)
		{
			set_UI_IdleState();
			break;
		}	


		//Check if the truck is already exist in the list
		for(i = 0; i < ui.truckIndex; i++)
		{
			if(ui.truck[i].ID != truckNumber)
				continue;

			if(ui.truck[i].state[PICKING] == START)
			{
				ui.truck[i].state[STAGING] = START;
				break;
			}
			else if(ui.truck[i].state[STAGING] == START)
			{
				ui.truck[i].state[STAGING] = END;
				break;
			}
		}

		//call App_sendDataOnModbus
		//App_sendDataOnModbus(ui.truck[i-1].Id, ui.truck.state);

		//call App_writeEEPROM

		set_UI_IdleState();
		break;

	
		case UI_LOADING:
		//Check for the limit, if exceeds make it to zero
		//Check for the empty activity
		if(ui.truckIndex > MAX_NUMBER_OF_TRUCKS)
		{
			set_UI_IdleState();
			break;
		}	

		truckNumber = atoi(ui.buffer);	

		//if truckNumber is invalid do nothing
		if(truckNumber == 0)
		{
			set_UI_IdleState();
			break;
		}	


		//Check if the truck is already exist in the list
		for(i = 0; i < ui.truckIndex; i++)
		{
			if(ui.truck[i].ID != truckNumber)
				continue;

			if(ui.truck[i].state[STAGING] == START
				|| ui.truck[i].state[STAGING] == END)
			{
				ui.truck[i].state[LOADING] = START;
				break;
			}
			else if(ui.truck[i].state[LOADING] == START)
			{
				ui.truck[i].state[LOADING] = END;
				break;
			}
		}

		//call App_sendDataOnModbus
		//App_sendDataOnModbus(ui.truck[i-1].Id, ui.truck.state);

		//call App_writeEEPROM

		set_UI_IdleState();
		break;



	}



}


UINT8 mapKey(UINT8 scancode, UINT8 duration)
{
	UINT8 keypressed = 0xFF;

	switch(ui.state)
	{

		case UI_IDLE:
		keypressed = keyMap[scancode];

		if(keypressed =='\x0P')
			ui.state = UI_PICKING;
		else if(keypressed == '\x0S')
			ui.state = UI_STAGING;
		else if(keypressed == '\x0L')
			ui.state = UI_LOADING;

		break;

		case UI_PICKING:

		keypressed = keyMap[scancode];

		if( (keypressed != '\x0A')&&(keypressed != '\x08') )
		{
			keypressed = 0xFF;
		}

		break;



		case UI_LOADING:

		if(scancode == ui.prevcode)
		{
			if(duration < MIN_KEYPRESS_DURATION )
			{
				ui.keyIndex++;
				if(ui.keyIndex >= MAX_CHAR_PER_KEY)
					ui.keyIndex = 0;
			}
			else
			{
				ui.keyIndex = 0;
			}

			
		}
		else
		{
			ui.keyIndex = 0;
		}


		default:
		break;

	}

	return keypressed;
}


UINT8 getStation(void)
{
	UINT8 i,station = 0;

/*	if( ui.bufferIndex == 1 )
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
*/
	return station;
}


void getData(void)
{
	UINT8 i;

/*	for( i = 0; i< ui.bufferIndex; i++)
	{
		ui.input[ui.inputIndex] = ui.buffer[i];
		ui.inputIndex++;
		
	}
	ui.input[ui.inputIndex] = '\0';
	ui.inputIndex++;

	if( ui.inputIndex >= MAX_INPUT_CHARS )
		ui.inputIndex = 0;
*/
}


void clearUIBuffer(void)
{
//	memset(ui.buffer,0, MAX_INPUT_CHARS);
	ui.bufferIndex = 0;
	ui.keyIndex = 0;
	ui.prevcode = 0xFF;

}


void clearUIInput(void)
{
	//memset((UINT8*)ui.input,0, MAX_INPUT_CHARS);
//	ui.inputIndex = 0;
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


void set_UI_IdleState(void)
{
	ui.state = UI_IDLE;
	ui.bufferIndex = 0;
	LCD_clear( );
}		