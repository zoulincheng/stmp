#include "basictype.h"
#include "radio_config.h"
#include "si446x.h"
#include "boardiodef.h"
#include <string.h>
#include "sysprintf.h"
#include "spi.h"


#define SI446X_WAIT_CTS_CNT		32

static void SI446X_RX_FIFO_RESET( void );
static void SI446X_TX_FIFO_RESET( void );
static void SI446X_TXRX_FIFO_RESET( void );
static void SI446X_GLOGAL_CFG_CMMBINED_FIFO(void);
static void SI446X_GPIO_CONFIG( INT8U G0, INT8U G1, INT8U G2, INT8U G3,INT8U IRQ, INT8U SDO, INT8U GEN_CONFIG );
static const INT8U  si446x_cfg[] = RADIO_CONFIGURATION_DATA_ARRAY;


static volatile u_char rfchnum = 0;

extern INT8U SPI_SendByteData(INT8U byte);

//redefine this two variable for deferent cpu and plantform
union si446x_cmd_reply_union Si446xCmd;


/*
=================================================================================
SI446X_WAIT_CTS( );
Function : wait the device ready to response a command
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_WAIT_CTS( void )
{
    INT8U cts;
    do
    {
        RF_SPINSEL(0);
        SPI_SendByteData( READ_CMD_BUFF );
        cts = SPI_SendByteData(0xFF);
        RF_SPINSEL(1);
    }while((cts != 0xFF));
}


/*
=================================================================================
SI446X_CMD( );
Function : Send a command to the device
INTPUT   : cmd, the buffer stores the command array
           cmdsize, the size of the command array
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_CMD( INT8U *cmd, INT8U cmdsize )
{
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);
    while( cmdsize -- )
    {
        SPI_SendByteData( *cmd++ );
    }
	RF_SPINSEL(1);
}





/*
=================================================================================
SI446X_POWER_UP( );
Function : Power up the device
INTPUT   : f_xtal, the frequency of the external high-speed crystal
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_POWER_UP( INT32U f_xtal )
{
    INT8U cmd[7];
    cmd[0] = POWER_UP;
    cmd[1] = 0x01;
    cmd[2] = 0x00;
    cmd[3] = f_xtal>>24;
    cmd[4] = f_xtal>>16;
    cmd[5] = f_xtal>>8;
    cmd[6] = f_xtal;
    SI446X_CMD( cmd, 7 );
}


/*
=================================================================================
SI446X_READ_RESPONSE( );
Function : read a array of command response
INTPUT   : buffer,  a buffer, stores the data responsed
           size,    How many bytes should be read
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_READ_RESPONSE( INT8U *buffer, INT8U size )
{
	//u_char ubCTS = 0x00;
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);
	SPI_SendByteData( READ_CMD_BUFF );
	//ubCTS = SPI_SendByteData( 0xFF );
	//if (ubCTS == 0xff)
	{
		while( size -- )
	    {
	        *buffer++ = SPI_SendByteData( 0xFF );
	    }
    }
    RF_SPINSEL(1);
}



/*
=================================================================================
SI446X_NOP( );
Function : NO Operation command
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
static INT8U SI446X_NOP( void )
{
    INT8U cts;
    RF_SPINSEL(0);
    cts = SPI_SendByteData( NOP );
    RF_SPINSEL(1);
	return cts;
}




/*
=================================================================================
SI446X_PART_INFO( );
Function : Read the PART_INFO of the device, 8 bytes needed
INTPUT   : buffer, the buffer stores the part information
OUTPUT   : NONE
=================================================================================
*/
void SI446X_PART_INFO( INT8U *buffer )
{
    INT8U cmd = PART_INFO;
    u_char ubData[16] = {0x00};
    SI446X_CMD( &cmd, 1 );
    //SI446X_READ_RESPONSE(buffer, 9);
    SI446X_READ_RESPONSE(ubData, 8);
    MEM_DUMP(0,"part", ubData, 8);
}



/*
=================================================================================
SI446X_FUNC_INFO( );
Function : Read the FUNC_INFO of the device, 7 bytes needed
INTPUT   : buffer, the buffer stores the FUNC information
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_FUNC_INFO( INT8U *buffer )
{
    INT8U cmd = FUNC_INFO;
    SI446X_CMD( &cmd, 1 );
    SI446X_READ_RESPONSE( buffer, 7 );
}



/*
=================================================================================
SI446X_INT_STATUS( );
Function : Read the INT status of the device, 9 bytes needed
INTPUT   : buffer, the buffer stores the int status
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_INT_STATUS( INT8U *buffer )
{
    INT8U cmd[4];
    cmd[0] = GET_INT_STATUS;
    cmd[1] = 0;
    cmd[2] = 0;
    cmd[3] = 0;

    SI446X_CMD( cmd, 4 );
    SI446X_READ_RESPONSE( buffer, 9 );
}







/*
=================================================================================
SI446X_GET_PROPERTY( );
Function : Read the PROPERTY of the device
INTPUT   : buffer, the buffer stores the PROPERTY value
           GROUP_NUM, the group and number of the parameter
           NUM_GROUP, number of the group
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_GET_PROPERTY_X( SI446X_PROPERTY GROUP_NUM, INT8U NUM_PROPS, INT8U *buffer  )
{
    INT8U cmd[4];

    cmd[0] = GET_PROPERTY;
    cmd[1] = GROUP_NUM>>8;
    cmd[2] = NUM_PROPS;
    cmd[3] = GROUP_NUM;

    SI446X_CMD( cmd, 4 );
    SI446X_READ_RESPONSE( buffer, NUM_PROPS + 1 );
}




/*
=================================================================================
SI446X_SET_PROPERTY_X( );
Function : Set the PROPERTY of the device
INTPUT   : GROUP_NUM, the group and the number of the parameter
           NUM_GROUP, number of the group
           PAR_BUFF, buffer stores parameters
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_SET_PROPERTY_X( SI446X_PROPERTY GROUP_NUM, INT8U NUM_PROPS, INT8U *PAR_BUFF )
{
    INT8U  cmd[20], i = 0;
    if( NUM_PROPS >= 16 )   { return; }
    cmd[i++] = SET_PROPERTY;
    cmd[i++] = GROUP_NUM>>8;
    cmd[i++] = NUM_PROPS;
    cmd[i++] = GROUP_NUM;
    while( NUM_PROPS-- )
    {
        cmd[i++] = *PAR_BUFF++;
    }
    SI446X_CMD( cmd, i );
}



/*
=================================================================================
SI446X_SET_PROPERTY_1( );
Function : Set the PROPERTY of the device, only 1 byte
INTPUT   : GROUP_NUM, the group and number index
           proerty,  the value to be set
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_SET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM, INT8U proerty )
{
    INT8U  cmd[5];
    
	//SI446X_WAIT_CTS( );
	
    cmd[0] = SET_PROPERTY;
    cmd[1] = GROUP_NUM>>8;
    cmd[2] = 1;
    cmd[3] = GROUP_NUM;
    cmd[4] = proerty;
    SI446X_CMD( cmd, 5 );
}




/*
=================================================================================
SI446X_GET_PROPERTY_1( );
Function : Get the PROPERTY of the device, only 1 byte
INTPUT   : GROUP_NUM, the group and number index
OUTPUT   : the PROPERTY value read from device
=================================================================================
*/
static INT8U SI446X_GET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM )
{
    INT8U  cmd[4];

    cmd[0] = GET_PROPERTY;
    cmd[1] = GROUP_NUM>>8;
    cmd[2] = 1;
    cmd[3] = GROUP_NUM;
    SI446X_CMD( cmd, 4 );
    SI446X_READ_RESPONSE( cmd, 2 );
    return cmd[1];
}
/*
=================================================================================
SI446X_RESET( );
Function : reset the SI446x device
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_RESET( void )
{
    RF_SDN(1);
    //clock_wait(1);
    /*delay code*/
    
    RF_SDN(0);
    /*delay code*/
    //clock_wait(50);
    
    RF_SPINSEL(1);
}


