
#ifndef __COMMUNICATION__
#define __COMMUNICATION__

#include "board.h"


typedef enum
{
	COM_RESP_NONE = 0xFF,
	COM_RESP_OK = 0,
	COM_RESP_OVERRUN = 0xF0,
	COM_RESP_CHECKSUM_ERROR = 0xF1,
	COM_RESP_INVALID_CMD = 0xF2, 
	COM_RESP_INVALID_DATA = 0xF3
}COM_RESP_CODE;






enum 
{ 
	COM_RESET = 0,
	COM_START = 1,
	COM_IN_PACKET_COLLECTION = 2,
	COM_IN_TX_DATA = 3
};




enum 
{
	COM_DEVICE_ADDRESS_INDEX = 0,
	COM_TX_CODE_INDEX = 1,
	COM_TX_DATA_START_INDEX = 2,
	COM_RX_DATA_START_INDEX = 2
};





void COM_init(UINT8 cmd_sop , UINT8 cmd_eop ,UINT8 resp_sop , UINT8 resp_eop , UINT8 (*callBack)(far UINT8* rxdata,far UINT8* txCode,far UINT8** txPacket));


void COM_txStr(rom UINT8 *str);
UINT8 COM_txBuffer(far UINT8 *txData, UINT8 length);
void COM_txCMD_CHAN1(UINT8 deviceAddress, 
			UINT8 cmd, far UINT8 *buffer , UINT8 length);
void COM1_reset(void);
void COM2_reset(void);
void COM1_task(void);
void COM2_task(void);
#endif


