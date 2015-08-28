#ifndef CONFIG_H
#define CONFIG_H

/*
*------------------------------------------------------------------------------
* config.h
*
*/


/*----------------------------------------
*	BOARD MOULE CONFIG
*----------------------------------------*/
#define MHz_32					(32000000UL)	// Hz
#define MHz_16					(16000000UL)	// Hz
#define MHz_8					(8000000UL)	// Hz

/*----------------------------------------
*	OSCILLATOR CONFIG
*----------------------------------------*/

#define SYSTEM_CLOCK			(MHz_32)


/*----------------------------------------
*	TIMER CONFIG
*----------------------------------------*/
#define FULLSCALE_16BIT				(65535)
#define TIMER0_TIMEOUT_DURATION 	(1000UL)			//1ms
#define TIMESTAMP_DURATION 		(200)			

/*----------------------------------------
*	DEVICE CONFIG
*----------------------------------------*/
#define __FACTORY_CONFIGURATION__

#define DEVICE_ADDRESS			0x01


/*----------------------------------------
* Display Configuration (MMD , Scan Digit , Latch Digit)
*----------------------------------------*/
#define MMD_MAX_CHARS		28
#define MMD_MAX_SEGMENTS	1
#define MMD_MAX_ADDRESS		28
#define NO_OF_DIGIT			16
//#define __MMD_STATIC__
//#define MMD_TEST
//#define COMMON_CATHODE
//#define __DISPLAY_TEST__ 

//#define __DIGIT_DISPLAY_TEST__

/*----------------------------------------
*	USART Configuration
*----------------------------------------*/
#define ACTIVE_USARTS		1
//#define UART2_ACTIVE
#define UART1_BAUD			9600
#define UART2_BAUD			19200
//#define PASS_THROUGH
//#define UART_TEST

/*----------------------------------------
*	COM module configuration
*----------------------------------------*/

//#define __NO_CHECKSUM__
#define __BCC_XOR__
//#define __RESPONSE_ENABLED__
//#define __LOOP_BACK__
#define BROADCAST_ADDRESS		0xFF
#define CMD_SOP	0xAA
#define CMD_EOP 0xBB
#define RESP_SOP	0xCC
#define RESP_EOP	0xDD


enum
{
	CMD_PACKET_SIZE = 30,
	RESP_PACKET_SIZE = 30
};

#define 	RX_PACKET_SIZE		(60)	
#define 	TX_PACKET_SIZE		(60)


/*----------------------------------------
*	Keypad Configurations
*----------------------------------------*/

//#define __SIMULATION__

/*----------------------------------------
*	RTC CONFIGURATION
*----------------------------------------*/
//#define TIME_DEBUG

//#define RTC_DS1307
//#define RTC_DS3232

//#define __SET_RTC__
//#define RTC_DATA_ON_UART


//#define __ERROR_DEBUG__

/*----------------------------------------
*	APP CONFIGURATION
*----------------------------------------*/


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

#endif
/*
*  End of config.h
*/



