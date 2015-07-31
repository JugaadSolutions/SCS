/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (c) 2006 Christian Walter <wolti@sil.at>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: mbfunccoils.c,v 1.8 2007/02/18 23:47:16 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_FUNC_READ_ADDR_OFF           			( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_READ_COILCNT_OFF        			( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_SIZE               			( 4 )
#define MB_PDU_FUNC_READ_COILCNT_MAX        			( 0x07D0 )

#define MB_PDU_FUNC_WRITE_ADDR_OFF          			( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_WRITE_VALUE_OFF         			( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_SIZE              			( 4 )

#define MB_PDU_FUNC_WRITE_MUL_ADDR_OFF      			( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF   			( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF   			( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_FUNC_WRITE_MUL_VALUES_OFF    			( MB_PDU_DATA_OFF + 5 )
#define MB_PDU_FUNC_WRITE_MUL_SIZE_MIN      			( 5 )
#define MB_PDU_FUNC_WRITE_MUL_COILCNT_MAX   			( 0x07B0 )

#define MB_PDU_FUNC_READ_COILS_ADDR_OFF     			( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_READ_COILS_DISCCNT_OFF  			( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_COILS_SIZE    	    			( 4 )
#define MB_PDU_FUNC_READ_COILS_DISCCNT_MAX  			( 0x07D0 )


//utils
#define BITS_UCHAR      8U


//mbfuncinput
#define MB_PDU_FUNC_READ_INPUT_ADDR_OFF           		( MB_PDU_DATA_OFF  )
#define MB_PDU_FUNC_READ_INPUT_REGCNT_OFF         		(  MB_PDU_FUNC_READ_ADDR_OFF + 2 )
#define MB_PDU_FUNC_READ_INPUT_SIZE              		( 4 )
#define MB_PDU_FUNC_READ_INPUT_REGCNT_MAX  			    ( 0x007D )

#define MB_PDU_FUNC_READ_RSP_BYTECNT_OFF    			( MB_PDU_DATA_OFF )


//mbfuncholding
#define MB_PDU_FUNC_READ_HOLDING_ADDR_OFF               ( MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_READ_HOLDING_REGCNT_OFF             ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_HOLDING_SIZE                   ( 4 )
#define MB_PDU_FUNC_READ_HOLDING_REGCNT_MAX             ( 0x007D )

#define MB_PDU_FUNC_WRITE_HOLDING_ADDR_OFF              ( MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_WRITE_HOLDING_VALUE_OFF             ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_HOLDING_SIZE                  ( 4 )

#define MB_PDU_FUNC_WRITE_MUL_HOLDING_ADDR_OFF          ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_WRITE_MUL_HOLDING_REGCNT_OFF        ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_MUL_HOLDING_BYTECNT_OFF       ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_FUNC_WRITE_MUL_HOLDING_VALUES_OFF        ( MB_PDU_DATA_OFF + 5 )
#define MB_PDU_FUNC_WRITE_MUL_HOLDING_SIZE_MIN          ( 5 )
#define MB_PDU_FUNC_WRITE_MUL_HOLDING_REGCNT_MAX        ( 0x0078 )

#define MB_PDU_FUNC_READWRITE_READ_HOLDING_ADDR_OFF     ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_READWRITE_READ_HOLDING_REGCNT_OFF   ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READWRITE_WRITE_HOLDING_ADDR_OFF    ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_FUNC_READWRITE_WRITE_HOLDING_REGCNT_OFF  ( MB_PDU_DATA_OFF + 6 )
#define MB_PDU_FUNC_READWRITE_HOLDING_BYTECNT_OFF       ( MB_PDU_DATA_OFF + 8 )
#define MB_PDU_FUNC_READWRITE_WRITE_HOLDING_VALUES_OFF  ( MB_PDU_DATA_OFF + 9 )
#define MB_PDU_FUNC_READWRITE_HOLDING_SIZE_MIN          ( 9 )


//mbfuncdisc
#define MB_PDU_FUNC_READ_COILS_ADDR_OFF           ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_READ_COILS_DISCCNT_OFF        ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_COILS_SIZE               ( 4 )
#define MB_PDU_FUNC_READ_COILS_DISCCNT_MAX        ( 0x07D0 )


/* ----------------------- Static functions ---------------------------------*/
eMBException    prveMBError2Exception( eMBErrorCode eErrorCode );

/* ----------------------- Start implementation -----------------------------*/

#if MB_FUNC_READ_COILS_ENABLED > 0

eMBException
eMBFuncReadCoils( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    USHORT          usCoilCount;
    UCHAR           ucNBytes;
    UCHAR          *pucFrameCur;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1] );
        usRegAddress++;

        usCoilCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_COILCNT_OFF] << 8 );
        usCoilCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_COILCNT_OFF + 1] );

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception. 
         */
        if( ( usCoilCount >= 1 ) &&
            ( usCoilCount < MB_PDU_FUNC_READ_COILCNT_MAX ) )
        {
            /* Set the current PDU data pointer to the beginning. */
            pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
            *usLen = MB_PDU_FUNC_OFF;

            /* First byte contains the function code. */
            *pucFrameCur++ = MB_FUNC_READ_COILS;
            *usLen += 1;

            /* Test if the quantity of coils is a multiple of 8. If not last
             * byte is only partially field with unused coils set to zero. */
            if( ( usCoilCount & 0x0007 ) != 0 )
            {
                ucNBytes = ( UCHAR )( usCoilCount / 8 + 1 );
            }
            else
            {
                ucNBytes = ( UCHAR )( usCoilCount / 8 );
            }
            *pucFrameCur++ = ucNBytes;
            *usLen += 1;

            eRegStatus =
                eMBRegCoilsCB( pucFrameCur, usRegAddress, usCoilCount,
                               MB_REG_READ );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
            else
            {
                /* The response contains the function code, the starting address
                 * and the quantity of registers. We reuse the old values in the 
                 * buffer because they are still valid. */
                *usLen += ucNBytes;;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid read coil register request because the length
         * is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#if MB_FUNC_WRITE_COIL_ENABLED > 0
eMBException
eMBFuncWriteCoil( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    UCHAR           ucBuf[2];

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1] );
        usRegAddress++;

        if( ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF + 1] == 0x00 ) &&
            ( ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF ) ||
              ( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0x00 ) ) )
        {
            ucBuf[1] = 0;
            if( pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF )
            {
                ucBuf[0] = 1;
            }
            else
            {
                ucBuf[0] = 0;
            }
            eRegStatus =
                eMBRegCoilsCB( &ucBuf[0], usRegAddress, 1, MB_REG_WRITE );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid write coil register request because the length
         * is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif

#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
eMBException
eMBFuncWriteMultipleCoils( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    USHORT          usCoilCnt;
    UCHAR           ucByteCount;
    UCHAR           ucByteCountVerify;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen > ( MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF + 1] );
        usRegAddress++;

        usCoilCnt = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF] << 8 );
        usCoilCnt |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF + 1] );

        ucByteCount = pucFrame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];

        /* Compute the number of expected bytes in the request. */
        if( ( usCoilCnt & 0x0007 ) != 0 )
        {
            ucByteCountVerify = ( UCHAR )( usCoilCnt / 8 + 1 );
        }
        else
        {
            ucByteCountVerify = ( UCHAR )( usCoilCnt / 8 );
        }

        if( ( usCoilCnt >= 1 ) &&
            ( usCoilCnt <= MB_PDU_FUNC_WRITE_MUL_COILCNT_MAX ) &&
            ( ucByteCountVerify == ucByteCount ) )
        {
            eRegStatus =
                eMBRegCoilsCB( &pucFrame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF],
                               usRegAddress, usCoilCnt, MB_REG_WRITE );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
            else
            {
                /* The response contains the function code, the starting address
                 * and the quantity of registers. We reuse the old values in the 
                 * buffer because they are still valid. */
                *usLen = MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid write coil register request because the length
         * is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif


//utils
void
xMBUtilSetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits,
                UCHAR ucValue )
{
    USHORT          usWordBuf;
    USHORT          usMask;
    USHORT          usByteOffset;
    USHORT          usNPreBits;
    USHORT          usValue = ucValue;

    //assert( ucNBits <= 8 );
	if( ucNBits > 8 )
	return ;

    //assert( ( size_t )BITS_UCHAR == sizeof( UCHAR ) * 8 );

    /* Calculate byte offset for first byte containing the bit values starting
     * at usBitOffset. */
    usByteOffset = ( USHORT )( ( usBitOffset ) / BITS_UCHAR );

    /* How many bits precede our bits to set. */
    usNPreBits = ( USHORT )( usBitOffset - usByteOffset * BITS_UCHAR );

    /* Move bit field into position over bits to set */
    usValue <<= usNPreBits;

    /* Prepare a mask for setting the new bits. */
    usMask = ( USHORT )( ( 1 << ( USHORT ) ucNBits ) - 1 );
    usMask <<= usBitOffset - usByteOffset * BITS_UCHAR;

    /* copy bits into temporary storage. */
    usWordBuf = ucByteBuf[usByteOffset];
    usWordBuf |= ucByteBuf[usByteOffset + 1] << BITS_UCHAR;

    /* Zero out bit field bits and then or value bits into them. */
    usWordBuf = ( USHORT )( ( usWordBuf & ( ~usMask ) ) | usValue );

    /* move bits back into storage */
    ucByteBuf[usByteOffset] = ( UCHAR )( usWordBuf & 0xFF );
    ucByteBuf[usByteOffset + 1] = ( UCHAR )( usWordBuf >> BITS_UCHAR );
}

