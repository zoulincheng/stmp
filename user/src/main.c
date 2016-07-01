
/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "board_init.h"
#include "debugUart.h"
#include "sysprintf.h"
#include "boardiodef.h"
#include "basictype.h"
#include "rtc.h"

/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
void delay(uint16_t nCount)
{
	/* Decrement nCount value */
	while (nCount != 0)
	{
		nCount--;
	}
}


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
	boardClkInit( );
	debugUartInit( );
	si446xRadioInit( );
	rtcInit( );
	rtcOn(MS2ST(500));
	enableInterrupts( );
	while (1)
	{
		delay(0xffff);
		delay(0xffff);
		delay(0xffff);
		delay(0xffff);
		LED_SET(1);
		//XPRINTF((0, "%ld\r\n", 12345678));
		//SI446X_PART_INFO(NULL);
		delay(0xffff);
		delay(0xffff);
		delay(0xffff);
		delay(0xffff);
		LED_SET(0);
		//XPRINTF((0, "sys2\r\n"));
	}
}




#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
	while (1)
	{
	}
}
#endif


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