/*!
 * This function is used to load all properties and commands with a list of NULL terminated commands.
 * Before this function @si446x_reset should be called.
 */
static INT8U SI446X_CFG_PARAM_INIT(const INT8U* pSetPropCmd)
{
	INT8U col;
	INT8U numOfBytes;
	INT8U ubacmd[16];

	/* While cycle as far as the pointer points to a command */
	while (*pSetPropCmd != 0x00)
	{
		/* Commands structure in the array:
		* --------------------------------
		* LEN | <LEN length of data>
		*/

		numOfBytes = *pSetPropCmd++;

		if (numOfBytes > 16u)
		{
			/* Number of command bytes exceeds maximal allowable length */
			return 1;
		}

		for (col = 0u; col < numOfBytes; col++)
		{
			ubacmd[col] = *pSetPropCmd;
			pSetPropCmd++;
		}
		SI446X_CMD(ubacmd, numOfBytes);
	}

	return 0;
}



/*
=================================================================================
SI446X_CONFIG_INIT( );
Function : configuration the device
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_CONFIG_INIT( void )
{
	#if 0
    INT8U i;
    INT16U j = 0;
	u_char *pcfg = (u_char *)si446x_cfg;
    while( ( i = pcfg[j] ) != 0 )
    {
        j += 1;
        SI446X_CMD( pcfg + j, i );
        j += i;
    }
	#else
	SI446X_CFG_PARAM_INIT(si446x_cfg);
	#endif
    
#if PACKET_LENGTH > 0           //fixed packet length
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, PACKET_LENGTH );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_CRC_CONFIG, 0xA2 );
    SI446X_SET_PROPERTY_1( PKT_CRC_CONFIG, 0x05 );
#else                           //variable packet length
	#if 1
    SI446X_SET_PROPERTY_1( PKT_CONFIG1, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_CRC_CONFIG, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_LEN_FIELD_SOURCE, 0x01 );
    SI446X_SET_PROPERTY_1( PKT_LEN, 0x2A );
    SI446X_SET_PROPERTY_1( PKT_LEN_ADJUST, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_12_8, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, 0x01 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_CONFIG, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_CRC_CONFIG, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_12_8, 0x00 );
    //SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_7_0, 0x10 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_7_0, 0x80 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_CONFIG, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_CRC_CONFIG, 0x00 );
    #else
    //SI446X_SET_PROPERTY_1( PKT_CONFIG1, 0x00 ); //crc not ivert,lisbyte first,msbit first
    //SI446X_SET_PROPERTY_1( PKT_CRC_CONFIG, 0x04 );//crc seed 0, CRC_16_IBM
    SI446X_SET_PROPERTY_1( PKT_LEN_FIELD_SOURCE, 0x01 );//field is packet length
    SI446X_SET_PROPERTY_1( PKT_LEN, 0x2A );//msbyte first,1byte len,len save fifo,field 2 is var field
    SI446X_SET_PROPERTY_1( PKT_LEN_ADJUST, (INT8U)-3);//0xfd 1 byte packet field, 2 bytes crc
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_12_8, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, 0x01 );
    //SI446X_SET_PROPERTY_1( PKT_FIELD_1_CONFIG, 0x00 );
    //SI446X_SET_PROPERTY_1( PKT_FIELD_1_CRC_CONFIG, 0x80|0x02);//start crc from this field, and enable
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_12_8, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_7_0, 0x10);//max byte 129
    //SI446X_SET_PROPERTY_1( PKT_FIELD_2_CONFIG, 0x00 );
    //SI446X_SET_PROPERTY_1( PKT_FIELD_2_CRC_CONFIG, 0x20|0x08|0x02);//send crc, check crc, crc enable
    #endif
#endif //PACKET_LENGTH

    //重要： 4463的GDO2，GDO3控制射频开关，  0X20 ,0X21 
    //发射时必须： GDO2=1，GDO3=0
    //接收时必须： GDO2=0，GDO3=1
    SI446X_GPIO_CONFIG( 0, 0, 0x20, 0x21, 0, 0, 0 );//重要

}


/*
=================================================================================
SI446X_RADIO_CONFIG_INIT( );
Function : configuration the device
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_RADIO_CONFIG_INIT( void )
{
	SI446X_CFG_PARAM_INIT(si446x_cfg);
}

/*
=================================================================================
SI446X_RADIO_PKT_COMMON_CFG( );
Function : configuration pkt tx and rx common
INTPUT   : NONE
OUTPUT   : NONE
0x120d-0x1220
=================================================================================
*/
static void SI446X_RADIO_PKT_COMMON_CFG( void )
{
	#if 0
    SI446X_SET_PROPERTY_1(PKT_CRC_CONFIG, 0x05);//used all 0 for crc seed,CCIT-16: X16+X12+X5+1.
    SI446X_SET_PROPERTY_1(PKT_CONFIG1,0x80);//filed split,packet handler enable, manch 01, crc no invert, crc low byte first, MSB first
    #else
    SI446X_SET_PROPERTY_1(PKT_CRC_CONFIG, 0x00);//used all 0 for crc seed,CCIT-16: X16+X12+X5+1.
    SI446X_SET_PROPERTY_1(PKT_CONFIG1,0x80);//filed split,packet handler enable, manch 01, crc no invert, crc low byte first, MSB first
    #endif

	//open rx ,tx ,crc, syde 
	SI446X_SET_PROPERTY_1(INT_CTL_ENABLE, 0x07);//open PH, MODEM
	SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PSENT_EN|SI446X_INT_CTL_PH_PRX_EN);
	SI446X_SET_PROPERTY_1(INT_CTL_MODEM_ENABLE, SI446X_INT_CTL_MODEM_SYDE_EN);
	SI446X_SET_PROPERTY_1(INT_CTL_CHIP_ENABLE, SI446X_INT_CTL_CHIP_CMERR_EN);
}

