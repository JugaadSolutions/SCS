#include "config.h"
#include "communication.h"
#include "uart.h"
#include "timer.h"
#include <usart.h>


enum
{
	IGNORE = 0,
	PARSE_SUCCESS = 1,
	PARSE_FAILURE = 2
};

enum
{
	TIMEOUT = 65535
};

typedef struct _COMMUNICATTION
{
	
	UINT8 state;
	UINT8 rx_sop;
	UINT8 rx_eop;
	UINT8 tx_sop;
	UINT8 tx_eop;
	UINT8 rxPacketBuffer[RX_PACKET_SIZE];
	UINT8 txPacketBuffer[TX_PACKET_SIZE];
	UINT8 rxPacketIndex;
	UINT8 txPacketLength;
	UINT8 txCode;
	UINT8 timeout;
	UINT8 (*callBack)(UINT8* cmd_data, UINT8* respID,  UINT8** resp_data);

	UINT32 prevAppTime, curAppTime;
	UINT8 prevState ;
}COMMUNICATION;

#pragma idata com1_data
COMMUNICATION communication1 = {0};
#pragma idata

UINT16 comTimeout = 0xFFFF;
UINT8 rom alert[]={"COM\n"};


UINT8 COM_BCC( UINT8* data  , UINT8 length);
UINT8 checksum();
UINT8 parse1Packet(UINT8 *respCode);
UINT8 parse2Packet(UINT8 *respCode);
void COM_reset(void);
void COM_txData(void);
void COM2_txData(void);
UINT8 checksum(UINT8 *buffer, UINT8 length);
void COM1_restart(void);

void COM_init(UINT8 rx_sop , UINT8 rx_eop ,UINT8 tx_sop , UINT8 tx_eop , UINT8 (*callBack)(UINT8* rxdata, UINT8* txCode,UINT8** txPacket))
{
	int i;

	//UART1_init();	//initialize uart


	communication1.rx_sop = rx_sop;
	communication1.rx_eop = rx_eop;
	communication1.tx_sop = tx_sop;
	communication1.tx_eop = tx_eop;
	communication1.callBack = callBack;

	COM1_restart();
}

void COM1_restart()
{
#if (defined __18F8722_H) ||(defined __18F46K22_H)
	UART1_init(UART1_BAUD);
#ifdef UART2_ACTIVE
	UART2_init(UART2_BAUD);
#endif
#else
	UART_init();	//initialize uart
#endif

	communication1.rxPacketIndex = 0;
	communication1.txPacketLength = 0;
	communication1.state = COM_START;
	communication1.txCode = IGNORE;
	communication1.timeout = TIMEOUT;
	communication1.prevAppTime = communication1.curAppTime;
	communication1.prevState = communication1.state;
}


void COM1_reset()
{
	communication1.rxPacketIndex = 0;
	communication1.txPacketLength = 0;
	communication1.state = COM_START;
	communication1.txCode = IGNORE;
	communication1.timeout = TIMEOUT;
	communication1.prevAppTime = communication1.curAppTime;
	communication1.prevState = communication1.state;
}



#ifdef __LOOP_BACK__
void COM_task()
{
	UINT8 uartData = 0;


#if(defined __18F8722_H) ||(defined __18F46K22_H)
	if( UART1_hasData() )
	{
		uartData = UART1_read();	

		UART1_write(uartData);
		UART1_transmit();

#ifdef  UART2_ACTIVE
		UART2_write(uartData);
		UART2_transmit();
#endif
		return;

	}
#else
	if( UART_hasData() )
	{
		uartData = UART_read();	

		UART_write(uartData);
		UART_transmit();
		return;

	}

#endif

}
#else


