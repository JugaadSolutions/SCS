#ifndef APP_H
#define APP_H

/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include "config.h"
#include "mb.h"
#include "modbusMaster.h"
#include "mmd.h"
#include "string.h"
#include "typedefs.h"
#include "eep.h"
#include "rtc_driver.h"
#include "math_fun.h"
#include "digit_driver.h"
#include "digitdisplay.h"
#include "uart.h"
/*
*------------------------------------------------------------------------------
* Private Macros
*------------------------------------------------------------------------------
*/

#define	MARQUEE_SEGMENT_START_ADDRESS		0
#define	MARQUEE_SEGMENT_CHARS				30
#define MARQUEE_SEGMENT_LENGTH				10

#define TIME_SEGMENT_START_ADDRESS		(MARQUEE_SEGMENT_START_ADDRESS+MARQUEE_SEGMENT_LENGTH)
#define TIME_SEGMENT						5
#define	TIME_SEGMENT_CHARS					6

#define TIME_HOUR_INDEX							0
#define TIME_SECOND_INDEX						2
#define TIME_MINUTE_INDEX						4
				
#define BACKLIGHT_SEGMENT_START_ADDRESS (TIME_SEGMENT_START_ADDRESS) + TIME_SEGMENT_CHARS
#define BACKLIGHT_SEGMENT_CHARS				5
#define BACKLIGHT_TRUCK_INDEX				 TIME_MINUTE_INDEX +2
#define BACKLIGHT_PICKING_INDEX				BACKLIGHT_TRUCK_INDEX + 1
#define BACKLIGHT_STAGING_INDEX				BACKLIGHT_PICKING_INDEX + 1
#define BACKLIGHT_LOADING_INDEX				BACKLIGHT_STAGING_INDEX + 1
#define BACKLIGHT_STATUS_INDEX				BACKLIGHT_LOADING_INDEX + 1

#define CURRENT_ACTIVITY_SEGMENTS			3

#define TRUCKS_SUPPORTED					16
#define TRUCKS_SUPPORTED_BOARD				4
#define ACTIVITIES_SUPPORTED				3
#define BREAKS_SUPPORTED					8
#define TRUCK_STATUS_BASE					8



#define DELAY_PERCENTAGE				10
#define ALARM_PERCENTAGE				25

#define ACTIVITY_PARAMETER_BUFFER_SIZE			20
#define MARQUEES_SUPPORTED				(BREAKS_SUPPORTED)

//#define EEP_DEVICE_ID							(UINT16)(0000)
//#define EEP_TIME_FORMAT							(UINT16)(0001)
#define EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS		(UINT16)(0208)  //0xD0
#define EEP_BREAK_SCHEDULE_BASE_ADDRESS 		(UINT16)(EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS + (sizeof(ACTIVITY_SCHEDULE) * ACTIVITIES_SUPPORTED*(TRUCKS_SUPPORTED+1)))
#define EEP_DELAY_PERCENTAGE					(UINT16)(EEP_BREAK_SCHEDULE_BASE_ADDRESS + (sizeof(ACTIVITY_SCHEDULE) *TRUCKS_SUPPORTED))
#define	EEP_ALARM_PERCENTAGE					(UINT16)(EEP_DELAY_PERCENTAGE + 1)

/*
*------------------------------------------------------------------------------
* Private Enumeration
*------------------------------------------------------------------------------
*/

enum
{
	CURRENT_ACTIVITY_DEVICE_START_ADDRESS = 5
};


enum
{
	TIMEOUT_COUNT = 1
};


typedef enum
{
	APP_STATE_NONE = 0,
	APP_STATE_ACTIVE = 1,
	APP_STATE_INACTIVE = 2
}APP_STATE;		



typedef enum
{
	ACTIVITY_NONE = 0,
	ACTIVITY_PICKING = 1,
	ACTIVITY_STAGING = 2,
	ACTIVITY_LOADING = 3,
	ACTIVITY_CANCEL = 4
}ACTIVITY; 


typedef enum
{
	ACTIVITY_SCHEDULED = 0,
	ACTIVITY_ONGOING = 1,
	ACTIVITY_COMPLETED = 2,
	ACTIVITY_CANCELLED = 3
}ACTIVITY_STATUS;