/*
=================================================================================
SI446X_RADIO_TX_FIELD_CFG( );
Function : configuration tx field data
INTPUT   : NONE
OUTPUT   : NONE
0x120d-0x1220
=================================================================================
*/
static void SI446X_RADIO_TX_FIELD_CFG( void )
{
	SI446X_SET_PROPERTY_1(PKT_FIELD_1_LENGTH_7_0, 0x01);  //1 byte for packet length
	SI446X_SET_PROPERTY_1(PKT_FIELD_1_LENGTH_12_8, 0x00);
	SI446X_SET_PROPERTY_1(PKT_FIELD_1_CONFIG, 0x00); //
	//SI446X_SET_PROPERTY_1(PKT_FIELD_1_CRC_CONFIG, 0x80);//start crc, disable crc
	SI446X_SET_PROPERTY_1(PKT_FIELD_1_CRC_CONFIG, 0x00);//start crc, disable crc


	SI446X_SET_PROPERTY_1(PKT_FIELD_2_LENGTH_7_0, 0x80);//max length 128
	SI446X_SET_PROPERTY_1(PKT_FIELD_2_LENGTH_12_8, 0x00);
	SI446X_SET_PROPERTY_1(PKT_FIELD_2_CONFIG, 0x00);
	//SI446X_SET_PROPERTY_1(PKT_FIELD_2_CRC_CONFIG, 0x22);//send crc, enable crc
	SI446X_SET_PROPERTY_1(PKT_FIELD_2_CRC_CONFIG, 0x00);//disable send crc, disable crc

}


/*
=================================================================================
SI446X_RADIO_RX_FIELD_CFG( );
Function : configuration rx field data
INTPUT   : NONE
OUTPUT   : NONE
0x1221-0x1234
=================================================================================
*/
static void SI446X_RADIO_RX_FIELD_CFG( void )
{
	SI446X_SET_PROPERTY_1(PKT_LEN, 0x2a);//rx msb first, 1 byte length,length put in fifo, field 2 is variable length field.
	SI446X_SET_PROPERTY_1(PKT_LEN_FIELD_SOURCE, 0x01);//field 1 is packet length
	SI446X_SET_PROPERTY_1(PKT_LEN_ADJUST, (INT8U)(0x00));//paket format is l byte length, packet data, 0 byte crc

	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_1_LENGTH_7_0, 0x01);//1byte packet length
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_1_LENGTH_12_8, 0x00);//
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_1_CONFIG, 0x00);
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_1_CRC_CONFIG, 0x00);//start crc, enable crc

	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_2_LENGTH_12_8, 0x00);
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_2_LENGTH_7_0, 0x80);//128
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_2_CONFIG, 0x00);
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_2_CRC_CONFIG, 0x00);//check crc, enable crc
}



/*!
 * Issue a change state command to the radio.
 *
 * @param NEXT_STATE1 Next state.
 */
static void SI446X_CHANGE_STATE(INT8U NEXT_STATE1)
{
	INT8U ubacmd[2] = {0x00};
    ubacmd[0] = SI446X_CMD_ID_CHANGE_STATE;
    ubacmd[1] = NEXT_STATE1;

    SI446X_CMD(ubacmd, 2);
}

/*
=================================================================================
SI446X_W_TX_FIFO( );
Function : write data to TX fifo
INTPUT   : txbuffer, a buffer stores TX array
           size,  how many bytes should be written
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_W_TX_FIFO( INT8U *txbuffer, INT8U size )
{
    RF_SPINSEL(0);
    SPI_SendByteData( WRITE_TX_FIFO );
    while( size -- )    
    { 
    	SPI_SendByteData( *txbuffer++ ); 
    }
    RF_SPINSEL(1);
}

/*
=================================================================================
SI446X_SEND_DATA( );
Function : send data , data length > 64
INTPUT   : txbuffer, a buffer stores TX array
           size,  how many bytes should be written
           channel, tx channel
           condition, tx condition
OUTPUT   : sent flag : 1 success, 0 failed
=================================================================================
*/
static void  SI446X_SEND_DATA( INT8U *txbuffer, INT8U size, INT8U channel, INT8U condition )
{
    INT8U cmd[5];
    INT8U tx_len = size;
    INT8U i = 0;
   

//    SI446X_TX_FIFO_RESET( );
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

    RF_SPINSEL(0);
    SPI_SendByteData( WRITE_TX_FIFO );
#if PACKET_LENGTH == 0
    //tx_len ++;
    tx_len += 1;//
    SPI_SendByteData( size );
#endif
	#if 0
    while( size -- )    
    { 
    	SPI_SendByteData( *txbuffer++ ); 
    }
    #endif
    for (i = 0; i < size; i++)
    {
    	SPI_SendByteData( txbuffer[i]); 
    }
    RF_SPINSEL(1);
    cmd[0] = START_TX;
    cmd[1] = channel;
    cmd[2] = condition;
    cmd[3] = 0;
    cmd[4] = tx_len;
    SI446X_CMD( cmd, 5 );
	RF_SPINSEL(1);
}




