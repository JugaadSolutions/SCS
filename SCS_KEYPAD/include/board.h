#ifndef __BOARD__
#define __BOARD__

/*
*------------------------------------------------------------------------------
* board.h
*
* Include file for port pin assignments
*

*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* File				: board.h
*------------------------------------------------------------------------------
*

*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include <p18f46k22.h>
#include <delays.h>
#include <timers.h>
#include "typedefs.h"
#include "config.h"


/*
*------------------------------------------------------------------------------
* Hardware Port Allocation
*------------------------------------------------------------------------------
*/




// HeartBeat Association
#define		HEART_BEAT				PORTEbits.RE2			// high - OFF, Low - ON
#define		HEART_BEAT_DIR			TRISEbits.TRISE2




//  UART  Association
#define 	TX1_EN					PORTCbits.RC0 		// serial transmit
#define		TX1_EN_DIR				TRISCbits.TRISC0
#define 	TX2_EN					PORTCbits.RC4 		// serial transmit
#define		TX2_EN_DIR				TRISCbits.TRISC4
#define 	SER1_TX					PORTCbits.RC6 		// serial transmit
#define		SER1_TX_DIR				TRISCbits.TRISC6
#define 	SER1_RX					PORTCbits.RC7			// serial receive
#define		SER1_RX_DIR				TRISCbits.TRISC7






// LCD Association
#define		LCD_DAT4_PORT			LATD

#define		LCD_D7					PORTDbits.RD7 			
#define		LCD_D7_DIR				TRISDbits.TRISD7
#define		LCD_D6					PORTDbits.RD6 			
#define		LCD_D6_DIR				TRISDbits.TRISD6
#define		LCD_D5					PORTDbits.RD5 			
#define		LCD_D5_DIR				TRISDbits.TRISD5
#define		LCD_D4					PORTDbits.RD4			
#define		LCD_D4_DIR				TRISDbits.TRISD4
#define		LCD_E					PORTDbits.RD1 			
#define		LCD_E_DIR				TRISDbits.TRISD1
#define		LCD_RW					PORTDbits.RD2 			
#define		LCD_RW_DIR				TRISDbits.TRISD2
#define		LCD_RS					PORTDbits.RD3			
#define		LCD_RS_DIR				TRISDbits.TRISD3

/* Keypad Association */

#define		KEYPAD_DEC_INT			PORTAbits.RA1		
#define		KEYPAD_DEC_INT_DIR		TRISAbits.TRISA1
#define		KEYPAD_BCD3				PORTAbits.RA4	
#define		KEYPAD_BCD3_DIR			TRISAbits.TRISA4
#define		KEYPAD_BCD2				PORTAbits.RA5
#define		KEYPAD_BCD2_DIR			TRISAbits.TRISA5
#define		KEYPAD_BCD1				PORTEbits.RE0		
#define		KEYPAD_BCD1_DIR			TRISEbits.TRISE0
#define		KEYPAD_BCD0				PORTEbits.RE1		
#define		KEYPAD_BCD0_DIR			TRISEbits.TRISE1

//Tower Lamp Association

#define LAMP_GREEN						PORTBbits.RB5
#define LAMP_GREEN_DIR					TRISBbits.TRISB5

#define LAMP_YELLOW 						PORTBbits.RB4 
#define LAMP_YELLOW_DIR					TRISBbits.TRISB4


#define LAMP_RED							PORTBbits.RB3
#define LAMP_RED_DIR						TRISBbits.TRISB3

#define BUZZER 						PORTDbits.RD0
#define BUZZER_DIR					TRISDbits.TRISD0




			
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



#define SYSTEM_CLOCK			(16000000UL)	// 16MHz internal oscillator	

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

extern void BOARD_init(void);

#endif
/*
*  End of device.h
*/