#ifndef __BOARD__
#define __BOARD__

/*
*------------------------------------------------------------------------------
* device.h
*
* Include file for port pin assignments
*

*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* File				: device.h
*------------------------------------------------------------------------------
*

*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include <p18f8722.h>
#include <delays.h>
#include <timers.h>
#include "typedefs.h"
#include "config.h"


/*
*------------------------------------------------------------------------------
* Hardware Port Allocation
*------------------------------------------------------------------------------
*/



// Rs485 / RS232 Serial commnunicaton port
#define		TX1_EN					PORTGbits.RG3			// TX control for RS485 communication
#define		TX1_EN_DIR				TRISGbits.TRISG3
//#define		TX2_EN					PORTGbits.RG4			// TX control for RS485 communication
//#define		TX2_EN_DIR				TRISGbits.TRISG4


#define 	SER2_TX					PORTGbits.RG1 		// serial transmit
#define		SER2_TX_DIR				TRISGbits.TRISG1
#define 	SER2_RX					PORTGbits.RG2			// serial receive
#define		SER2_RX_DIR				TRISGbits.TRISG2


#define 	SER1_TX					PORTCbits.RC6 		// serial transmit
#define		SER1_TX_DIR				TRISCbits.TRISC6
#define 	SER1_RX					PORTCbits.RC7			// serial receive
#define		SER1_RX_DIR				TRISCbits.TRISC7


// I2C communication Port
#define 	I2C_SCL					PORTCbits.RC3
#define 	I2C_SCL_DIR 			TRISCbits.RC3
#define 	I2C_SDA					PORTCbits.RC4
#define 	I2C_SDA_DIR 			TRISCbits.RC4


// HeartBeat
#define		HEART_BEAT				PORTGbits.RG0			// high - OFF, Low - ON
#define		HEART_BEAT_DIR			TRISGbits.TRISG0

// MMD PORTS
#define 	DISPLAY_CONTROL				PORTCbits.RC0
#define 	DISPLAY_CONTROL_DIRECTION	TRISCbits.TRISC0

#define		DATA_PORT				PORTA	//LATD	 				// 7seg display data (DB0-DB7)
#define 	DATA_PORT_DIR			TRISA
#define		DIGIT_PORT				PORTJ	//LATB	 				// digit drivers(upto 128 digits)
#define		DIGIT_PORT_DIR			TRISJ	//LATB	 				// digit drivers(upto 128 digits)

#define		ROW_SEL_A				PORTBbits.RB0			// decoder digit sel A
#define		ROW_SEL_A_DIR			TRISBbits.TRISB0
#define		ROW_SEL_B				PORTBbits.RB1		// decoder digit sel B
#define		ROW_SEL_B_DIR			TRISBbits.TRISB1
#define		ROW_SEL_C				PORTBbits.RB2		// decoder digit sel C
#define		ROW_SEL_C_DIR			TRISBbits.TRISB2
#define		ROW_SEL_D				PORTBbits.RB3		// decoder digit sel D
#define		ROW_SEL_D_DIR			TRISBbits.TRISB3
#define		ROW_SEL_E				PORTBbits.RB4		// decoder digit sel E
#define		ROW_SEL_E_DIR			TRISBbits.TRISB4
#define		ROW_SEL_F				PORTBbits.RB5		// decoder digit sel F
#define		ROW_SEL_F_DIR			TRISBbits.TRISB5
#define		ROW_SEL_G				PORTBbits.RB6	// decoder digit sel G
#define		ROW_SEL_G_DIR			TRISBbits.TRISB6
#define		ROW_SEL_H				PORTBbits.RB7	// decoder digit sel G
#define		ROW_SEL_H_DIR			TRISBbits.TRISB7

//Seven segment digit
#define	DIGIT_PORT_A               PORTD
#define DIGIT_PORT_A_DIRECTION     TRISD

#define DIGIT_PORT_B               PORTF
#define DIGIT_PORT_B_DIRECTION     TRISF