void COM1_task(void)
{
	volatile UINT8 uartData = 0,i;
	communication1.curAppTime = GetAppTime();
	if(RCSTA1bits.OERR == 1)
	{
		RCSTA1bits.CREN = 0;
		Delay10us(1);
		RCSTA1bits.CREN = 1;
	}
#ifdef  UART2_ACTIVE

	if(RCSTA2bits.OERR == 1)
	{
		RCSTA2bits.CREN = 0;
		Delay10us(1);
		RCSTA2bits.CREN = 1;
	}
#endif

	if( communication1.prevAppTime != communication1.curAppTime)
	{
		if( communication1.prevState == communication1.state && (communication1.state == COM_IN_PACKET_COLLECTION))
		{
			--communication1.timeout ;
			if( communication1.timeout == 0)
			{
				//COM_txStr("RESTART");
				COM1_restart();
				return;
			}
			
		}
		
		communication1.prevAppTime = communication1.curAppTime;
	}

	switch( communication1.state)
	{
		case COM_START:

#if(defined __18F8722_H) ||(defined __18F46K22_H)
			if( UART1_hasData() == FALSE )
				return;
		
			uartData = UART1_read();
#ifdef	PASS_THROUGH
			UART2_write(uartData);
			UART2_transmit();
#endif	
#else
			if( UART1_hasData() == FALSE )
				return;
			uartData = UART_read();	

			
#endif

			if( uartData == communication1.rx_sop )
			{
				communication1.rxPacketIndex = 0;
				communication1.state = COM_IN_PACKET_COLLECTION;
			}
		break;

		case COM_IN_PACKET_COLLECTION:

#if(defined __18F8722_H) ||(defined __18F46K22_H)

			if( UART1_hasData()==FALSE )
				return;
			uartData = UART1_read();
#ifdef PASS_THROUGH
			UART2_write(uartData);
			UART2_transmit();
#endif	
#else	

			if( UART_hasData()==FALSE )
				return;
			uartData = UART_read();	
#endif
		
			if(uartData == communication1.rx_eop )
			{
				UINT8 parseResult = 0;
				COM_RESP_CODE txCode = COM_RESP_NONE;
				UINT8 *txData ;

#ifdef __NO_CHECKSUM__
				parseResult = PARSE_SUCCESS;
#else				
				parseResult = parse1Packet(&txCode);		//parse packet 
#endif

				switch( parseResult)
				{
					case IGNORE:
					COM1_reset();	
					return;
					
					case PARSE_SUCCESS:
											
					if( communication1.callBack != 0 )
					{
						communication1.txPacketLength = 
							communication1.callBack(&communication1.rxPacketBuffer[COM_RX_DATA_START_INDEX], 
											&communication1.txCode,
													  &txData);
						if(communication1.txCode == COM_RESP_NONE )
						{
								COM1_reset();
								return;
						}
						communication1.txPacketBuffer[COM_DEVICE_ADDRESS_INDEX] = DEVICE_ADDRESS;	//store device address
						++communication1.txPacketLength;

						communication1.txPacketBuffer[COM_TX_CODE_INDEX] = communication1.txCode;	//store tx code
						++communication1.txPacketLength;

						for( i = COM_TX_DATA_START_INDEX ; i < communication1.txPacketLength ; i++)	//store data
						{
							communication1.txPacketBuffer[i] = *txData;
							txData++;
						}

					}

					else
					{
						COM1_reset();
					}

					break;
					
					case PARSE_FAILURE:
					{

						communication1.txPacketBuffer[COM_DEVICE_ADDRESS_INDEX] = DEVICE_ADDRESS;	//store device address
						++communication1.txPacketLength;
						
						communication1.txPacketBuffer[COM_TX_CODE_INDEX] = txCode;		//store tx code
						++communication1.txPacketLength;
						
					}
					
					break;
					
					default:
					break;
				}
				communication1.state = COM_IN_TX_DATA;
			}
			else
			{
				communication1.rxPacketBuffer[communication1.rxPacketIndex++]=uartData;
				communication1.timeout = 0;
				if( communication1.rxPacketIndex >= RX_PACKET_SIZE)
				{
					communication1.txPacketBuffer[COM_DEVICE_ADDRESS_INDEX] = DEVICE_ADDRESS;	//store device address
					++communication1.txPacketLength;

					communication1.txPacketBuffer[COM_TX_CODE_INDEX] = COM_RESP_OVERRUN;		//store tx code
					++communication1.txPacketLength;
					
					communication1.state = COM_IN_TX_DATA;
					
				}
			}
			break;

		case COM_IN_TX_DATA:

			COM_txData();

			COM1_reset();
	
		break;

		default:
			COM1_reset();
		break;

	}
	communication1.prevState = communication1.state;

}