typedef enum
{
	MILESTONE_NONE = 0,
	MILESTONE_START = 1,
	MILESTONE_END = 2
	
}MILESTONE;

typedef enum
{
	RESET = 0,
	ON_TIME = 1,
	DELAYED = 2
}STATUS;


typedef enum
{
	CMD_PICKING_START	= 0x80,
	CMD_PICKING_END		= 0x81,
	CMD_STAGING_START	= 0x82,
	CMD_STAGING_END		= 0x83,
	CMD_LOADING_START	= 0x84,
	CMD_LOADING_END		= 0x85,
	CMD_TRUCK_TIMINGS	= 0x86,

	CMD_RTC				= 0x87,
	CMD_CANCEL_TRUCK	= 0x88,
	CMD_HOOTER_OFF		= 0X89


};

typedef enum
{
	CMD_SET_SCHEDULE = 0x70,
	CMD_UPDATE_SHIPMENT_SCHEDULE = 0x71,
	CMD_SET_SEGMENT = 0x72,
	CMD_CLEAR_SEGMENT = 0x73,
	CMD_RESET = 0x74,
	CMD_GET_COMM_STATUS = 0x75

};

/*
*------------------------------------------------------------------------------
* app - the app structure. 
*------------------------------------------------------------------------------
*/
typedef struct _ACTIVITY_SCHEDULE
{
	UINT16 startMinute;
	UINT16 endMinute;
	UINT16 duration;
}ACTIVITY_SCHEDULE;

typedef struct _CurrentActivitySegment
{
	UINT8 no;
	ACTIVITY activity;
	STATUS status;
	UINT8 planProgress;
	INT32 planPercentage;
	UINT8 actualProgress;
	INT32 actualPercentage;
	ACTIVITY_SCHEDULE planSchedule;
	ACTIVITY_SCHEDULE actualSchedule;
	UINT8 free;
}CurrentActivitySegment;


typedef struct _ACTIVITY_TRIGGER_DATA
{
	UINT8 truck;
	ACTIVITY activity;
	MILESTONE mileStone;
}ACTIVITY_TRIGGER_DATA;	

typedef struct _PICKING_INFO
{
	UINT8 truck;
	UINT8 state;
	UINT8 timeout;
}PICKING_INFO;



typedef struct _TRUCK_SCHEDULE
{
	ACTIVITY_SCHEDULE schedule[ACTIVITIES_SUPPORTED];
}TRUCK_SCHEDULE;

typedef struct _SCHEDULE_UPDATE_INFO
{
	UINT8 truck;
	ACTIVITY activity;
	MILESTONE milestone;
	STATUS status;
	UINT8 startMinute_MSB;
	UINT8 startMinute_LSB;
	UINT8 endMinute_MSB;
	UINT8 endMinute_LSB;
	UINT8 curMinute_MSB;
	UINT8 curMinute_LSB;
}SCHEDULE_UPDATE_INFO;
 
typedef struct _DISPLAY_SCANNING
{
	//used to store status of truck
	UINT8 buffer[TRUCKS_SUPPORTED*2];
}DISPLAY_SCANNING;

typedef struct _SCHEDULE_DATA
{
	UINT8 truck;
	ACTIVITY_SCHEDULE schedule[ACTIVITIES_SUPPORTED];
}SCHEDULE_DATA;

typedef struct _SCHEDULE_STATUS
{
	ACTIVITY_STATUS activityStatus;
	STATUS status;
}SCHEDULE_STATUS;

typedef struct _TRUCK_INDICATOR_DATA
{
	UINT8 indicatorRed[4];
	UINT8 indicatorGreen[4];
}TRUCK_INDICATOR_DATA;

//Modbus Master
typedef struct _LOG
{
	UINT8 slaveID[MAX_LOG_ENTRIES];
	UINT8 regCount[MAX_LOG_ENTRIES];
	UINT8 prevIndex;
	UINT8 writeIndex;
	UINT8 readIndex;
	UINT16 entries[MAX_LOG_ENTRIES][LOG_BUFF_SIZE];
}LOG;




/*
*------------------------------------------------------------------------------
* Public Functions	Prototypes
*------------------------------------------------------------------------------
*/
UINT8 APP_comCallBack( far UINT8 *rxPacket,  far UINT8* txCode, far UINT8** txPacket);
void APP_init(void);
void APP_task(void);

#endif

