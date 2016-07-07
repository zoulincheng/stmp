#include "stm8l15x.h"
#include "radio.h"
#include "lib/random.h"
#include "basictype.h"

//#include "si4438.h"
#include "si446x_defs_u.h"
#include "si446x_cmd.h"
#include "si446x.h"

#include "sysprintf.h"
#include <string.h>
#include "stm8l15x_it.h"



/*---------------------------------------------------------------------------*/
static int si446x_radio_rf_init(void);
static int si446x_radio_prepare(const void *payload,unsigned short payload_len);
static int si446x_radio_transmit(unsigned short payload_len);
static int si446x_radio_send(const void *data, unsigned short len);
static int si446x_radio_read(void *buf, unsigned short bufsize);
static int si446x_radio_channel_clear(void);
static int si446x_radio_receiving_packet(void);
static int si446x_radio_pending_packet(void);
static int si446x_radio_on(void);
static int si446x_radio_off(void);
static int add_to_rxbuf(uint8_t * src);
static int read_from_rxbuf(void *dest, unsigned short len);

#define SI446X_MAX_PACKET_LEN	32
#define SI446X_CUR_RSSI			31

/*
 * The buffers which hold incoming data.
 */
#ifndef SI446X_RADIO_RXBUFS
#define SI446X_RADIO_RXBUFS 4
#endif 


/* +1 because of the first byte, which will contain the length of the packet. */
// +1 last data is rssi
//len1 [data] rssi1
static uint8_t si446x_rxbufs[SI446X_RADIO_RXBUFS][SI446X_MAX_PACKET_LEN];//len1 [data] rssi1  last_rssi


#if SI446X_RADIO_RXBUFS > 1
static volatile int8_t first = -1, last = 0;
#else   
static const int8_t first = 0, last = 0;
#endif  


#if SI446X_RADIO_RXBUFS > 1
#define CLEAN_RXBUFS() do{first = -1; last = 0;}while(0)
#define RXBUFS_EMPTY() (first == -1)
static int RXBUFS_FULL( )
{
	int8_t first_tmp = first;
	return first_tmp == last;
}
#else 
#define CLEAN_RXBUFS( ) (si446x_rxbufs[0][0] = 0)
#define RXBUFS_EMPTY( ) (si446x_rxbufs[0][0] == 0)
#define RXBUFS_FULL( ) (si446x_rxbufs[0][0] != 0)
#endif 

static uint8_t si446x_txbuf[SI446X_MAX_PACKET_LEN];

#define CLEAN_TXBUF() (si446x_txbuf[0] = 0)
#define TXBUF_EMPTY() (si446x_txbuf[0] == 0)

#define BUSYWAIT_UNTIL(cond, max_time)

static u_char ubcurrssi;
static uint8_t locked;

static volatile uint8_t is_transmitting;
static volatile uint8_t seqnum;

static volatile uint8_t rf_state_led_lock = 0;

volatile  SI446X_STATE si446x_state = SI446X_IDLE;
volatile uint8_t ubRxFlag;

volatile u_short  count_tx = 0;
volatile u_short  count_rx = 0;

#define RF_RSSI_THD	   100

extern void sysPrintExp(unsigned int dwPos);
extern u_long sysGetLR(void);

/* If set to 1, a send() returns only after the packet has been transmitted.
  This is necessary if you use the x-mac module, for example. */
#ifndef RADIO_WAIT_FOR_PACKET_SENT
#define RADIO_WAIT_FOR_PACKET_SENT 1
#endif  /* RADIO_WAIT_FOR_PACKET_SENT */


static u_char ubarxbuf[129] = {0x00};

#define GET_LOCK() locked++

/*--------------------------------------------------------------------------*/
static void RELEASE_LOCK(void)
{
	if(locked > 0)
		locked--;
}


/*---------------------------------------------------------------------------*/
static int si446x_radio_rf_init(void)
{
	si446xRadioInit( );
	
	locked = 0;
	si446x_state = SI446X_IDLE;

	CLEAN_RXBUFS();
	CLEAN_TXBUF();

	return 0;
}

/*---------------------------------------------------------------------------*/
static int si446x_radio_transmit(unsigned short payload_len)
{
	#define DEBUGTEST 1
	//si446x_txbuf[0] = payload_len;  //data packet length 
	si446x_txbuf[0] = payload_len + 2;  //data packet length  + 2 BYTE CRC

	GET_LOCK();

	while((si446x_state & SI446X_TX) || (si446x_state & SI446X_RX))
	{	
        /* we are not transmitting. This means that
           we just started receiving a packet or sending a paket,
           so we drop the transmission. */
		XPRINTF((10, "SEND DROP-----------\r\n"));       
		RELEASE_LOCK();
        return RADIO_TX_COLLISION;
     }

	si446x_state = SI446X_TX;
	is_transmitting = 1;
	if(si446xRadioSendFixLenData(si446x_txbuf) == 0x00) 
	{
		#if  DEBUGTEST > 0
		{
			MEM_DUMP(10, "TX->",(u_char*)si446x_txbuf, si446x_txbuf[0]+1);
		}
		#endif
		CLEAN_TXBUF( );
		is_transmitting = 0;
		RELEASE_LOCK( );
		si446x_state = SI446X_IDLE; // when send packet, return IDLE state
		//memset((u_char*)si4432_txbuf, 0, si4432_txbuf[0] + 1);//clear txbuf
		return RADIO_TX_OK;
	}
	else
	{
		RELEASE_LOCK( );
		/* TODO: Do we have to retransmit? */
		CLEAN_TXBUF( );
		XPRINTF((10, "transmit error\r\n"));
		si446x_recfg( );//init again 
		is_transmitting = 0;
		si446x_state = SI446X_IDLE; // when send packet, return IDLE state
		return RADIO_TX_ERR;
	}
}

