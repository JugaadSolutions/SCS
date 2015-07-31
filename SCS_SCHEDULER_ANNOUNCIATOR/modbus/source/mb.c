
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"



/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbfunc.h"
#include "timer.h"
#include "uart.h"

/* ----------------------- Static variables ---------------------------------*/

enum
{
	TIMEOUT = 5
};

static rom const UCHAR aucCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static rom const UCHAR aucCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};




#pragma idata MB_SALVE
sMB_SLAVE sMBslave = {0};
#pragma idata




/*
*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

UCHAR getOffset( void );

eMBErrorCode
eMBRTUSend( UCHAR ucSlaveAddress, const UCHAR * pucFrame, USHORT usLength );

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBInit( UCHAR ucSlaveAddress,ULONG ulBaudRate )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    /* check preconditions */
	if( ( ucSlaveAddress == MB_ADDRESS_BROADCAST ) ||
	    ( ucSlaveAddress < MB_ADDRESS_MIN ) || ( ucSlaveAddress > MB_ADDRESS_MAX ) )
	{
	    eStatus = MB_EINVAL;
	}
	else
	{
	    sMBslave.ucMBAddress = ucSlaveAddress;
	
	    /* Modbus RTU uses 8 Databits. */
#ifdef __p18f4520
	    UART_init( ulBaudRate );
#else
		UART1_init( ulBaudRate );
#endif
	
		/* If baudrate > 19200 then we should use the fixed timer values
		 * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
		 */
		if( ulBaudRate > 19200 )
		{
		    sMBslave.usTimerT35_50us = (17500);       /* 1750us. */
			sMBslave.uTimerT15 = 750; 				// 750us
		}
		else
		{
		    sMBslave.usTimerT35_50us = 35000000/ulBaudRate; // 1T * 1.5 = T1.5
			sMBslave.uTimerT15 = 15000000/ulBaudRate; // 1T * 3.5 = T3.5
		}

	}

	sMBslave.prevState = sMBslave.state;

    return eStatus;
}


eMBErrorCode
eMBPoll( void )
{
    static UCHAR    ucRcvAddress;
    static UCHAR    ucFunctionCode;
    static USHORT   usLength;
    static eMBException eException;
	static USHORT	crc;
	UCHAR 			offSet;

    eMBErrorCode    eStatus = MB_ENOERR;

	sMBslave.curAppTime = GetAppTime();
	if( sMBslave.prevAppTime != sMBslave.curAppTime )
	{
		if( (sMBslave.prevState == sMBslave.state) && (sMBslave.state == COM_IN_PACKET_COLLECTION) )
		{
			--sMBslave.timeout ;
			if( sMBslave.timeout == 0)
			{
				sMBslave.state = COM_START;
				return MB_ERR;
			}
			
		}
		
		sMBslave.prevAppTime = sMBslave.curAppTime;
	}

	switch ( sMBslave.state )
	{
        case COM_START:
			if(RCSTA1bits.OERR == 1)
			{
				RCSTA1bits.CREN = 0;
				Delay10us(1);
				RCSTA1bits.CREN = 1;
			}
	
#ifdef __p18f4520
			//if uart has data change the state to collect frame
			if( UART_hasData(  ) )
#else
			if( UART1_hasData(  ) )
#endif
			{
				sMBslave.usRcvBufferPos = 0;
				memset( sMBslave.ucRTUBuf, 0X00, MB_SER_PDU_SIZE_MAX );
#ifdef __p18f4520
        		sMBslave.ucRTUBuf[sMBslave.usRcvBufferPos++] = UART_read(  );
#else
        		sMBslave.ucRTUBuf[sMBslave.usRcvBufferPos++] = UART1_read(  );
#endif
				sMBslave.state = COM_IN_PACKET_COLLECTION;

				sMBslave.timeout = TIMEOUT;
			}
			
            break;

		case COM_IN_PACKET_COLLECTION:

#ifdef __p18f4520
			if( UART_hasData(  ) )
#else
			if( UART1_hasData(  ) )
#endif
			{
#ifdef __p18f4520
	        	sMBslave.ucRTUBuf[sMBslave.usRcvBufferPos++] = UART_read(  );
#else
	        	sMBslave.ucRTUBuf[sMBslave.usRcvBufferPos++] = UART1_read(  );
#endif
			}

			else 
			{	
				//get offset in order to determine the size of pdu
				//for function code less than 6 offset is zero
				//for function code 10 and F offset is (byte counts + 1)
				offSet = getOffset(  );
				if ( sMBslave.usRcvBufferPos >= ( MB_SER_PDU_SIZE_MIN 
						+ MB_SER_PDU_SIZE_CRC + offSet ) )
				
				sMBslave.state = COM_FRAME_RECEIVED;
			}

			break;

        case COM_FRAME_RECEIVED:

			ucRcvAddress = sMBslave.ucRTUBuf[MB_PDU_SLAVE_ID];

			// Check if the frame is for us. If not ignore the frame.
			if( ( ucRcvAddress == sMBslave.ucMBAddress ) )
			{
				sMBslave.state = COM_EXECUTE;
			}
			else
				sMBslave.state = COM_START;	
            
            break;

        case COM_EXECUTE:

			//check CRC
			crc = usMBCRC16( sMBslave.ucRTUBuf, sMBslave.usRcvBufferPos );

			//check crc for zero, if zero get actual length 
			if( crc == 0 )
				usLength = ( USHORT )( sMBslave.usRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC );
			else
				usLength = 0;
			
			//store the function in order to handle the data
            ucFunctionCode = sMBslave.ucRTUBuf[MB_PDU_FUNC_OFF];
            eException = MB_EX_ILLEGAL_FUNCTION;

			switch( ucFunctionCode )
			{
				case MB_FUNC_OTHER_REPORT_SLAVEID:
					eException = eMBFuncReportSlaveID( sMBslave.ucRTUBuf, &usLength );
				break;

				case MB_FUNC_READ_INPUT_REGISTER :
					eException = eMBFuncReadInputRegister( sMBslave.ucRTUBuf, &usLength  );
				break;

				case MB_FUNC_READ_HOLDING_REGISTER:
					 eException = eMBFuncReadHoldingRegister( sMBslave.ucRTUBuf, &usLength );
				break;

				case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
					eException = eMBFuncWriteMultipleHoldingRegister( sMBslave.ucRTUBuf, &usLength );
				break;

				case MB_FUNC_WRITE_REGISTER:
					eException = eMBFuncWriteHoldingRegister( sMBslave.ucRTUBuf, &usLength  );
				break;

				case MB_FUNC_READWRITE_MULTIPLE_REGISTERS:
					eException = eMBFuncReadWriteMultipleHoldingRegister( sMBslave.ucRTUBuf, &usLength );
				break;

				case MB_FUNC_READ_COILS:
					eException = eMBFuncReadCoils( sMBslave.ucRTUBuf, &usLength );
				break;

				case MB_FUNC_WRITE_SINGLE_COIL:
					eException = eMBFuncWriteCoil( sMBslave.ucRTUBuf, &usLength );
				break;

				case MB_FUNC_WRITE_MULTIPLE_COILS:
					eException = eMBFuncWriteMultipleCoils( sMBslave.ucRTUBuf, &usLength );
				break;

				case MB_FUNC_READ_DISCRETE_INPUTS:
					eException = eMBFuncReadDiscreteInputs( sMBslave.ucRTUBuf, &usLength );
				break;

				default:
				break;
			}
		
            //If the request was not sent to the broadcast address we
             // return a reply. 
            if( ucRcvAddress != MB_ADDRESS_BROADCAST )
            {
                if( eException != MB_EX_NONE )
                {
                    // An exception occured. Build an error frame.
                    usLength = 1;
                    sMBslave.ucRTUBuf[usLength++] = ( UCHAR )( ucFunctionCode | MB_FUNC_ERROR );
                    sMBslave.ucRTUBuf[usLength++] = eException;
                }
        
				//send response packet to the client
                eStatus = eMBRTUSend( sMBslave.ucMBAddress, sMBslave.ucRTUBuf, usLength );

            }

			sMBslave.state = COM_START;	
            break;


	}
	sMBslave.prevState = sMBslave.state;

}

