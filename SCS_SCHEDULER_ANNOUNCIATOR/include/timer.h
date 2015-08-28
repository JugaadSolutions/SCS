#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

/*
*------------------------------------------------------------------------------
* timer_driver.h
*
* Include file for timer_driver module.
*/
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include "board.h"
#include <timers.h>

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
#define TMR0_RELOAD	
#define TMR0_FUNC	SCH_update
// For 2ms display interrupt
// 2ms = (4/20000000Hz) * 10000
//#define DISP_ISR_PERIOD	(65535 - 10000)

// 2ms = (4/40000000Hz) * 20000
//#define DISP_ISR_PERIOD	(65535 - 7500)
#define TMR1_RELOAD		(65535 - 7500)
#define TMR1_FUNC		



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

extern UINT16 tickPeriod;
extern UINT16 heartBeatCount;
extern UINT16 AppUpdate_count ;
extern UINT16 comUpdateCount;
extern UINT16 mmdUpdateCount ;
extern UINT16 eMBUpdate_count  ;

/*
*------------------------------------------------------------------------------
* Public Function Prototypes (extern)
*------------------------------------------------------------------------------
*/

extern void TMR0_init(unsigned int reload , void (*func)());
extern void TMR1_init(unsigned int reload , void (*func)());
void TMR1_ISR(void);
void TMR0_ISR(void);
void TMR3_init(unsigned int reload , void (*func)());

#endif
/*
*  End of timer_driver.h
*/