/*---------------------------------------------------------------------------*/
static int si446x_radio_prepare(const void *payload, unsigned short payload_len)
{
	u_short uwCrc = 0;
	GET_LOCK( );
	if(payload_len > SI446X_MAX_PACKET_LEN) 
	{
		XPRINTF((10,"SI4432: payload length=%d is too long.\r\n", payload_len));
		RELEASE_LOCK( );
		return RADIO_TX_ERR;
	}
	#if !RADIO_WAIT_FOR_PACKET_SENT
	/* 
	* Check if the txbuf is empty. Wait for a finite time.
	* This should not occur if we wait for the end of transmission in 
	* si446x_radio_transmit().
	*/
	if(wait_for_tx()) 
	{
		XPRINTF((10,"si446x: radio prepare tx buffer full.\r\n"));
		RELEASE_LOCK( );
		return RADIO_TX_ERR;
	}
	#endif /* RADIO_WAIT_FOR_PACKET_SENT */
	
	/*
	* Copy to the txbuf. 
	* The first byte must be the packet length.
	*/
	CLEAN_TXBUF();
	uwCrc = crc16_data((const u_char*)payload, payload_len,0);
	memcpy(si446x_txbuf+1, payload, payload_len);
	si446x_txbuf[payload_len + 1] = uwCrc &0xff;
	si446x_txbuf[payload_len + 2] = (uwCrc>>8)&0xff;

	RELEASE_LOCK( );
	return RADIO_TX_OK;
}


/*---------------------------------------------------------------------------*/
int si446x_radio_send(const void *payload, unsigned short payload_len)
{
	si446x_radio_prepare(payload, payload_len);
	return si446x_radio_transmit(payload_len);
}

/*---------------------------------------------------------------------------*/
static int si446x_radio_off(void)
{
	return 1;
}
/*---------------------------------------------------------------------------*/
static int si446x_radio_on(void)
{
	return 1;
}



/*---------------------------------------------------------------------------*/
static int si446x_radio_channel_clear(void)
{
  //return ST_RadioChannelIsClear();
  	if ((si446x_state&SI446X_RX_RECEIVING == SI446X_RX_RECEIVING)||(si446x_state&SI446X_TX == SI446X_TX))
  	{
  		return 0;//rf busy
  	}

  	//need be improve
  	if (si446x_get_cur_rssi( ) > RF_RSSI_THD)
  	{
  		return 0;//busy
  	}
	return 1;
}


/*---------------------------------------------------------------------------*/
static int si446x_radio_receiving_packet(void)
{
  //return receiving_packet;
    return si446x_state & SI446X_RX_RECEIVING;
}

/*---------------------------------------------------------------------------*/
static int si446x_radio_pending_packet(void)
{
  return !RXBUFS_EMPTY();
}


/*---------------------------------------------------------------------------*/
int si446x_radio_is_on(void)
{
  return 0;
}


/*---------------------------------------------------------------------------*/
static int si446x_radio_read(void *buf, unsigned short bufsize)
{
  return read_from_rxbuf(buf, bufsize);
}


/*---------------------------------------------------------------------------*/
static int add_to_rxbuf(uint8_t *src)
{
	if(RXBUFS_FULL()) 
	{
		return 0;
	}

	memcpy(si446x_rxbufs[last], src, src[0] + 1);//src[0] is data length in buf not include src[0], so all data length need to add 1 
	si446x_rxbufs[last][SI446X_CUR_RSSI] = src[SI446X_CUR_RSSI];
	//memcpy(si4432_rxbufs[last], src, src[0]);
	#if SI446X_RADIO_RXBUFS > 1
	last = (last + 1) % SI446X_RADIO_RXBUFS;
	if(first == -1) 
	{
		first = 0;
	}
	#endif
	
	memset(src, 0, src[0] + 1);//clear buf
	return 1;
}


