#ifndef _APP_H_
#define _APP_H_


/*
*----------------------------------------------------------------------------------------------------------------
*	MACROS
*-----------------------------------------------------------------------------------------------------------------
*/

//#define __FACTORY_CONFIGURATION__
#define MAX_NO_OF_TRUCKS	16

/*
*----------------------------------------------------------------------------------------------------------------
*	Enumerations
*-----------------------------------------------------------------------------------------------------------------
*/
enum 
{
	RUNNING = 0,
	CANCELLED
};



typedef enum _APP_PARAM
{
	MAX_KEYPAD_ENTRIES = 3,
	MAX_ISSUES = 32,
	MAX_DEPARTMENTS = 5,
	MAX_LOG_ENTRIES = 15,
	LOG_BUFF_SIZE = 16

}APP_PARAM;

typedef enum
{
	PICKING_START = 0,
	PICKING_END,
	STAGING_START,
	STAGING_END,
	LOADING_START,
	LOADING_END,
	MAX_STATES
}TRUCK_STATE;


extern void APP_init(void);
extern void APP_task(void);
BOOL APP_checkPassword(UINT8 *password);
void App_updateLog(far UINT8 *data);
BOOL APP_activityValid(UINT8 *buffer);

void APP_managePicking(UINT8 *buffer);
UINT8 APP_validatePicking(UINT8 *buffer);

void APP_manageStaging(UINT8 *buffer);
UINT8 APP_validateStaging(UINT8 *buffer);

void APP_manageLoading(UINT8 *buffer);
UINT8 APP_validateLoading(UINT8 *buffer);

void APP_cancelTruck(UINT8 *buffer);

#endif
