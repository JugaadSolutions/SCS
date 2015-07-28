#include "config.h"
#include "mmd.h"
#include "string.h"

#define MAX_SIZE							40

#define TRUCKS_SUPPORTED					4
#define ACTIVITIES_SUPPORTED				3
#define TRUCK_STATUS_BASE					8

#define ACTIVITY_PARAMETER_BUFFER_SIZE		16


typedef struct _ACTIVITY_SCHEDULE
{
	UINT16 startMinute;
	UINT16 endMinute;
	UINT16 duration;
}ACTIVITY_SCHEDULE;



#define EEP_DEVICE_ID							(UINT16)(0000)
#define EEP_TIME_FORMAT							(UINT16)(0001)
#define EEP_SHIPMENT_SCHEDULE_BASE_ADDRESS		(UINT16)(0002)





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
	CMD_HOOTER_OFF		= 0X89,


	CMD_SET_SCHEDULE = 0x70,
	CMD_UPDATE_SHIPMENT_SCHEDULE = 0x71,
	CMD_SET_SEGMENT = 0x72,
	CMD_CLEAR_SEGMENT = 0x73,
	CMD_RESET = 0x74,
	CMD_GET_COMM_STATUS = 0x75
};

/*
*-----------------------------------------------------------
* shipment announciator
*------------------------------------------------------------
*/
#define CURRENT_ACTIVITY_SEGMENTS			3

#define TRUCKS_SUPPORTED_ANN				16+1
#define ACTIVITIES_SUPPORTED_ANN			3
#define BREAKS_SUPPORTED					9


enum
{
	PARAMETER_TRUCK_INDEX = 0,
	PARAMETER_TRUCK_LENGTH = 2,

	PARAMETER_ACTIVITY_INDEX = PARAMETER_TRUCK_INDEX + PARAMETER_TRUCK_LENGTH,
	PARAMETER_ACTIVITY_LENGTH = 8,

	PARAMETER_STATUS_INDEX = PARAMETER_ACTIVITY_INDEX + 	PARAMETER_ACTIVITY_LENGTH,
	PARAMETER_STATUS_LENGTH = 2,

	PARAMETER_PLAN_PROGRESS_INDEX = PARAMETER_STATUS_INDEX +	PARAMETER_STATUS_LENGTH,
	PARAMETER_PLAN_PROGRESS_LENGTH = 6,

	PARAMETER_PLAN_PERCENTAGE_INDEX = PARAMETER_PLAN_PROGRESS_INDEX +PARAMETER_PLAN_PROGRESS_LENGTH,
	PARAMETER_PLAN_PERCENTAGE_LENGTH = 2,

	PARAMETER_ACTUAL_PROGRESS_INDEX = PARAMETER_PLAN_PERCENTAGE_INDEX +	PARAMETER_PLAN_PERCENTAGE_LENGTH,
	PARAMETER_ACTUAL_PROGRESS_LENGTH = 6,

	PARAMETER_ACTUAL_PERCENTAGE_INDEX = PARAMETER_ACTUAL_PROGRESS_INDEX +PARAMETER_ACTUAL_PROGRESS_LENGTH,
	PARAMETER_ACTUAL_PERCENTAGE_LENGTH = 6
};



enum
{
	CURRENT_ACTIVITY_DEVICE_START_ADDRESS = 5
};





typedef enum
{
	APP_STATE_NONE = 0,
	APP_STATE_ACTIVE = 1,
	APP_STATE_INACTIVE = 2
}APP_STATE;		



UINT8 APP_comCallBack( far UINT8 *rxPacket,  far UINT8* txCode, far UINT8** txPacket);
void APP_init(void);
void APP_task(void);

