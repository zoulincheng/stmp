#ifndef __EVENTS_H
#define __EVENTS_H


#define E_nIRQ                  0x0001 
#define E_tx                    0x0002   
#define E_UPDATE_TIME           0x0004   
#define E_UPLOAD_LOCK           0x0008
#define E_tx_check              0x0010
#define E_rx_timeout            0x0020
#define E_rssi                  0x0040
#define E_data_req              0x0080
#define E_HeartBeat             0x0100
#define E_I2C                   0x0200
#define E_app_pkt_process       0x0400
#define E_I2C_INT               0x0800
#define E_NETWORK_INIT          0x1000
#define E_WAIT_FATHER_REP		0x2000
#define E_NETWORK_SEARCH        0x4000
#define E_DATA_REQ_RESTORE      0x8000

#define SPI_INT                 0x0001
#define RTC_INT                 0x0002
#define I2C_INT                 0x0004

extern NODE data_req_restore;
extern NODE nIRQ_node;
extern NODE tx_node;
extern NODE tx_check_node;
extern NODE update_time_node;
extern NODE rx_timeout_node;
extern NODE rssi_node;
extern NODE data_req;
extern NODE heart_beat;
extern NODE i2c;
extern NODE app_pkt_pro;
extern NODE i2c_int;
extern NODE network_init;
extern NODE wait_father_rep;
extern NODE network_search;


#ifdef DEBUG_FLAG
extern NODE debug_node;
#endif

#ifdef DEBUG_FLAG_2
extern NODE debug_2_node;
#endif

extern volatile EVENT events;

u8 set_event( EVENT event_flag );
void reset_event(void);
void poll_event( void );
void Set_Int_Event( u16 flag );
u16 check_Int( void );
void Int_to_events( void );

#endif
