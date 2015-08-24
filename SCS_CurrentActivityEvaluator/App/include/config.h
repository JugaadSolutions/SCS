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

#define DEVICE_ADDRESS			0x04



/*----------------------------------------
* Display Configuration (MMD , Scan Digit , Latch Digit)
*----------------------------------------*/
//MMD module configuration
#define MMD_MAX_CHARS		12
#define MMD_MAX_SEGMENTS	2
#define MMD_MAX_ADDRESS		24
#define NO_OF_DIGIT			16
//#define __MMD_STATIC__
//#define MMD_TEST
//#define __DISPLAY_TEST__
//#define COMMON_CATHODE
//#define __DIGIT_DISPLAY_TEST__
			

/*----------------------------------------
*	USART Configuration
*----------------------------------------*/
#define UART2_ACTIVE
#define UART1_BAUD			19200
#define UART2_BAUD			19200
//#define PASS_THROUGH
//#define UART_TEST
///#define UART2TEST
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


#define 	RX_PACKET_SIZE		(50)	
#define 	TX_PACKET_SIZE		(50)


/*----------------------------------------
*	Keypad Configurations
*----------------------------------------*/

//#define __SIMULATION__

/*----------------------------------------
*	RTC CONFIGURATION
*----------------------------------------*/
#define TIME_DEBUG

//#define RTC_DS1307
//#define RTC_DS3232

//#define __SET_RTC__
//#define RTC_DATA_ON_UART


//#define __ERROR_DEBUG__

/*----------------------------------------
*	APP CONFIGURATION
*----------------------------------------*/
#define MSG_LENGTH 		20


#define NO_OF_DATA			40

#define MSG_MAX_CHARS 60
#define MAX_TRANSITIONS 20

/*---------------------------------
*	MODBUS MASTER CONFIGURATION
----------------------------------*/
#define BAUD_RATE	 		19200	// modbus master serial baud rate
#define TIMEOUT		 		10   	// the timeout period is 200msec * 3 = 0.6sec
#define POLLING 			10  	// the scan rate 200msec * 10 = 2sec
#define RETRY_COUNT			5		// number of retries incase of no or error response
#define SLAVE_ID			1		// server ID
#define STARTING_ADDRESS	0


// The total amount of available memory on the master to store data
#define TOTAL_NO_OF_REGISTERS 1

#define	MAX_LOG_ENTRIES  15
#define	LOG_BUFF_SIZE 	 40
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



