#include "stm8l15x.h"
#include "boardiodef.h"
#include "board_init.h"

void boardClkInit(void)
{
	//CLK_DeInit( );
	//CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
	//CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
	//CLK_ClockSecuritySystemEnable( );
	//CLK_HSICmd(ENABLE);

	CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
	//init LED PIN
	GPIO_Init(LED_PORT, LED_PIN, GPIO_Mode_Out_OD_Low_Slow);
}