UCHAR
xMBUtilGetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits )
{
    USHORT          usWordBuf;
    USHORT          usMask;
    USHORT          usByteOffset;
    USHORT          usNPreBits;

    /* Calculate byte offset for first byte containing the bit values starting
     * at usBitOffset. */
    usByteOffset = ( USHORT )( ( usBitOffset ) / BITS_UCHAR );

    /* How many bits precede our bits to set. */
    usNPreBits = ( USHORT )( usBitOffset - usByteOffset * BITS_UCHAR );

    /* Prepare a mask for setting the new bits. */
    usMask = ( USHORT )( ( 1 << ( USHORT ) ucNBits ) - 1 );

    /* copy bits into temporary storage. */
    usWordBuf = ucByteBuf[usByteOffset];
    usWordBuf |= ucByteBuf[usByteOffset + 1] << BITS_UCHAR;

    /* throw away unneeded bits. */
    usWordBuf >>= usNPreBits;

    /* mask away bits above the requested bitfield. */
    usWordBuf &= usMask;

    return ( UCHAR ) usWordBuf;
}

eMBException
prveMBError2Exception( eMBErrorCode eErrorCode )
{
    eMBException    eStatus;

    switch ( eErrorCode )
    {
        case MB_ENOERR:
            eStatus = MB_EX_NONE;
            break;

        case MB_ENOREG:
            eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
            break;

        case MB_ETIMEDOUT:
            eStatus = MB_EX_SLAVE_BUSY;
            break;

        default:
            eStatus = MB_EX_SLAVE_DEVICE_FAILURE;
            break;
    }

    return eStatus;
}

