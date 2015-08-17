#include "mutex.h"
#include "board.h"

#pragma idata MUTEX_DATA
UINT8 semaphore = 1;
#pragma idata

UINT8 mutex_lock( void )
{
	if( semaphore == 1 )
	{	
		semaphore = 0;
		return 1;
	}
	else
	{	
		return 0;
	}
}

void mutex_unlock( void )
{

	if( semaphore == 0 )
	{				
		semaphore = 1;
	}

}