/*
=================================================================================
SI446X_START_TX( );
Function : start TX command
INTPUT   : channel, tx channel
           condition, tx condition
           tx_len, how many bytes to be sent
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_START_TX( INT8U channel, INT8U condition, INT16U tx_len )
{
    INT8U cmd[5];

    cmd[0] = START_TX;
    cmd[1] = channel;
    cmd[2] = condition;
    cmd[3] = tx_len>>8;
    cmd[4] = tx_len;
    SI446X_CMD( cmd, 5 );
}
/*
=================================================================================
SI446X_READ_PACKET( );
Function : read RX fifo
INTPUT   : buffer, a buffer to store data read
OUTPUT   : received bytes
=================================================================================
*/
static INT8U SI446X_READ_PACKET( INT8U *buffer )
{
    INT8U length = 0;
    INT8U i = 0;;
    //user add
    //INT8U packetLen = 0;
    INT8U *pBuf = buffer+1;
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);

    SPI_SendByteData( READ_RX_FIFO );
#if PACKET_LENGTH == 0
    length = SPI_SendByteData( 0xFF );
    //user add
    
    buffer[0] = SPI_SendByteData( 0xFF );

    XPRINTF((0, "L = %02x, bL = %02x\r\n", length, buffer[0]));
    if (length >= (buffer[0] + 1))
    	length = buffer[0];
    else
    	length = buffer[0];
#else
    length = PACKET_LENGTH;
#endif
    i = length;
    while( length -- )
    {
        //*buffer++ = SPI_SendByteData( 0xFF );
        *pBuf++ = SPI_SendByteData( 0xFF );
    }
    RF_SPINSEL(1);
    return i;
}

static void SI446X_READ_PACKET_LEN( INT8U *pibuf )
{
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);

    SPI_SendByteData( READ_RX_FIFO );
#if PACKET_LENGTH == 0
    pibuf[0] = SPI_SendByteData( 0xFF );
    pibuf[1] = SPI_SendByteData( 0xFF );
    RF_SPINSEL(1);
#endif
}




/*
=================================================================================
SI446X_READ_DATA_TO_BUF( );
Function : read RX fifo
INTPUT   : buffer, a buffer to store data read
OUTPUT   : received bytes
=================================================================================
*/
static INT8U SI446X_READ_DATA_TO_BUF( INT8U *pbuf , INT8U ubNum)
{
	INT8U length;
    INT8U i = 0;
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);

    SPI_SendByteData( READ_RX_FIFO );
    length = ubNum;
    i = length;
    while( length -- )
    {
        //*buffer++ = SPI_SendByteData( 0xFF );
        *pbuf++ = SPI_SendByteData( 0xFF );
    }
    RF_SPINSEL(1);
    return i;
}



/*
=================================================================================
SI446X_START_RX( );
Function : start RX state
INTPUT   : channel, receive channel
           condition, receive condition
           rx_len, how many bytes should be read
           n_state1, next state 1
           n_state2, next state 2
           n_state3, next state 3
OUTPUT   : NONE
=================================================================================
*/
//static void SI446X_START_RX( INT8U channel, INT8U condition, INT16U rx_len,
//                      INT8U n_state1, INT8U n_state2, INT8U n_state3 )

static void SI446X_START_RX( SI446X_RF_CH channel, SI446X_RX_CONDITION condition, INT16U rx_len,
                      SI446X_DEV_STATE n_state1, SI446X_DEV_STATE n_state2, SI446X_DEV_STATE n_state3 )
                      
{
    INT8U cmd[8];
    //SI446X_RX_FIFO_RESET( );
    //SI446X_TX_FIFO_RESET( );
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

    cmd[0] = START_RX;
    cmd[1] = channel;
    cmd[2] = condition;
    cmd[3] = rx_len>>8;
    cmd[4] = rx_len;
    cmd[5] = n_state1;
    cmd[6] = n_state2;
    cmd[7] = n_state3;
    SI446X_CMD( cmd, 8 );
}
/*
=================================================================================
SI446X_RX_FIFO_RESET( );
Function : reset the RX FIFO of the device
INTPUT   : None
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_RX_FIFO_RESET( void )
{
    INT8U cmd[2];

    cmd[0] = FIFO_INFO;
    cmd[1] = 0x02;
    SI446X_CMD( cmd, 2 );
}
/*
=================================================================================
SI446X_TX_FIFO_RESET( );
Function : reset the TX FIFO of the device
INTPUT   : None
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_TX_FIFO_RESET( void )
{
    INT8U cmd[2];

    cmd[0] = FIFO_INFO;
    cmd[1] = 0x01;
    SI446X_CMD( cmd, 2 );
}


/*
=================================================================================
SI446X_TXRX_FIFO_RESET( );
Function : reset the TX AND RX FIFO of the device
INTPUT   : None
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_TXRX_FIFO_RESET( void )
{
    INT8U cmd[2];

    cmd[0] = FIFO_INFO;
    cmd[1] = 0x03;
    SI446X_CMD( cmd, 2 );
}



/*
=================================================================================
SI446X_PKT_INFO( );
Function : read packet information
INTPUT   : buffer, stores the read information
           FIELD, feild mask
           length, the packet length
           diff_len, diffrence packet length
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_PKT_INFO( INT8U *buffer, INT8U FIELD, INT16U length, INT16U diff_len )
{
    INT8U cmd[6];
    cmd[0] = PACKET_INFO;
    cmd[1] = FIELD;
    cmd[2] = length >> 8;
    cmd[3] = length;
    cmd[4] = diff_len >> 8;
    cmd[5] = diff_len;

    SI446X_CMD( cmd, 6 );
    SI446X_READ_RESPONSE( buffer, 3 );
}

/*
=================================================================================
SI446X_PKT_INFO_LEN( );
Function : read packet information
INTPUT   : buffer, stores the read information
           FIELD, feild mask
           length, the packet length
           diff_len, diffrence packet length
OUTPUT   : NONE
=================================================================================
*/
static INT8U SI446X_PKT_INFO_LEN(INT8U FIELD, INT16U length, INT16U diff_len )
{
    INT8U cmd[6];
    cmd[0] = PACKET_INFO;
    cmd[1] = FIELD;
    cmd[2] = length >> 8;
    cmd[3] = length;
    cmd[4] = diff_len >> 8;
    cmd[5] = diff_len;

    SI446X_CMD( cmd, 6 );
    SI446X_READ_RESPONSE( cmd, 3 );
    return cmd[2];
}

