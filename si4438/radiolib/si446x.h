#ifndef __SI446X_H_

#define __SI446X_H_
#include "si446x_defs_u.h"
#include "si446x_cmd.h"

#define  PACKET_LENGTH      64 //0-64, if = 0: variable mode, else: fixed mode

extern union si446x_cmd_reply_union Si446xCmd;



#define HWLH_TEST_CH				2



//INT_CTL_PH_ENABLE
#define SI446X_INT_CTL_PH_FLTOK_EN				0x80 //#define RF_INT_CTL_PH_FILTER_MATCH_EN				0x80
#define SI446X_INT_CTL_PH_FLMISS_EN				0x40//#define RF_INT_CTL_PH_FILTER_MISS_EN				0x40
#define SI446X_INT_CTL_PH_PSENT_EN				0x20//#define RF_INT_CTL_PH_PACKET_SENT_EN				0x20
#define SI446X_INT_CTL_PH_PRX_EN				0x10//#define RF_INT_CTL_PH_PACKET_RX_EN					0x10
#define SI446X_INT_CTL_PH_CRCE_EN				0x08//#define RF_INT_CTL_PH_CRC_ERROR_EN					0x08
#define SI446X_INT_CTL_PH_ACRC_EN				0x04//#define RF_INT_CTL_PH_ALT_CRC_ERROR_EN				0x04
#define SI446X_INT_CTL_PH_TFAE_EN				0x02//#define RF_INT_CTL_PH_TX_FIFO_ALMOST_EMPTY_EN		0x02
#define SI446X_INT_CTL_PH_RFAF_EN				0x01//#define RF_INT_CTL_PH_RX_FIFO_ALMOST_FULL_EN		0x01

//INT_CTL_MODEM_ENABLE
//#define SI446X_INT_CTL_MODEM_RSSI_LATCH_EN				0x80//#define RF_INT_CTL_MODEM_RSSI_LATCH_EN				0x80
#define SI446X_INT_CTL_MODEM_PODE_EN			0x40//#define RF_INT_CTL_MODEM_POSTAMBLE_DETECT_EN		0x40
#define SI446X_INT_CTL_MODEM_INSY_EN			0x20//#define RF_INT_CTL_MODEM_INVALID_SYNC_EN			0x20
#define SI446X_INT_CTL_MODEM_RSSIJ_EN			0x10//#define RF_INT_CTL_MODEM_RSSI_JUMP_EN				0x10
#define SI446X_INT_CTL_MODEM_RSSI_EN			0x08//#define RF_INT_CTL_MODEM_RSSI_EN					0x08
#define SI446X_INT_CTL_MODEM_INPR_EN			0x04//#define RF_INT_CTL_MODEM_INVALID_PREAMBLE_EN		0x04
#define SI446X_INT_CTL_MODEM_PRDE_EN			0x02//#define RF_INT_CTL_MODEM_PREAMBLE_DETECT_EN			0x02
#define SI446X_INT_CTL_MODEM_SYDE_EN			0x01//#define RF_INT_CTL_MODEM_SYNC_DETECT_EN				0x01

//INT_CTL_CHIP_ENABLE
#define SI446X_INT_CTL_CHIP_CAL_EN				0x40
#define SI446X_INT_CTL_CHIP_FUOE_EN				0x20//#define INT_CTL_CHIP_FIFO_UNDERFLOW_OVERFLOW_ERROR_EN	0x20
#define SI446X_INT_CTL_CHIP_STCH_EN				0x10//#define INT_CTL_CHIP_STATE_CHANGE_EN					0x10
#define SI446X_INT_CTL_CHIP_CMERR_EN			0x08//#define INT_CTL_CHIP_CMD_ERROR_EN						0x08
#define SI446X_INT_CTL_CHIP_CHRE_EN				0x04//#define INT_CTL_CHIP_CHIP_READY_EN						0x04
#define SI446X_INT_CTL_CHIP_LBY_EN				0x02//#define INT_CTL_CHIP_LOW_BATT_EN						0x02
#define SI446X_INT_CTL_CHIP_WUT_EN				0x01//#define INT_CTL_CHIP_WUT_EN								0x01

#define SI446X_FIFO_SIZE			64
#define SI446X_FIFO_TFAE_TH			10
#define SI446X_FIFO_RFAF_TH			54
#define SI446X_FIFO_TFAF_TH			48


/// si4438 state
typedef enum _SI446X_STATE {
  SI446X_OFF = 0,
  SI446X_RX  = 0x01,
  SI446X_TX  = 0x02,
  
  SI446X_IDLE = 0x10,		// searching for preamble + sync word
  SI446X_RX_RECEIVING = 0x20,		// receiving bytes
  SI446X_RX_PRE = 0x40,		// 
  SI446X_OP_STATE = 0x73,
  
  SI446X_TURN_OFF = 0x80,
}SI446X_STATE;




//#define WIRE_LESS_40KBPS
#define WIRE_LESS_9_6KBPS



#ifdef WIRE_LESS_40KBPS
#define  SI4432_WAIT_INT_TIME		6000
#define  SI4432_WAIT_PKSENT_TIME	2000
#define  SI4432_WAIT_TXFFAEM_TIME	2000
#define  SEND_PACKET_TIMEOUT		20//20ms(240000)
#define  SEND_128PACKET_TIME		30//30ms
#define  SEND_WAIT_TXFIFO_EMPTY		20//20ms(120000)
#endif