//mbfuncother
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0

/* ----------------------- Static variables ---------------------------------*/
static UCHAR    ucMBSlaveID[MB_FUNC_OTHER_REP_SLAVEID_BUF];
static USHORT   usMBSlaveIDLen;

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBSetSlaveID( UCHAR ucSlaveID, BOOL xIsRunning,
               UCHAR const *pucAdditional, USHORT usAdditionalLen )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    /* the first byte and second byte in the buffer is reserved for
     * the parameter ucSlaveID and the running flag. The rest of
     * the buffer is available for additional data. */
    if( usAdditionalLen + 2 < MB_FUNC_OTHER_REP_SLAVEID_BUF )
    {
        usMBSlaveIDLen = 0;
        ucMBSlaveID[usMBSlaveIDLen++] = ucSlaveID;
        ucMBSlaveID[usMBSlaveIDLen++] = ( UCHAR )( xIsRunning ? 0xFF : 0x00 );
        if( usAdditionalLen > 0 )
        {
            memcpy( &ucMBSlaveID[usMBSlaveIDLen], pucAdditional,
                    ( size_t )usAdditionalLen );
            usMBSlaveIDLen += usAdditionalLen;
        }
    }
    else
    {
        eStatus = MB_ENORES;
    }
    return eStatus;
}

eMBException
eMBFuncReportSlaveID( UCHAR * pucFrame, USHORT * usLen )
{
    memcpy( &pucFrame[MB_PDU_DATA_OFF], &ucMBSlaveID[0], ( size_t )usMBSlaveIDLen );
    *usLen = ( USHORT )( MB_PDU_DATA_OFF + usMBSlaveIDLen );
    return MB_EX_NONE;
}

#endif

//mbfuncinput
#if MB_FUNC_READ_INPUT_ENABLED > 0