/*
=================================================================================
SI446X_FIFO_INFO( );
Function : read fifo information
INTPUT   : buffer, stores the read information
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_FIFO_INFO( INT8U *buffer )
{
    INT8U cmd[2];
    cmd[0] = FIFO_INFO;
    cmd[1] = 0x03;

    SI446X_CMD( cmd, 2 );
    SI446X_READ_RESPONSE( buffer, 3);
}
/*
=================================================================================
SI446X_GPIO_CONFIG( );
Function : config the GPIOs, IRQ, SDO
INTPUT   :
OUTPUT   : NONE
=================================================================================
*/
static void SI446X_GPIO_CONFIG( INT8U G0, INT8U G1, INT8U G2, INT8U G3,
                         INT8U IRQ, INT8U SDO, INT8U GEN_CONFIG )
{
    INT8U cmd[10];
    cmd[0] = GPIO_PIN_CFG;
    cmd[1] = G0;
    cmd[2] = G1;
    cmd[3] = G2;
    cmd[4] = G3;
    cmd[5] = IRQ;
    cmd[6] = SDO;
    cmd[7] = GEN_CONFIG;
    SI446X_CMD( cmd, 8 );
    SI446X_READ_RESPONSE( cmd, 8 );
}

/*
* \breif config global_config for reconfig fifo tx and rx  two FIFOs are combined 
*		 into a single 129-byte shared FIFO
* note user add                    
*/

static void SI446X_GLOGAL_CFG_CMMBINED_FIFO(void)
{
	SI446X_SET_PROPERTY_1(GLOBAL_CONFIG, 0x30);
}


/*! This function sends the PART_INFO command to the radio and receives the answer
 *  into @Si446xCmd union.
 */
static void SI446X_GET_PART_INFO(void)
{
	INT8U ubacmd[16] = {0};
	INT8U *pcmd = (INT8U *)&ubacmd[1];
	
	SI446X_PART_INFO(ubacmd);
	
    Si446xCmd.PART_INFO.CHIPREV         = pcmd[0];
    Si446xCmd.PART_INFO.PART            = ((U16)pcmd[1] << 8) & 0xFF00;
    Si446xCmd.PART_INFO.PART           |= (U16)pcmd[2] & 0x00FF;
    Si446xCmd.PART_INFO.PBUILD          = pcmd[3];
    Si446xCmd.PART_INFO.ID              = ((U16)pcmd[4] << 8) & 0xFF00;
    Si446xCmd.PART_INFO.ID             |= (U16)pcmd[5] & 0x00FF;
    Si446xCmd.PART_INFO.CUSTOMER        = pcmd[6];
    Si446xCmd.PART_INFO.ROMID           = pcmd[7];
}

/* Full driver support functions */

/*!
 * Sends the FUNC_INFO command to the radio, then reads the resonse into @Si446xCmd union.
 */
static void SI446X_GET_FUNC_INFO(void)
{
	INT8U ubacmd[16] = {0x00};
	INT8U *pcmd = (INT8U *)&ubacmd[1];

	SI446X_FUNC_INFO(ubacmd);
	
    Si446xCmd.FUNC_INFO.REVEXT          = pcmd[0];
    Si446xCmd.FUNC_INFO.REVBRANCH       = pcmd[1];
    Si446xCmd.FUNC_INFO.REVINT          = pcmd[2];
    Si446xCmd.FUNC_INFO.FUNC            = pcmd[5];
}

/*!
 * Get the Interrupt status/pending flags form the radio and clear flags if requested.
 *
 * @param PH_CLR_PEND     Packet Handler pending flags clear.
 * @param MODEM_CLR_PEND  Modem Status pending flags clear.
 * @param CHIP_CLR_PEND   Chip State pending flags clear.
 */
static void SI446X_GET_INT_STATUS(void)
{
	INT8U ubacmd[16] = {0x00};
	INT8U *pcmd = (INT8U *)&ubacmd[1];

	SI446X_INT_STATUS(ubacmd);
	
    Si446xCmd.GET_INT_STATUS.INT_PEND       = pcmd[0];
    Si446xCmd.GET_INT_STATUS.INT_STATUS     = pcmd[1];
    Si446xCmd.GET_INT_STATUS.PH_PEND        = pcmd[2];
    Si446xCmd.GET_INT_STATUS.PH_STATUS      = pcmd[3];
    Si446xCmd.GET_INT_STATUS.MODEM_PEND     = pcmd[4];
    Si446xCmd.GET_INT_STATUS.MODEM_STATUS   = pcmd[5];
    Si446xCmd.GET_INT_STATUS.CHIP_PEND      = pcmd[6];
    Si446xCmd.GET_INT_STATUS.CHIP_STATUS    = pcmd[7];
}



/*!
 * Requests the current state of the device and lists pending TX and RX requests
 */
static void SI446X_REQUEST_DEVICE_STATE(void)//si446x_request_device_state(void)
{
                              
    INT8U ubacmd[16] = {0x00};
	INT8U *pcmd = (INT8U *)&ubacmd[1];
	
    ubacmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;

    SI446X_CMD(ubacmd, SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE);
    SI446X_READ_RESPONSE(ubacmd, SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE+1);

    Si446xCmd.REQUEST_DEVICE_STATE.CURR_STATE       = pcmd[0];
    Si446xCmd.REQUEST_DEVICE_STATE.CURRENT_CHANNEL  = pcmd[1];
}


/*!
 * Requests the current state of the device and lists pending TX and RX requests
 */
static void SI446X_REQUEST_DEVICE_STATE_CH(INT8U *pState, INT8U *pChannel)//si446x_request_device_state(void)
{
	#if 0
    Pro2Cmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;
    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd );
    #endif
                              
    INT8U ubacmd[16] = {0x00};
	INT8U *pcmd = (INT8U *)&ubacmd[1];
	
    ubacmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;

    SI446X_CMD(ubacmd, SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE);
    SI446X_READ_RESPONSE(ubacmd, SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE+1);

    *pState = pcmd[0];
    *pChannel = pcmd[1];
}