#endif



UINT8 parse1Packet(UINT8 *respCode)
{

#ifdef __NO_CHECKSUM__
	return PARSE_SUCCESS;
#else
	UINT8 receivedChecksum = communication1.rxPacketBuffer[communication1.rxPacketIndex-1];
	UINT8 genChecksum = 0;


	if((communication1.rxPacketBuffer[ COM_DEVICE_ADDRESS_INDEX] != DEVICE_ADDRESS)
			&& (communication1.rxPacketBuffer[COM_DEVICE_ADDRESS_INDEX] != BROADCAST_ADDRESS) )
		return IGNORE;
	
	genChecksum = checksum(communication1.rxPacketBuffer,communication1.rxPacketIndex-1);
	
	if( receivedChecksum == genChecksum)
	{
		--communication1.rxPacketIndex;
		communication1.rxPacketBuffer[communication1.rxPacketIndex] = '\0'; //remove checksum from packet
	 
		return PARSE_SUCCESS;
	}
	else
	{	
		*respCode = COM_RESP_CHECKSUM_ERROR;
	 	return PARSE_FAILURE;
	}
#endif
}



void COM_txData()
{
	UINT8 bcc = 0;
	UINT8 i= 0;

	bcc = checksum(communication1.txPacketBuffer, communication1.txPacketLength);

#if(defined __18F8722_H) ||(defined __18F46K22_H)

	UART1_write(communication1.tx_sop);

	for( i = 0; i < communication1.txPacketLength; i++ )
	{
		UART1_write(communication1.txPacketBuffer[i]);
	}

	UART1_write(bcc);
	UART1_write(communication1.tx_eop);


#ifdef __RESPONSE_ENABLED__
	UART1_transmit();
#endif


#else 	//(defined __18F8722_H) ||(defined __18F46K22_H)
	
	UART_write(communication1.tx_sop);

	for( i = 0; i < communication1.txPacketLength; i++ )
	{
		UART_write(communication1.txPacketBuffer[i]);
	}

	UART_write(bcc);
	UART_write(communication1.tx_eop);


#ifdef __RESPONSE_ENABLED__
	UART_transmit();
#endif
	ClrWdt();

#endif
	
}





void COM_txStr(rom UINT8 *str)
{
#if(defined __18F8722_H) ||(defined __18F46K22_H)

	while(*str)
	{
		UART1_write(*str);
		str++;
	}
	UART1_transmit();

#else

	while(*str)
	{
		UART_write(*str);
		str++;
	}
	UART_transmit();
#endif
}


UINT8 checksum(UINT8 *buffer, UINT8 length)
{
	
	UINT8 bcc = 0;
	UINT8 i , j ;
	
#ifdef __BCC_LRC__

	for( i = 0 ; i < length ; i++)
	{
		bcc += buffer[i];
	}
	return bcc;

#elif defined __BCC_XOR__

	for( i = 0; i < length; i++)
	{
		bcc ^=buffer[i];
	}
	return bcc;

#endif
}		




void COM_txCMD_CHAN1(UINT8 deviceAddress, 
			UINT8 cmd, UINT8 *buffer , UINT8 length)
{
	UINT8 cmdPacket[25] = {0};
	UINT8 i,j,cs;

	i = 0;
	cmdPacket[i++]= CMD_SOP;
	cmdPacket[i++] = deviceAddress+2;
	cmdPacket[i++] = length;
	cmdPacket[i++] = cmd;
	for( j =0; j < length ; j++)
	{
		cmdPacket[i+j] = buffer[j];
	}
	i+= j;
 	cs = checksum(&cmdPacket[1], i - 1 );
	while((cs == CMD_SOP ) || (cs == CMD_EOP)) //if check sum matches sop or eop
	{
		cmdPacket[2]++; 						// change length
		cs = checksum(&cmdPacket[1], i - 1 ); //recalculate check sum
	}
	cmdPacket[i++] = cs;
	cmdPacket[i++] = CMD_EOP;

	for( j = 0 ; j < i ; j++)
	{
		UART1_write(cmdPacket[j]);
	}
	UART1_transmit();

}
	