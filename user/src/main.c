
/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "board_init.h"
#include "debugUart.h"
#include "xprintf.h"

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
	boardClkInit( );
	debugUartInit( );
	xdev_out(dbgSendChar);
	while (1)
	{
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