eMBException
eMBFuncReadInputRegister( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    USHORT          usRegCount;
    UCHAR          *pucFrameCur;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;
	USHORT temp;
    if( *usLen == ( MB_PDU_FUNC_READ_INPUT_SIZE + MB_PDU_SIZE_MIN ) )
    {
	    usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_INPUT_ADDR_OFF]);
		usRegAddress <<= 8;
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_INPUT_ADDR_OFF + 1] );
        usRegAddress++;

		usRegCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_INPUT_REGCNT_OFF]);
        usRegCount  <<= 8 ;
        usRegCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_INPUT_REGCNT_OFF + 1] );

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception. 
         */
        if( ( usRegCount >= 1 )
            && ( usRegCount < MB_PDU_FUNC_READ_INPUT_REGCNT_MAX ) )
        {
            /* Set the current PDU data pointer to the beginning. */
            pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
            *usLen = MB_PDU_FUNC_OFF;

            /* First byte contains the function code. */
            *pucFrameCur++ = MB_FUNC_READ_INPUT_REGISTER;
            *usLen += 1;

            /* Second byte in the response contain the number of bytes. */
            *pucFrameCur++ = ( UCHAR )( usRegCount * 2 );
            *usLen += 1;

            eRegStatus =
                eMBRegInputCB( pucFrameCur, usRegAddress, usRegCount );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
            else
            {
                *usLen += usRegCount * 2;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid read input register request because the length
         * is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif



//mbfuncholding
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0

eMBException
eMBFuncWriteHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_FUNC_WRITE_HOLDING_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_HOLDING_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_HOLDING_ADDR_OFF + 1] );
        usRegAddress++;

        /* Make callback to update the value. */
        eRegStatus = eMBRegHoldingCB( &pucFrame[MB_PDU_FUNC_WRITE_HOLDING_VALUE_OFF],
                                      usRegAddress, 1, MB_REG_WRITE );

        /* If an error occured convert it into a Modbus exception. */
        if( eRegStatus != MB_ENOERR )
        {
            eStatus = prveMBError2Exception( eRegStatus );
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
#endif

#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
eMBException
eMBFuncWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    USHORT          usRegCount;
    UCHAR           ucRegByteCount;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen >= ( MB_PDU_FUNC_WRITE_MUL_HOLDING_SIZE_MIN + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_HOLDING_ADDR_OFF] );
		usRegAddress <<= 8;
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_HOLDING_ADDR_OFF + 1] );
        usRegAddress++;

        usRegCount = ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_HOLDING_REGCNT_OFF] );
		usRegCount <<= 8;
        usRegCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_WRITE_MUL_HOLDING_REGCNT_OFF + 1] );

        ucRegByteCount = pucFrame[MB_PDU_FUNC_WRITE_MUL_HOLDING_BYTECNT_OFF];

        if( ( usRegCount >= 1 ) &&
            ( usRegCount <= MB_PDU_FUNC_WRITE_MUL_HOLDING_REGCNT_MAX ) &&
            ( ucRegByteCount == ( UCHAR ) ( 2 * usRegCount ) ) )
        {
            /* Make callback to update the register values. */
            eRegStatus =
                eMBRegHoldingCB( &pucFrame[MB_PDU_FUNC_WRITE_MUL_HOLDING_VALUES_OFF],
                                 usRegAddress, usRegCount, MB_REG_WRITE );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
            else
            {
                /* The response contains the function code, the starting
                 * address and the quantity of registers. We reuse the
                 * old values in the buffer because they are still valid.
                 */
                *usLen = MB_PDU_FUNC_WRITE_MUL_HOLDING_BYTECNT_OFF;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
#endif

#if MB_FUNC_READ_HOLDING_ENABLED > 0

eMBException
eMBFuncReadHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    USHORT          usRegCount;
    UCHAR          *pucFrameCur;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_FUNC_READ_HOLDING_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_HOLDING_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_HOLDING_ADDR_OFF + 1] );
        usRegAddress++;

        usRegCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_HOLDING_REGCNT_OFF] << 8 );
        usRegCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_HOLDING_REGCNT_OFF + 1] );

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception. 
         */
        if( ( usRegCount >= 1 ) && ( usRegCount <= MB_PDU_FUNC_READ_HOLDING_REGCNT_MAX ) )
        {
            /* Set the current PDU data pointer to the beginning. */
            pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
            *usLen = MB_PDU_FUNC_OFF;

            /* First byte contains the function code. */
            *pucFrameCur++ = MB_FUNC_READ_HOLDING_REGISTER;
            *usLen += 1;

            /* Second byte in the response contain the number of bytes. */
            *pucFrameCur++ = ( UCHAR ) ( usRegCount * 2 );
            *usLen += 1;

            /* Make callback to fill the buffer. */
            eRegStatus = eMBRegHoldingCB( pucFrameCur, usRegAddress, usRegCount, MB_REG_READ );
            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
            else
            {
                *usLen += usRegCount * 2;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif

#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0

eMBException
eMBFuncReadWriteMultipleHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegReadAddress;
    USHORT          usRegReadCount;
    USHORT          usRegWriteAddress;
    USHORT          usRegWriteCount;
    UCHAR           ucRegWriteByteCount;
    UCHAR          *pucFrameCur;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen >= ( MB_PDU_FUNC_READWRITE_HOLDING_SIZE_MIN + MB_PDU_SIZE_MIN ) )
    {
        usRegReadAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_READ_HOLDING_ADDR_OFF] << 8U );
        usRegReadAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_READ_HOLDING_ADDR_OFF + 1] );
        usRegReadAddress++;

        usRegReadCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_READ_HOLDING_REGCNT_OFF] << 8U );
        usRegReadCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_READ_HOLDING_REGCNT_OFF + 1] );

        usRegWriteAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_WRITE_HOLDING_ADDR_OFF] << 8U );
        usRegWriteAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_WRITE_HOLDING_ADDR_OFF + 1] );
        usRegWriteAddress++;

        usRegWriteCount = ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_WRITE_HOLDING_REGCNT_OFF] << 8U );
        usRegWriteCount |= ( USHORT )( pucFrame[MB_PDU_FUNC_READWRITE_WRITE_HOLDING_REGCNT_OFF + 1] );

        ucRegWriteByteCount = pucFrame[MB_PDU_FUNC_READWRITE_HOLDING_BYTECNT_OFF];

        if( ( usRegReadCount >= 1 ) && ( usRegReadCount <= 0x7D ) &&
            ( usRegWriteCount >= 1 ) && ( usRegWriteCount <= 0x79 ) &&
            ( ( 2 * usRegWriteCount ) == ucRegWriteByteCount ) )
        {
            /* Make callback to update the register values. */
            eRegStatus = eMBRegHoldingCB( &pucFrame[MB_PDU_FUNC_READWRITE_WRITE_HOLDING_VALUES_OFF],
                                          usRegWriteAddress, usRegWriteCount, MB_REG_WRITE );

            if( eRegStatus == MB_ENOERR )
            {
                /* Set the current PDU data pointer to the beginning. */
                pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
                *usLen = MB_PDU_FUNC_OFF;

                /* First byte contains the function code. */
                *pucFrameCur++ = MB_FUNC_READWRITE_MULTIPLE_REGISTERS;
                *usLen += 1;

                /* Second byte in the response contain the number of bytes. */
                *pucFrameCur++ = ( UCHAR ) ( usRegReadCount * 2 );
                *usLen += 1;

                /* Make the read callback. */
                eRegStatus =
                    eMBRegHoldingCB( pucFrameCur, usRegReadAddress, usRegReadCount, MB_REG_READ );
                if( eRegStatus == MB_ENOERR )
                {
                    *usLen += 2 * usRegReadCount;
                }
            }
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    return eStatus;
}

