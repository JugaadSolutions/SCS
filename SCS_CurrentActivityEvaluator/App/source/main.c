
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
extern UINT16 heartBeatCount ;
extern UINT16 comUpdateCount ;
extern UINT16 mmdUpdateCount;
extern UINT16 AppUpdate_count;
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
#define TICK_PERIOD	(65535 - 8000) //	500us


void main(void)
{
	UINT8 i,j,k, count;
	BOOL ledStrip_On = 0;

#if defined (MMD_TEST)
	MMD_Config mmdConfig= {0};
	UINT8 line[16] ="DENSO - BREAK "; 
#endif


	BRD_init();
	HB_init();
	DigitDisplay_init(16);
	MMD_init();  // Display initialization

	RTC_Init(); //RTC Initialization
	
	APP_init();

	TMR0_init(TICK_PERIOD,DigitDisplay_task);	//initialize timer0
	TMR1_init(MMD_REFRESH_PERIOD,MMD_refreshDisplay);


	EnableInterrupts();

#ifdef __DIGIT_DISPLAY_TEST__

	for(i = 32; i < 128 ; i++ )
	{
		for( k = 0; k < 10 ; k++)
			{
				DDR_loadDigit(i,k);
				DelayMs(75);
			}
	}
#endif

#if defined (MMD_TEST)
	MMD_clearSegment(0);
	mmdConfig.startAddress = 0;
	mmdConfig.length = MMD_MAX_CHARS;
	mmdConfig.symbolCount = strlen(line);
	mmdConfig.symbolBuffer = line;
	mmdConfig.scrollSpeed = SCROLL_SPEED_HIGH;
			
	MMD_configSegment( 0 , &mmdConfig);

#endif

#ifdef UART2TEST
	for(i = 0 ; i < 26 ; i++)
	{
		 UART2_write(i + 'A');
		 UART2_transmit();		
	}	
#endif


	while(1)
	{

		if(  heartBeatCount >= 550)
		{
			RTC_Task();
			HB_task();
			heartBeatCount = 0;
		}

		if( mmdUpdateCount >= 10 )
		{
			MMD_task();
			mmdUpdateCount = 0;
		}

		if(AppUpdate_count >=550 )
		{
		  APP_task();
          AppUpdate_count = 0;
		
		}

		MB_task();

		if(eMBUpdate_count >= 10)
		{
			eMBPoll();	//modbus task
			eMBUpdate_count = 0;
		}
	}
}

/*
*  End of main.c
*/