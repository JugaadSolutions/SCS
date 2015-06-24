#ifndef _APP_H_
#define _APP_H_


/*
*----------------------------------------------------------------------------------------------------------------
*	MACROS
*-----------------------------------------------------------------------------------------------------------------
*/

//#define __FACTORY_CONFIGURATION__


/*
*----------------------------------------------------------------------------------------------------------------
*	Enumerations
*-----------------------------------------------------------------------------------------------------------------
*/
typedef enum 
{
	OFF,
	ON
}INDICATOR_STATE;

typedef enum _ISSUE_TYPE
{
	NO_ISSUE,
	RAISED,
	RESOLVED
}ISSUE_TYPE;



typedef enum _APP_PARAM
{
	MAX_KEYPAD_ENTRIES = 27,
	MAX_ISSUES = 32,
	MAX_DEPARTMENTS = 20,
	MAX_LOG_ENTRIES = 16,
	LOG_BUFF_SIZE = MAX_KEYPAD_ENTRIES+1

}APP_PARAM;

typedef enum _LOGDATA
{
	HW_TMEOUT = 10,
	APP_TIMEOUT = 1000,
	TIMESTAMP_UPDATE_VALUE = (APP_TIMEOUT/HW_TMEOUT)
}LOGDATA;

typedef enum
{
	ISSUE_RESOLVED,
	ISSUE_RAISED,
	ISSUE_ACKNOWLEDGED,
	ISSUE_CRITICAL
}APP_STATE;

enum
{
	CMD_GET_STATUS = 0x80,
	CMD_GET_ADMIN_PASSWORD = 0x81,
	CMD_GET_LOGON_PASSWORD = 0x82,
	CMD_GET_BUZZER_TIMEOUT = 0x83,


	CMD_SET_ADMIN_PASSWORD = 0x91,
	CMD_SET_LOGON_PASSWORD = 0x92,
	CMD_SET_BUZZER_TIMEOUT = 0x93,
	CMD_SET_BUZZER_ON		= 0x94,
	CMD_SET_BUZZER_OFF		= 0x95,	


	CMD_PING = 0xA0,
	CMD_CLEAR_ISSUES = 0xA1,
	CMD_RESOLVE_ISSUE = 0xA2

	
};

typedef struct _OpenIssue
{
	UINT8 tag[32];
	INT8 ID;
}OpenIssue;

extern void APP_init(void);
extern void APP_task(void);
void APP_TxDataOnModubs(UINT16 id, UINT8 state, UINT8 command);

#endif