#endif

//mbfundisc
#if MB_FUNC_READ_COILS_ENABLED > 0

eMBException
eMBFuncReadDiscreteInputs( UCHAR * pucFrame, USHORT * usLen )
{
    USHORT          usRegAddress;
    USHORT          usDiscreteCnt;
    UCHAR           ucNBytes;
    UCHAR          *pucFrameCur;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_FUNC_READ_COILS_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_COILS_ADDR_OFF] << 8 );
        usRegAddress |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_COILS_ADDR_OFF + 1] );
        usRegAddress++;

        usDiscreteCnt = ( USHORT )( pucFrame[MB_PDU_FUNC_READ_COILS_DISCCNT_OFF] << 8 );
        usDiscreteCnt |= ( USHORT )( pucFrame[MB_PDU_FUNC_READ_COILS_DISCCNT_OFF + 1] );

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception. 
         */
        if( ( usDiscreteCnt >= 1 ) &&
            ( usDiscreteCnt < MB_PDU_FUNC_READ_COILS_DISCCNT_MAX ) )
        {
            /* Set the current PDU data pointer to the beginning. */
            pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
            *usLen = MB_PDU_FUNC_OFF;

            /* First byte contains the function code. */
            *pucFrameCur++ = MB_FUNC_READ_DISCRETE_INPUTS;
            *usLen += 1;

            /* Test if the quantity of coils is a multiple of 8. If not last
             * byte is only partially field with unused coils set to zero. */
            if( ( usDiscreteCnt & 0x0007 ) != 0 )
            {
                ucNBytes = ( UCHAR ) ( usDiscreteCnt / 8 + 1 );
            }
            else
            {
                ucNBytes = ( UCHAR ) ( usDiscreteCnt / 8 );
            }
            *pucFrameCur++ = ucNBytes;
            *usLen += 1;

            eRegStatus =
                eMBRegDiscreteCB( pucFrameCur, usRegAddress, usDiscreteCnt );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
            else
            {
                /* The response contains the function code, the starting address
                 * and the quantity of registers. We reuse the old values in the 
                 * buffer because they are still valid. */
                *usLen += ucNBytes;;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid read coil register request because the length
         * is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif


#endif