/********************************************************************
* FUNCTIONS USED TO CRC CALCULATION
********************************************************************/

USHORT
usMBCRC16( UCHAR * pucFrame, USHORT usLen )
{
    UCHAR           ucCRCHi = 0xFF;
    UCHAR           ucCRCLo = 0xFF;
	USHORT          crc = 0;

    int             iIndex;

    while( usLen-- )
    {
        iIndex = ucCRCLo ^ *( pucFrame++ );
        ucCRCLo = ( UCHAR )( ucCRCHi ^ aucCRCHi[iIndex] );
        ucCRCHi = aucCRCLo[iIndex];
    }

	crc = ((USHORT) ucCRCHi << 8) ;
	crc |=  ucCRCLo;
	return crc;
}

/********************************************************************
* FUNCTIONS USED TO TRANSMIT THE RESPONSE PACKET
********************************************************************/
eMBErrorCode
eMBRTUSend( UCHAR ucSlaveAddress, const UCHAR * pucFrame, USHORT usLength )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          usCRC16;
	UCHAR			i;		
	UINT8			usSndBufferCount = 0;

    DISABLE_UART_RX_INTERRUPT(  );

	// First byte before the Modbus-PDU is the slave address.
	sMBslave.ucRTUBuf[usSndBufferCount] = ucSlaveAddress;
	usSndBufferCount = 1;

	usSndBufferCount += usLength - 1;
	
	//Calculate CRC16 checksum for Modbus-Serial-Line-PDU.
	usCRC16 = usMBCRC16( ( UCHAR * ) sMBslave.ucRTUBuf, usSndBufferCount );
	sMBslave.ucRTUBuf[usSndBufferCount++] = ( UCHAR )( usCRC16 & 0xFF );
	sMBslave.ucRTUBuf[usSndBufferCount++] = ( UCHAR )( usCRC16 >> 8 );


	//transmit the response packet
	for( i = 0; i < usSndBufferCount; i++ )
#ifdef __p18f4520
		UART_write( sMBslave.ucRTUBuf[i] );
#else
		UART1_write( sMBslave.ucRTUBuf[i] );
#endif

#ifdef __p18f4520
	UART_transmit(  );
#else
	UART1_transmit(  );
#endif

	//interframe delay
	Delay10us( sMBslave.usTimerT35_50us/10 );
		
    ENABLE_UART_RX_INTERRUPT(  );
    return eStatus;
}



/********************************************************************
* function used to get the multiplier to determine the data reception
********************************************************************/

UCHAR getOffset( void )
{
	UCHAR functionCode;
	UCHAR offset = 0;

   	functionCode = sMBslave.ucRTUBuf[MB_PDU_FUNC_OFF];

	if( ( functionCode > 6 ) && ( sMBslave.usRcvBufferPos > 6 ) )
	{
		//get the byte count 
		offset = sMBslave.ucRTUBuf[MB_SER_PDU_REG_COUNT_OFF + 1];

		//consider byte count as a single byte
		offset++ ;
	}	

	return offset;	
}