#ifdef WIRE_LESS_9_6KBPS
#define  SI4432_WAIT_INT_TIME		6000
#define  SI4432_WAIT_PKSENT_TIME	2000
#define  SI4432_WAIT_TXFFAEM_TIME	2000
#define  SEND_PACKET_TIMEOUT		700//70ms(720000)
#define  SEND_128PACKET_TIME		140//140ms
#define  SEND_WAIT_TXFIFO_EMPTY		700//70ms(480000)
#endif


typedef enum _SI446X_TX_POWER_DBM
{
	POWER_20_0_DBM	= 0x7f,
	POWER_18_4_DBM	= 0x6f,
	POWER_16_8_DBM	= 0x5f,
	POWER_15_2_DBM	= 0x4f,
	POWER_13_6_DBM	= 0x3f,
	POWER_12_0_DBM	= 0x2f,
	POWER_10_4_DBM	= 0x16,
	POWER_08_8_DBM	= 0x00
}SI446X_TX_POWER_DBM;



typedef enum _SI446X_DEV_STATE
{
	SI446X_NOCHANGE		= 0x00,
	SI446X_SLEEP 		= 0x01,//not applicable
	SI446X_SPI_ACTIVE 	= 0x02,
	SI446X_READY 		= 0x03,
	SI446X_READY2 		= 0x04,
	SI446X_TX_TUNE 		= 0x05,
	SI446X_RX_TUNE 		= 0x06,
	SI446X_TX_STATE		= 0x07,
	SI446X_RX_STATE 	= 0x08
}SI446X_DEV_STATE;


typedef enum _SI446X_FRR_MODE
{
	FRR_MODE_DISABLED 		= 0x00,
	FRR_MODE_INT_STATUS		= 0x01,
	FRR_MODE_INT_PEND		= 0x02,
	FRR_MODE_INT_PH_STATUS	= 0x03,
	FRR_MODE_INT_PH_PEND	= 0x04,
	FRR_MODE_INT_MODEM_STATUS = 0x05,
	FRR_MODE_INT_MODEM_PEND	= 0x06,
	FRR_MODE_INT_CHIP_STATUS	= 0x07,
	FRR_MODE_INT_CHIP_PEND		= 0x08,
	FRR_MODE_CURRENT_STATE	= 0x09,
	FRR_MODE_LATCHED_RSSI	= 0x0a
}SI446X_FRR_MODE;


typedef enum _SI446X_TX_CONDITION
{
	NOCHANGE_AFTER_TX		=  0x00,
	RX_AFTER_TX				=  0x80,
	SLEEP_ARTER_TX			=  0x10
}SI446X_TX_CONDITION;


typedef enum _SI446X_RX_CONDITION
{
	RX_IMMEDIATE	=  0x00,
	RX_WUT			=  0x01
}SI446X_RX_CONDITION;

typedef enum _SI446X_RF_CHANNEL
{
	RF_CH_00	= 0,
	RF_CH_01	= 1,
	RF_CH_02	= 2,
	RF_CH_03	= 3,
	RF_CH_04	= 4,
	RF_CH_05	= 5,
	RF_CH_06	= 6,
	RF_CH_07	= 7,
	RF_CH_08	= 8,
	RF_CH_09	= 9,
	RF_CH_10	= 10,
	RF_CH_11	= 11,
	RF_CH_12	= 12,
	RF_CH_13	= 13,
	RF_CH_14	= 14,
	RF_CH_15	= 15,
	RF_CH_16	= 16,
	RF_CH_17	= 17,
	RF_CH_18	= 18,
	RF_CH_19	= 19,
	RF_CH_20	= 20,
	RF_CH_21	= 21,
	RF_CH_22	= 22,
	RF_CH_23	= 23,
	RF_CH_24	= 24,
	RF_CH_25	= 25,
	RF_CH_26	= 26,
	RF_CH_27	= 27,
	RF_CH_28	= 28,
	RF_CH_29	= 29,
	RF_CH_30	= 30,
	RF_CH_31	= 31
}SI446X_RF_CH;

extern void si446x_set_pksent(void);

extern void si446xToRx(void);
extern void si446xStartRX(void);
extern void si446xReadMutiData(INT8U *pBuf);
extern void si446xRadioInit(void);
extern int si446xRadioSendFixLenData(const INT8U *pubdata);

extern void si446x_change_dev_current_state(SI446X_DEV_STATE e_state);
extern void si446x_set_tx_power(SI446X_TX_POWER_DBM e_txdbm);
extern INT8U si446x_get_property_1(SI446X_PROPERTY GROUP_NUM);

extern void si446x_get_modem_status(INT8U MODEM_CLR_PEND);
extern void si446x_radio_init(void);
extern void si446x_start_rx( SI446X_RF_CH channel, SI446X_RX_CONDITION condition, INT16U rx_len, SI446X_DEV_STATE n_state1, SI446X_DEV_STATE n_state2, SI446X_DEV_STATE n_state3);
extern int si446x_send_fix_len_data(const INT8U *pubdata, SI446X_RF_CH channel, SI446X_TX_CONDITION condition, INT16U tx_len);
extern void si446x_read_data(INT8U *pioBuf, INT8U ubLen);
extern void si446x_get_int_status(void);


#endif

