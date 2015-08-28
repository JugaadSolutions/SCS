
/*
*------------------------------------------------------------------------------
* main.c
*
* main application specific module.
*
* (C)2008 Sam's Logic.
*
* The copyright notice above does not evidence any
* actual or intended publication of such source code.
*
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* File				: main.c
* Created by		: Sam
* Last changed by	: Sam
* Last changed		: 07/07/2009
*------------------------------------------------------------------------------
*
* Revision 0.0 07/07/2009 Sam
* Initial revision
*
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include "board.h"
#include "timer.h"	// Timer related functions
#include "uart.h"
#include "modbusMaster.h"
#include "heartBeat.h"
#include "app.h"
#include "string.h"
#include "mmd.h"
#include "mb.h"
#include "digitdisplay.h"
#include "rtc_driver.h"
#include "digit_driver.h"

/*
*------------------------------------------------------------------------------
* Private Defines
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Processor config bits
*------------------------------------------------------------------------------
*/

#pragma config OSC     = INTIO67
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF
#pragma config PWRT     = OFF
#pragma config BOREN    = ON
#pragma config BORV     = 3
#pragma config WDT      = OFF
#pragma config WDTPS    = 512	//32768
#pragma config MODE 	= MC
#pragma config MCLRE    = ON
#pragma config LPT1OSC  = OFF
//#pragma config PBADEN   = OFF
#pragma config STVREN   = ON
#pragma config LVP      = OFF
//#pragma config ICPRT  = OFF       // Dedicated In-Circuit Debug/Programming
#pragma config XINST    = OFF       // Extended Instruction Set
#pragma config CP0      = OFF
#pragma config CP1      = ON
#pragma config CP2      = ON
#pragma config CP3      = ON
#pragma config CPB      = ON
#pragma config CPD      = OFF
#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
#pragma config WRT2     = OFF
//#pragma config WRT3   = OFF
#pragma config WRTB     = OFF//N       // Boot Block Write Protection
#pragma config WRTC     = OFF
#pragma config WRTD     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
#pragma config EBTR2    = OFF
#pragma config EBTR3    = OFF
#pragma config EBTRB    = OFF


extern UINT32 TimerUpdate_count;
extern UINT16 keypadUpdate_count;
extern UINT16 eMBUpdate_count;

/*
*------------------------------------------------------------------------------
* Private Macros
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables
*------------------------------------------------------------------------------
*/
void EnableInterrupts(void);

/*
*------------------------------------------------------------------------------
* Private Variables (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Constants
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Constants (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Function Prototypes (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Functions
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* void main(void)

* Summary	: Application specifc main routine. Initializes all port and
*			: pheriperal and put the main task into an infinite loop.
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/

#define MMD_REFRESH_PERIOD	(65535 - 20000)
#define TICK_PERIOD	(65535 - 8000)

void main(void)
{
	UINT8 i,j,k;
	unsigned long temp;
	BOOL ledStrip_On = 0;

#if defined (MMD_TEST)
	MMD_Config mmdConfig= {0};
	UINT8 line[] = "ABCEFGHIJK";
#endif

	BRD_init();

#ifdef __DIGIT_DISPLAY_TEST__

	for(i = 128; i > 32 ; i-- )
	{
		for( k = 0; k < 11 ; k++)
			{
				DDR_loadDigit(i,k);
				DelayMs(100);
			}
	}
#endif




	HB_init();
	DigitDisplay_init(NO_OF_DIGIT);
	MMD_init();  // Display initialization

	RTC_Init(); //RTC Initialization

	APP_init();

	TMR0_init(tickPeriod ,DigitDisplay_task);	//initialize timer0
	TMR1_init(MMD_REFRESH_PERIOD,MMD_refreshDisplay);


	EnableInterrupts();



#if defined (MMD_TEST)
	MMD_clearSegment(0);
	mmdConfig.startAddress = MARQUEE_SEGMENT_START_ADDRESS;
	mmdConfig.length = MARQUEE_SEGMENT_LENGTH;
	mmdConfig.symbolCount = strlen(line);
	mmdConfig.symbolBuffer = line;
	mmdConfig.scrollSpeed = SCROLL_SPEED_LOW;
			
	MMD_configSegment( 0 , &mmdConfig);
/*
	MMD_clearSegment(1);
	mmdConfig.startAddress = 10;
	mmdConfig.length = 11;//MARQUEE_SEGMENT_LENGTH;
	mmdConfig.symbolCount = 11;
	mmdConfig.symbolBuffer = line1;
	mmdConfig.scrollSpeed = 0;//SCROLL_SPEED_LOW;
			
	MMD_configSegment( 1 , &mmdConfig);
*/

#endif

#ifdef UART2TEST
	for(i = 0 ; i < 26 ; i++)
	{
		 UART2_write(i + 'A');
		 UART2_transmit();		
	}	
#endif

	//Heart Beat to blink at every 500ms
	temp = (500UL *1000UL)/TIMER0_TIMEOUT_DURATION;

	while(1)
	{

		if(  heartBeatCount >= temp)
		{
			RTC_Task();
			HB_task();
			heartBeatCount = 0;
		}

		if( mmdUpdateCount >= 40 )
		{
			MMD_task();
			mmdUpdateCount = 0;
		}

		if(AppUpdate_count >= temp )
		{
		  APP_task();
          AppUpdate_count = 0;
		
		}

		if(eMBUpdate_count >= 10)
		{
		
			eMBPoll();	//modbus task
			MB_task();
			eMBUpdate_count = 0;
		}
	}
}

/*
*  End of main.c
*/