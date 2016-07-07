#include "stm8l15x.h"
#include "basictype.h"
#include "chain.h"
#include "events.h"

NODE nIRQ_node;
NODE tx_node;
NODE tx_check_node;
NODE update_time_node;
NODE rx_timeout_node;
NODE data_req_restore;
NODE rssi_node;
NODE data_req;
NODE heart_beat;
NODE i2c;
//NODE preamble_node;
// for app
NODE app_pkt_pro;
NODE check_nirq;
NODE network_init;
NODE wait_father_rep;
NODE network_search;

#ifdef DEBUG_FLAG
NODE debug_node;
#endif


volatile EVENT events = 0;
volatile u16 interrupt_events = 0;

u8 set_event( EVENT event_flag )
{
	//  disableInterrupts();    // Hold off interrupts
	events |= event_flag;  // Stuff the event bit(s)
	//  enableInterrupts();     // Release interrupts
	return ( SUCCESS );
}

void reset_event(void)
{
	events = 0;
}

void poll_event(void)
{
	while(events)
	{
		if(events & E_nIRQ)
		{
			events &= ~(E_nIRQ);
		}
		#ifdef I2C_COM
		else if(events & E_I2C_INT)
		{
			events &= ~(E_I2C_INT);
		}
		#endif
		else if(events & E_UPDATE_TIME)
		{
			events &= ~(E_UPDATE_TIME);
		}
		else if(events & E_rx_timeout)
		{
			events &= ~(E_rx_timeout);
		}
		else if(events & E_tx_check)
		{
			events &= ~(E_tx_check);
		}
		else if(events & E_tx)
		{
			events &= ~(E_tx);
		}
		else if(events & E_rssi)
		{
			events &= ~(E_rssi);
		}
		else if(events & E_data_req)
		{
			events &= ~(E_data_req);
		}
		else if(events & E_HeartBeat)
		{
			events &= ~(E_HeartBeat);
		}
		// for app
		else if(events & E_app_pkt_process)
		{
			events &= ~(E_app_pkt_process);
		}
		else if(events & E_DATA_REQ_RESTORE)
		{
			events &= ~(E_DATA_REQ_RESTORE);
		}
		#ifdef I2C_COM
		else if(events & E_I2C)
		{
			events &= ~E_I2C;
		}
		#endif
		else if(events & E_NETWORK_INIT)
		{
			events &= ~E_NETWORK_INIT;
		}
		else if(events & E_NETWORK_SEARCH)
		{
			events &= ~(E_NETWORK_SEARCH);
		}
		else
		{
			events = 0;
		}
	}
}

void Set_Int_Event( u16 flag )
{ 
  interrupt_events |= flag;
}

u16 check_Int( void )
{ 
	return interrupt_events;
}


void Int_to_events( void )
{
	//disableInterrupts();
    if( interrupt_events & SPI_INT )
    {
		//set_event(E_nIRQ);
		interrupt_events &= ~SPI_INT;
    }
	if( interrupt_events & RTC_INT )
    {
		set_event(E_UPDATE_TIME);
		interrupt_events &= ~RTC_INT;
    }
	#ifdef I2C_COM
    if( interrupt_events & I2C_INT )
    {
		set_event(E_I2C_INT);
		interrupt_events &= ~I2C_INT;
    }
	#endif
	//enableInterrupts();	
}

