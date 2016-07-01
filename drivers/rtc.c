#include "stm8l15x.h"
#include "basictype.h"
#include "sysprintf.h"
#include "stm8l15x_it.h"
#include "stm8l15x_rtc.h"
#include "rtc.h"


void rtcInit(void)
{
	RTC_DeInit();
	CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);    //LSI=38kHz
	CLK_PeripheralClockConfig(CLK_Peripheral_RTC,ENABLE);
	RTC_WakeUpCmd(DISABLE);
	//RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);        //LSI/
	RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);          //LSI/16=421.05us
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	/*
	RTC_WakeUpCmd(DISABLE);
	RTC_SetWakeUpCounter(next_wakeup);
	RTC_WakeUpCmd(ENABLE);
	*/
}

void rtcOn(U16 uwWakeTime)
{
	RTC_WakeUpCmd(DISABLE);
	RTC_SetWakeUpCounter(uwWakeTime);
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	RTC_WakeUpCmd(ENABLE);
}

void rtc_off(void)
{
	RTC_ITConfig(RTC_IT_WUT, DISABLE);
	RTC_WakeUpCmd(DISABLE);
}


/**
  * @brief RTC / CSS_LSE Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(RTC_CSSLSE_IRQHandler, 4)
{
	/* In order to detect unexpected events during development,
	 it is recommended to set a breakpoint on the following instruction.
	*/
	RTC_ClearITPendingBit(RTC_IT_WUT);
	XPRINTF((0, "rtc interrupt\r\n"));
	rtcOn(MS2ST(500));
}