/*!
 * Gets the Modem status flags. Optionally clears them.
 *
 * @param MODEM_CLR_PEND Flags to clear.
 */
static void SI446X_GET_MODEM_STATUS( INT8U MODEM_CLR_PEND )
{

    INT8U ubacmd[16] = {0x00};
	INT8U *pcmd = (u_char *)&ubacmd[1];

    ubacmd[0] = SI446X_CMD_ID_GET_MODEM_STATUS;
    ubacmd[1] = MODEM_CLR_PEND;
    
    SI446X_CMD(ubacmd, SI446X_CMD_ARG_COUNT_GET_MODEM_STATUS);
    SI446X_READ_RESPONSE(ubacmd, SI446X_CMD_REPLY_COUNT_GET_MODEM_STATUS+1);


    Si446xCmd.GET_MODEM_STATUS.MODEM_PEND   = pcmd[0];
    Si446xCmd.GET_MODEM_STATUS.MODEM_STATUS = pcmd[1];
    Si446xCmd.GET_MODEM_STATUS.CURR_RSSI    = pcmd[2];
    Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI   = pcmd[3];
    Si446xCmd.GET_MODEM_STATUS.ANT1_RSSI    = pcmd[4];
    Si446xCmd.GET_MODEM_STATUS.ANT2_RSSI    = pcmd[5];
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET =  ((U16)pcmd[6] << 8) & 0xFF00;
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET |= (U16)pcmd[7] & 0x00FF;
}






/*!
 * GET current rssi
 *
 * @param MODEM_CLR_PEND Flags to clear.
 */
static INT8U SI446X_GET_CUR_RSSI(void)
{

    INT8U ubacmd[16] = {0x00};
	INT8U *pcmd = (u_char *)&ubacmd[1];

    ubacmd[0] = SI446X_CMD_ID_GET_MODEM_STATUS;
    ubacmd[1] = 0x7f;
    
    SI446X_CMD(ubacmd, SI446X_CMD_ARG_COUNT_GET_MODEM_STATUS);
    SI446X_READ_RESPONSE(ubacmd, SI446X_CMD_REPLY_COUNT_GET_MODEM_STATUS+1);

	/*
    Si446xCmd.GET_MODEM_STATUS.MODEM_PEND   = pcmd[0];
    Si446xCmd.GET_MODEM_STATUS.MODEM_STATUS = pcmd[1];
    Si446xCmd.GET_MODEM_STATUS.CURR_RSSI    = pcmd[2];
    Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI   = pcmd[3];
    Si446xCmd.GET_MODEM_STATUS.ANT1_RSSI    = pcmd[4];
    Si446xCmd.GET_MODEM_STATUS.ANT2_RSSI    = pcmd[5];
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET =  ((U16)pcmd[6] << 8) & 0xFF00;
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET |= (U16)pcmd[7] & 0x00FF;
    */
    return pcmd[2];
}


/*
=================================================================================
SI446X_GET_FIFO_INFO_RX_BYTES( );
Function : read fifo information
INTPUT   : none
OUTPUT   : rx fifo bytes
=================================================================================
*/
static INT8U SI446X_GET_FIFO_INFO_RX_BYTES(void)
{
    INT8U cmd[2];
    INT8U ubaBuf[3] = {0x00};
    cmd[0] = FIFO_INFO;
    cmd[1] = 0x00;

    SI446X_CMD( cmd, 2 );
    SI446X_READ_RESPONSE( ubaBuf, 3);
    return ubaBuf[1];
}



/*------------------------------------------------------------------------------------------*/

static volatile INT8U si446x_ubpksent = 0;

//void si446x_frr_ctl_cfg(void);


void si446x_set_pksent(void)
{
	si446x_ubpksent = 1;
}


static int si446x_wait_pksent(void)
{
	#if 1
	INT32U timeout = 0;
	si446x_ubpksent = 0;
	while(!si446x_ubpksent)
	{
		timeout++;
		if ( timeout > 7200000)//time out
		{
			return 1;
		}
	}
	si446x_ubpksent = 0;
	return 0;
	#else
	#endif
}


//close nirq pin interrupt
static void si446x_nirq_disable(void)
{
	//RF_NIRQ_DISABLE();
	
}

static void clock_wait(u_long udwTime)
{
	unsigned int i,k;
	for(k = 0; k < udwTime; k++)
	{
		for(i = 0; i<3190; i++)
		{
			;
		}
	}		
}

static void si446x_rst( )
{
    RF_SDN(1);
    clock_wait(1);
    /*delay code*/
    
    RF_SDN(0);
    /*delay code*/
    clock_wait(20);
    RF_SPINSEL(1);   
}

static void si446x_nIRQ_Config(void)
{
	//RF_NIRQ_CFG( );
	EXTI_SetPinSensitivity(EXTI_Pin_4, EXTI_Trigger_Falling);
	GPIO_Init(RF_NIRQ_PORT, RF_NIRQ_PIN, GPIO_Mode_In_PU_IT);
}

//open nirq pin interrupt
static void si446x_nirq_enable(void)
{

}

void si446xRadioInit(void)
{
	rfchnum = 0;

	si446x_nirq_disable( );
	//SPI_Config( );
	spiInit( );

    si446x_rst( );        //SI446X 模块复位
    
    SI446X_RADIO_CONFIG_INIT( ); //config radio param

    SI446X_RADIO_PKT_COMMON_CFG( );
    SI446X_RADIO_TX_FIELD_CFG( );
    SI446X_RADIO_RX_FIELD_CFG( );
    
	si446x_get_int_status( );
	memset(Si446xCmd.RAW, 0, 16);

    si446x_nIRQ_Config( );
    si446xStartRX( );
}


void si446x_recfg(void)
{
	si446x_nirq_disable( );
	//SPI_Config( );
    spiInit( );
	
    si446x_rst( );        //SI446X 模块复位
    SI446X_RADIO_CONFIG_INIT( ); //config radio param

    SI446X_RADIO_PKT_COMMON_CFG( );
    SI446X_RADIO_TX_FIELD_CFG( );
    SI446X_RADIO_RX_FIELD_CFG( );

    si446x_get_int_status( );
	    
    si446x_nIRQ_Config( );
    si446xStartRX( );
}



