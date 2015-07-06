
/*
*------------------------------------------------------------------------------
* device.c
*
* Board specipic drivers module(BSP)
*
* (C)2009 Sam's Logic.
*
* The copyright notice above does not evidence any
* actual or intended publication of such source code.
*
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* File				: device.c
* Created by		: Sam
* Last changed by	: Sam
* Last changed		: 11/12/2010 
*------------------------------------------------------------------------------
* Revision 1.3 11/12/2010  Sam
* Demo Release
* Revision 1.2 15/11/2010  Sam
* Updated according to the current project requirement
* Revision 1.1 07/07/2010 Sam
* First Release
* Revision 1.0 14/07/2009 Sam
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
#include "config.h"
#include "typedefs.h"




/*
*------------------------------------------------------------------------------
* Private Defines
*------------------------------------------------------------------------------
*/

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

/*
*------------------------------------------------------------------------------
* Private Variables (static)
*------------------------------------------------------------------------------
*/

static BOOL ledState;

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
* void InitializeBoard(void)

* Summary	: This function configures all i/o pin directions
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
void BRD_init(void)
{
	OSCCON = 0x70;
	DelayMs(5);
	// set all anolog channels as Digital I/O
	ADCON0 = 0x00;
	ADCON1 = 0x0F;
	ADCON2 = 0xB5;

	MEMCON = 0x80;

	// Configure heart beat LED output
	HEART_BEAT_DIR 	= PORT_OUT;
	HEART_BEAT 		= SWITCH_OFF;

	// Rs485 Direction Controle
	TX1_EN_DIR 		= PORT_OUT;
	TX1_EN 			= SWITCH_OFF;

	TX2_EN_DIR 		= PORT_OUT;
	TX2_EN 			= SWITCH_OFF;

	// Configure Serial port
	SER2_TX_DIR 		=  PORT_OUT;
	SER2_RX_DIR 		=  PORT_IN;

	SER1_TX_DIR 		=  PORT_OUT;
	SER1_RX_DIR 		=  PORT_IN;

	// Configure I2C Port
	I2C_SCL_DIR = PORT_OUT;
	I2C_SDA_DIR = PORT_OUT;	


	// MMD Configuration
	DISPLAY_CONTROL_DIRECTION = PORT_OUT;
	DISPLAY_CONTROL			  = DISPLAY_DISABLE;


	DATA_PORT_DIR = 0x00;
	DIGIT_PORT_DIR = 0x00;
	DIGIT_PORT = 0x00;
	

	ROW_SEL_A_DIR = PORT_OUT;
	ROW_SEL_A = SWITCH_OFF;
	ROW_SEL_B_DIR = PORT_OUT;
	ROW_SEL_B = SWITCH_OFF;
	ROW_SEL_C_DIR = PORT_OUT;
	ROW_SEL_C = SWITCH_OFF;
	ROW_SEL_D_DIR = PORT_OUT;
	ROW_SEL_D = SWITCH_OFF;
	ROW_SEL_E_DIR = PORT_OUT;
	ROW_SEL_E = SWITCH_OFF;
	ROW_SEL_F_DIR = PORT_OUT;
	ROW_SEL_F = SWITCH_OFF;
	ROW_SEL_G_DIR = PORT_OUT;
	ROW_SEL_G = SWITCH_OFF;


	OSCTUNEbits.PLLEN = 1;
	DelayMs(100);


	//DATA OUT
    DISPLAY_PORT_DIRECTION = PORT_OUT;
   
	//DIGIT SELECTION
	SSD_DITIT_SEL_A_DIR = PORT_OUT;
	SSD_DITIT_SEL_B_DIR = PORT_OUT;
	

   // Enable internal PORTB pull-ups
    INTCON2bits.RBPU = 0;

}

	



/*
*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

/*
*  End of device.c
*/