//Seven segment data
#define	DATA_1_PORT        	       PORTH
#define DATA_1_PORT_DIRECTION  	   TRISH

					
//#define 	HOOTER					PORTCbits.RC0 		// serial transmit
//#define		HOOTER_DIR				TRISCbits.TRISC0

			
/*
*------------------------------------------------------------------------------
* Public Defines
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Macros
*------------------------------------------------------------------------------
*/



#define SYSTEM_CLOCK			(32000000UL)	// 8MHz internal oscillator	

#define PERIPHERAL_CLOCK			(SYSTEM_CLOCK / 4)UL

#define PERIPHERAL_CLOCK_PERIOD 	(1 / PERIPHERAL_CLOCK)UL



// Direction controle bit is processor specific ,
#define PORT_OUT				(BOOL)(0)
#define PORT_IN					(BOOL)(0xFF)

#define OFF_FOREVER				(BOOL)(0)
#define LOOP_FOREVER			(BOOL)(1)

#define SWITCH_OFF				(BOOL)(0)
#define SWITCH_ON				(BOOL)(1)

#define DISPLAY_DISABLE			(BOOL)(1)
#define DISPLAY_ENABLE			(BOOL)(0)


#define GetSystemClock()		(SYSTEM_CLOCK)      // Hz
#define GetInstructionClock()	(GetSystemClock()/4)
#define GetPeripheralClock()	GetInstructionClock()

#define ENTER_CRITICAL_SECTION()	INTCONbits.GIE = 0;// Disable global interrupt bit.


#define EXIT_CRITICAL_SECTION()		INTCONbits.GIE = 1;// Enable global interrupt bit.

#define ENABLE_GLOBAL_INT()			EXIT_CRITICAL_SECTION()


#define DISABLE_INT0_INTERRUPT()	INTCONbits.INT0IE = 0
#define ENABLE_INT0_INTERRUPT()		INTCONbits.INT0IE = 1
#define CLEAR_INTO_INTERRUPT()		INTCONbits.INT0IF = 0


#define DISABLE_TMR0_INTERRUPT()	INTCONbits.TMR0IE = 0
#define ENABLE_TMR0_INTERRUPT()		INTCONbits.TMR0IE = 1

#define DISABLE_TMR1_INTERRUPT()	PIE1bits.TMR1IE = 0
#define ENABLE_TMR1_INTERRUPT()		PIE1bits.TMR1IE = 1



#define DISABLE_UART2_TX_INTERRUPT()	PIE3bits.TX2IE = 0
#define ENABLE_UART2_TX_INTERRUPT()		PIE3bits.TX2IE = 1

#define DISABLE_UART2_RX_INTERRUPT()	PIE3bits.RC2IE = 0
#define ENABLE_UART2_RX_INTERRUPT()		PIE3bits.RC2IE = 1

#define DISABLE_UART1_TX_INTERRUPT()	PIE1bits.TX1IE = 0
#define ENABLE_UART1_TX_INTERRUPT()		PIE1bits.TX1IE = 1

#define DISABLE_UART1_RX_INTERRUPT()	PIE1bits.RC1IE = 0
#define ENABLE_UART1_RX_INTERRUPT()		PIE1bits.RC1IE = 1

#define DISABLE_UART_RX_INTERRUPT()		PIE1bits.RC1IE = 0
#define ENABLE_UART_RX_INTERRUPT()		PIE1bits.RC1IE = 1

#define ENB_485_TX()	TX_EN = 1;
#define ENB_485_RX()	TX_EN = 0

#define Delay10us(us)		Delay10TCYx(((GetInstructionClock()/1000000)*(us)))
#define DelayMs(ms)												\
	do																\
	{																\
		unsigned int _iTemp = (ms); 								\
		while(_iTemp--)												\
			Delay1KTCYx((GetInstructionClock()+999999)/1000000);	\
	} while(0)

/*
*------------------------------------------------------------------------------
* Public Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables (extern)
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* Public Constants (extern)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Function Prototypes (extern)
*------------------------------------------------------------------------------
*/

extern void BRD_init(void);

#endif
/*
*  End of device.h
*/