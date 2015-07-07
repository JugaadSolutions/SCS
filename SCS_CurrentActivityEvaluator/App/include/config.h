#ifndef CONFIG_H
#define CONFIG_H

/*
*------------------------------------------------------------------------------
* config.h
*
*/


#define DEVICE_ADDRESS			0x01


//MMD module configuration
#define MMD_MAX_CHARS		11
#define MMD_MAX_SEGMENTS	2
#define MMD_MAX_ADDRESS		32
//#define __MMD_STATIC__
//#define MMD_TEST
//#define __DISPLAY_TEST__
//#define COMMON_CATHODE
/*----------------------------------------
*	TIMER Configuration
*----------------------------------------*/

#define TIMESTAMP_DURATION 		(200)			

/*----------------------------------------
*	USART Configuration
*----------------------------------------*/
#define ACTIVE_USARTS		1
//#define UART2_ACTIVE
#define UART1_BAUD			19200
#define UART2_BAUD			19200
//#define PASS_THROUGH
#define UART_TEST

/*----------------------------------------
*	COM module configuration
*----------------------------------------*/

//#define __NO_CHECKSUM__
#define __BCC_XOR__
#define __RESPONSE_ENABLED__
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

#define 	RX_PACKET_SIZE		(30)	
#define 	TX_PACKET_SIZE		(30)


/*----------------------------------------
*	Keypad Configurations
*----------------------------------------*/

//#define __FACTORY_CONFIGURATION__
//#define __SIMULATION__

/*----------------------------------------
*	RTC CONFIGURATION
*----------------------------------------*/


//RTC CONFIGURATION
//#define TIME_DEBUG

//#define RTC_DS1307
#define RTC_DS3232

//#define __SET_RTC__
//#define RTC_DATA_ON_UART


//#define __ERROR_DEBUG__

/*----------------------------------------
*	APP CONFIGURATION
*----------------------------------------*/
#define MSG_LENGTH 		20


#define NO_OF_DATA			25

#define MSG_MAX_CHARS 60
#define MAX_TRANSITIONS 20


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