u_char si446x_get_rfch(void)
{
	return rfchnum;
}


u_char si446x_set_rfch(u_char ubchannel)
{
	u_char ubOldchannel = rfchnum;
	rfchnum = ubchannel;
	return ubOldchannel;
}


/*
* \brief This funtion is used to init tx, rx fifo, and combine two 64 bytes fifo to 129 bytes fifo
*        and start rx at HWLH_TEST_CH channel, 
*/
void si446xStartRX(void)
{
	si446x_get_int_status( );
	memset(Si446xCmd.RAW, 0, 16);
	//SI446X_START_RX(rfchnum, 0, 0, 0,0x00,0x03);
	SI446X_START_RX(rfchnum, 0, 0, 0,0x03,0x08);
}

/*
* \brief This funtion is used to init tx, rx fifo, and combine two 64 bytes fifo to 129 bytes fifo 
*/
void si446xToRx(void)
{
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );
	SI446X_CHANGE_STATE(8);
}


void si446x_reset_fifo(void)
{
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  data        Pointer to data that is to be written to frame buffer, data[0] is data length, data[0]+1
 *						is total packet length.
 *  \param  len         Length of data. The maximum length is large 64 bytes.
 */
int si446xRadioSendFixLenData(const INT8U *pubdata)
{
	INT8U ubDataNLen = 0;
	const INT8U *pbuf = pubdata;
	int nResult = 0;
	int i = 0;
	//u_char ubState;
	//u_char ubCh;
	
	SI446X_CHANGE_STATE(5);
	//SI446X_CHANGE_STATE(3);
	ubDataNLen = pubdata[0] + 1;
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

	si446x_get_int_status( );
	memset(Si446xCmd.RAW, 0, 16);


    RF_SPINSEL(0);
    SPI_SendByteData(WRITE_TX_FIFO);
    for (i = 0; i < ubDataNLen; i++)
    {
    	SPI_SendByteData( pbuf[i]); 
    }
    RF_SPINSEL(1);
    
	//SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PSENT_EN);
	//SI446X_SET_PROPERTY_1(INT_CTL_MODEM_ENABLE, 0x00);
	//SI446X_START_TX(HWLH_TEST_CH, 0, ubDataNLen);
	//SI446X_START_TX(rfchnum, 0, ubDataNLen);
	
	SI446X_START_TX(rfchnum, 0x80, ubDataNLen);
	nResult = si446x_wait_pksent( );
	si446xStartRX( );
	return nResult;
}

static void si446xWriteMutiData(const INT8U *pcBuf, INT8U ubDataLen)
{
	int i = 0;
	SI446X_WAIT_CTS( );

    RF_SPINSEL(0);
    SPI_SendByteData( WRITE_TX_FIFO );
    for (i = 0; i < ubDataLen; i++)
    {
    	SPI_SendByteData( pcBuf[i]);
    }
	RF_SPINSEL(1);
}

void si446xReadMutiData(INT8U *pBuf)
{
	int i = 0;
	INT8U ubLen = 0;
	u_char *pData = pBuf+1;
	//ubLen = SI446X_PKT_INFO_LEN(0, 0, 0);
	//ubLen = SI446X_GET_FIFO_INFO_RX_BYTES( );
	SI446X_WAIT_CTS( );
    RF_SPINSEL(0);
    SPI_SendByteData( READ_RX_FIFO );
    ubLen = SPI_SendByteData(0xFF);
    //ubLen = pData[0];
    if (ubLen + 1 > 128 )
    {
    	pBuf[0] = 0;
    	return;
    }
    for (i = 0; i < ubLen; i++)
    {
    	pData[i]=SPI_SendByteData(0xFF);
    }
    pBuf[0] = ubLen;
	RF_SPINSEL(1);
}


static void si446x_frr_ctl_cfg(void)
{
	SI446X_SET_PROPERTY_1(FRR_CTL_A_MODE, FRR_MODE_INT_CHIP_PEND);
	SI446X_SET_PROPERTY_1(FRR_CTL_B_MODE, FRR_MODE_INT_PH_PEND);
	SI446X_SET_PROPERTY_1(FRR_CTL_C_MODE, FRR_MODE_INT_MODEM_PEND);
	SI446X_SET_PROPERTY_1(FRR_CTL_D_MODE, FRR_MODE_CURRENT_STATE);
}

/*
* \brief start rx
* \param channel   the rf channel is used for receive data
* \param condition start rx immediate or after WUT
* \param rx_len    receive data length, when zero receive variable data, not zero receive fix len data
* \param n_state1  This parameter selects the desired operational state of the chip to automatically enter upon timeout of Preamble detection.
* \param n_state2  This parameter selects the desired operational state of the chip to automatically enter upon reception of a valid packet.
* \parma n_state3  This parameter selects the desired operational state of the chip to automatically enter upon reception of an invalid packet.
*/

void si446x_start_rx( SI446X_RF_CH channel, SI446X_RX_CONDITION condition, INT16U rx_len,
                      SI446X_DEV_STATE n_state1, SI446X_DEV_STATE n_state2, SI446X_DEV_STATE n_state3)
{
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );
	//SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PRX_EN|SI446X_INT_CTL_PH_CRCE_EN);
	//SI446X_SET_PROPERTY_1(INT_CTL_MODEM_ENABLE, SI446X_INT_CTL_MODEM_SYDE_EN);
	//SI446X_START_RX( channel, 0, 0,0,0,0x03 );
	SI446X_START_RX( channel, condition, rx_len, n_state1, n_state2, n_state3 );
}


/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  pubdata        Pointer to data that is to be written to frame buffer.
 *  \param  channel        radio send channel
 *  \param  condition	   radio condition after send packet
 *  \param  tx_len		   send data length
 */
int si446x_send_fix_len_data(const INT8U *pubdata, SI446X_RF_CH channel, SI446X_TX_CONDITION condition, INT16U tx_len)
{
	//when data length > 64, 
	const INT8U *pbuf = pubdata;
	int nResult = 0;
	int i = 0;

	SI446X_CHANGE_STATE(5);

    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

    RF_SPINSEL(0);
    SPI_SendByteData(WRITE_TX_FIFO);
    for (i = 0; i < tx_len; i++)
    {
    	SPI_SendByteData( pbuf[i]); 
    }
    RF_SPINSEL(1);
    
	//SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PSENT_EN);
	//SI446X_SET_PROPERTY_1(INT_CTL_MODEM_ENABLE, 0x00);
	//SI446X_START_TX(HWLH_TEST_CH, 0, ubDataNLen);
	SI446X_START_TX(channel, condition, tx_len);

	nResult = si446x_wait_pksent( );
	si446xToRx( );
	return nResult;
}