/*---------------------------------------------------------------------------*/
static int read_from_rxbuf(void *dest, unsigned short len)
{
	//PRINTF("first 0   %d\r\n", first);
	u_char  packet_rssi;
	int8_t rssi;
	if(RXBUFS_EMPTY()) 
	{          
		return 0;
	}

	if(si446x_rxbufs[first][0] > len) 
	{   /* Too large packet for dest. */
		len = 0;
	} 
	else 
	{
		//len = si446x_rxbufs[first][0];
		len = si446x_rxbufs[first][0]-2;
		packet_rssi = si446x_rxbufs[first][SI446X_CUR_RSSI];
		rssi = -((int8_t)((0xff-packet_rssi)>>1));
		//memcpy(dest, (uint8_t*)&si446x_rxbufs[first][0] + 1, len);
		memcpy(dest, (uint8_t*)&si446x_rxbufs[first][0] + 1, len);
		//packetbuf_set_attr(PACKETBUF_ATTR_RSSI, last_rssi);
	}

	#if SI446X_RADIO_RXBUFS > 1
	{
		int first_tmp;
		first = (first + 1) % SI446X_RADIO_RXBUFS;
		first_tmp = first;
		if(first_tmp == last) 
		{
			CLEAN_RXBUFS();
		}
	}
	#else
	CLEAN_RXBUFS();
	#endif

	return len;
}





static int is_broadcast_addr(uint8_t mode, uint8_t *addr)
{

  return 1;
}



/******************************************************************/
//TX receive led state  red led
#define RF_CHECK_TIME_BASE			(30*60*CLOCK_SECOND)
#define RF_CHECK_TIME_RATE			(1000)

static void rf_check_process(void)
{
	if ( count_tx ==0 || count_rx == 0 )
	{
		ubRxFlag = 0;
		si446x_recfg( );
		si446x_state = SI446X_IDLE;

		XPRINTF((0, "rf recfg\n"));
	}
	XPRINTF((0, "tx %d, rx %d\n", count_tx, count_rx));
	count_tx = 0;
	count_rx = 0;
}



void static ph_prx_handler(void)
{
	si446xReadMutiData(ubarxbuf);

	if (ubarxbuf[0] != 0)
	{
		ubarxbuf[SI446X_CUR_RSSI] = ubcurrssi; //add rssi to last data position
	}
	si446xStartRX( );
	si446x_state = SI446X_IDLE;
	ubRxFlag = 0;	
}

void static modem_syde_handler(void)
{
	//u_char ubrssi = 0;
	si446x_get_modem_status(0x00);
	ubcurrssi = Si446xCmd.GET_MODEM_STATUS.CURR_RSSI;
	//last_rssi = -((int8_t)((0xff-ubcurrssi)>>1));
	//XPRINTF((0, "last_rssi = %d\r\n", last_rssi));
	ubarxbuf[0] = 0;
	si446x_state = SI446X_RX;
	ubRxFlag = 1;
}

void static chip_cmderror_handler(void)
{
	si446x_change_dev_current_state(SI446X_SLEEP);
	si446xStartRX( );
}


#if 0
/**
  * @brief External IT PIN4 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI4_IRQHandler, 12)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  EXTI_ClearITPendingBit(EXTI_IT_Pin4);
  Set_Int_Event( SPI_INT );
}
#endif

INTERRUPT_HANDLER(EXTI4_IRQHandler, 12)
{	
	U8 si446x_ph_pend = 0x00;
	U8 si446x_modem_pend = 0x00;
	U8 si446x_chip_pend = 0x00;

	EXTI_ClearITPendingBit(EXTI_IT_Pin4);

	//EXTI_ClearITPendingBit(EXTI_Line6);	
			
	si446x_get_int_status( );
	si446x_ph_pend = Si446xCmd.GET_INT_STATUS.PH_PEND;
	si446x_modem_pend = Si446xCmd.GET_INT_STATUS.MODEM_PEND;
	si446x_chip_pend = Si446xCmd.GET_INT_STATUS.CHIP_PEND;
	
	//packet sent finish
	if (si446x_ph_pend& SI446X_INT_CTL_PH_PSENT_EN)//packet sent
	{
		si446x_set_pksent( );
		//XPRINTF((0, "PH = %02x\r\n", ubph_en));
		count_tx++;
	}
	//packet rx finish
	if (si446x_ph_pend & SI446X_INT_CTL_PH_PRX_EN)
	{
		ph_prx_handler( );
		count_rx++;
	}
	//packet crc error
	if (si446x_ph_pend & SI446X_INT_CTL_PH_CRCE_EN)
	{
		si446x_change_dev_current_state(SI446X_SLEEP);
		si446xStartRX( );
	}

	//syde
	if (si446x_modem_pend & SI446X_INT_CTL_MODEM_SYDE_EN)
	{
		modem_syde_handler();
	}

	//cmd error
	if (si446x_chip_pend & SI446X_INT_CTL_CHIP_CMERR_EN)
	{
		chip_cmderror_handler( );
		XPRINTF((0, "cmd_error\n"));
	}
	//Set_Int_Event( SPI_INT );
}


/*--------------------------------------------------------------------------*/
const struct radio_driver si446x_radio_driver = {
  si446x_radio_rf_init,
  si446x_radio_prepare,
  si446x_radio_transmit,
  si446x_radio_send,
  si446x_radio_read,
  si446x_radio_channel_clear,
  si446x_radio_receiving_packet,
  si446x_radio_pending_packet,
  si446x_radio_on,
  si446x_radio_off,
};



