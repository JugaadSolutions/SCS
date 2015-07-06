#include "utilities.h"



/*
*------------------------------------------------------------------------------
* UINT8 ConvertBCD2HEX(UINT16 bcd)
*
* Summary	: Convert BCD 1 byte to HEX 1 byte
*
* Input		: UINT16 bcd - bcd value
*
* Output	: UINT8 - converted HEX value
*
*------------------------------------------------------------------------------
*/
UINT8 ConvertBCD2HEX(UINT16 bcd)
{
	UINT8 temp;
	temp=((bcd>>8)*100)|((bcd>>4)*10)|(bcd&0x0f);
	return temp;
}


/*
*------------------------------------------------------------------------------
* UINT8 HEX2BCD(UINT8 hexIn)
*
* Summary	: Convert 1 b yte HEX to 1 byte BCD
*
* Input		: UINT8 bcd - bcd value
*
* Output	: UINT8 - converted HEX value
*
*------------------------------------------------------------------------------
*/
UINT8 HEX2BCD(UINT8 hexIn)
{
	UINT8 temp1,temp2;
	temp1 = (hexIn / 10);
	temp2 = (hexIn % 10);
	return (((temp1 & 0x0F) << 4 ) | (temp2 & 0x0F));
}


/*
*------------------------------------------------------------------------------
* UINT8 BCDtoBin(UINT8 val)

* Summary	:
*
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
UINT8 BCDtoBin(UINT8 val)		 // To read from RTC.
{
	UINT8 bin = 0;
	bin = (((val & 0xF0)>> 4) * 10 ) + (val &0x0F);

   return bin;
}


/*
*------------------------------------------------------------------------------
* UINT16 BinToBCD(UINT8 temp)

* Summary	:
*
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
UINT16 BinToBCD(UINT8 temp) 	     // To write to RTC
{
   temp += (temp/0x0A)*0x06;
   return temp;
}

void UTL_binaryToBCDASCII(UINT8 data , UINT8* asciiStr )
{
	asciiStr[1] = data%10 + '0';
	asciiStr[0] = data/10 + '0';
}
	