/*
* \brief init the radio param
* \note before this, need to init spi
*/
void si446x_radio_init(void)
{
    SI446X_RADIO_CONFIG_INIT( ); //config radio param
    SI446X_RADIO_PKT_COMMON_CFG( );
    SI446X_RADIO_TX_FIELD_CFG( );
    SI446X_RADIO_RX_FIELD_CFG( );

    si446x_frr_ctl_cfg( );

    si446x_nIRQ_Config( );
    si446xStartRX( );
}


/*
* \brief Read specify length from fifo
* \param pioBuf the pointer to the buf that save data
* \param ubLen data length
*/

void si446x_read_data(INT8U *pioBuf, INT8U ubLen)
{
	int i = 0;
	SI446X_WAIT_CTS( );
    RF_SPINSEL(0);
    SPI_SendByteData( READ_RX_FIFO );
    for (i = 0; i < ubLen; i++)
    {
    	pioBuf[i]=SPI_SendByteData(0xFF);
    }
	RF_SPINSEL(1);
}



/*
 * \brief  This function set radio tx power
 *
 * \param  ubPower   New tx power value need to be set  must be <= 0x7f  
*/

void si446x_set_tx_power(SI446X_TX_POWER_DBM e_txdbm)
{
	SI446X_SET_PROPERTY_1(PA_PWR_LVL, e_txdbm&0x7f);
}


/*
 * \brief  This function get current radio tx power
 * \return current tx power
*/

SI446X_TX_POWER_DBM  si446x_get_current_tx_power(void)
{
	return (SI446X_TX_POWER_DBM)SI446X_GET_PROPERTY_1(PA_PWR_LVL);
}


/*

*/
void si446x_change_dev_current_state(SI446X_DEV_STATE e_state)
{
	SI446X_CHANGE_STATE((INT8U)e_state);
}


/*
* \brief This funtion is used to read all interrupt values and clear interrupt pend
*        all interrupt value is save to Si446xCmd
*/
void si446x_get_int_status(void)
{
	SI446X_GET_INT_STATUS( );
}

INT8U si446x_get_property_1(SI446X_PROPERTY GROUP_NUM)
{
	return SI446X_GET_PROPERTY_1(GROUP_NUM);
}

/*
* \brief This funtion is used to read all modem values 
*        all modem value is save to Si446xCmd
*/
void si446x_get_modem_status(INT8U MODEM_CLR_PEND)
{
	SI446X_GET_MODEM_STATUS(MODEM_CLR_PEND);
}

INT8U si446x_get_cur_rssi(void)
{
	return SI446X_GET_CUR_RSSI( );
}

#if 0
/*
* \brief Fast read A mode
* \param pioBuf The pointer to buf that save data
* \param ubLen  read data lengh
* \note ubLen <=4
*/
void si446x_frr_a_read(INT8U *pioBuf, INT8U ubLen)
{
	int i = 0;
	RF_SPINSEL(0);
	SPI_SendByteData(SI446X_CMD_ID_FRR_A_READ);
	for (i = 0; i < ubLen; i++)
	{
		pioBuf[i] = SPI_SendByteData(0xff);
	}
	RF_SPINSEL(1); 
}

/*
* \brief Fast read b mode
* \param pioBuf The pointer to buf that save data
* \param ubLen  read data lengh
* \note ubLen <=4
*/
void si446x_frr_b_read(INT8U *pioBuf, INT8U ubLen)
{
	int i = 0;
	INT8U ubCmd = SI446X_CMD_ID_FRR_B_READ;
	SI446X_CMD(&ubCmd, 1);
	SI446X_READ_RESPONSE(pioBuf, ubLen);
}


/*
* \brief Fast read c mode
* \param pioBuf The pointer to buf that save data
* \param ubLen  read data lengh
* \note ubLen <=4
*/
void si446x_frr_c_read(INT8U *pioBuf, INT8U ubLen)
{
	int i = 0;
	RF_SPINSEL(0);
	SPI_SendByteData(SI446X_CMD_ID_FRR_C_READ);
	for (i = 0; i < ubLen; i++)
	{
		pioBuf[i] = SPI_SendByteData(0xff);
	}
	RF_SPINSEL(1); 
}




/*
* \brief Fast read d mode
* \param pioBuf The pointer to buf that save data
* \param ubLen  read data lengh
* \note ubLen <=4
*/
void si446x_frr_d_read(INT8U *pioBuf, INT8U ubLen)
{
	int i = 0;
	RF_SPINSEL(0);
	SPI_SendByteData(SI446X_CMD_ID_FRR_D_READ);
	for (i = 0; i < ubLen; i++)
	{
		pioBuf[i] = SPI_SendByteData(0xff);
	}
	RF_SPINSEL(1); 
}



INT8U si446x_get_frr_chip_pend(void)
{
	INT8U ubPend = 0x00;
	si446x_frr_a_read(&ubPend, 1);
	return ubPend;
}

INT8U si446x_get_frr_ph_pend(void)
{
	INT8U ubaPend[5] = 0x00;
	si446x_frr_b_read(ubaPend, 5);
	MEM_DUMP(10, "PH", ubaPend, 5);
	return ubaPend[1];
}

INT8U si446x_get_frr_modem_pend(void)
{
	INT8U ubaPend[5] = {0x00};
	si446x_frr_c_read(ubaPend, 5);
	return ubaPend[1];
}


INT8U si446x_get_frr_current_state(void)
{
	INT8U ubPend = 0x00;
	si446x_frr_d_read(&ubPend, 1);
	return ubPend;
}



/*!
 * Clear all Interrupt status/pending flags. Does NOT read back interrupt flags
 *
 */
void si446x_int_status_fast_clear( void )
{
    INT8U ubCmd = SI446X_CMD_ID_GET_INT_STATUS;
	SI446X_CMD(&ubCmd, 1);
}
#endif


