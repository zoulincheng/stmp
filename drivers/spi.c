#include "basictype.h"
#include "stm8l15x.h"

#include "sysprintf.h"
#include "boardiodef.h"


void spiInit(void)
{
	SPI_DeInit(SPI1);
	GPIO_Init(RF_SDN_PORT, RF_SDN_PIN| RF_NSEL_PIN|RF_SCLK_PIN|RF_SDI_PIN, GPIO_Mode_Out_PP_High_Fast);
	GPIO_Init(RF_SDO_PORT, RF_SDO_PIN, GPIO_Mode_In_PU_No_IT);
	/* Enable SPI clock */
	CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);
	/* Set the MOSI,MISO and SCK at high level */
	GPIO_ExternalPullUpConfig(GPIOB, RF_SCLK_PIN|RF_SDI_PIN|RF_SDO_PIN, ENABLE);

	/* SD_SPI Config */
	SPI_Init(SPI1, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2, SPI_Mode_Master,
	         SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_2Lines_FullDuplex,
	         SPI_NSS_Soft, 0x07);
	/* SD_SPI enable */
	SPI_Cmd(SPI1, ENABLE);	
}

void spiIoShutdown(void)
{
	SPI_Cmd(SPI1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, DISABLE);
	/* Set the MOSI,MISO and SCK at high level */
	//GPIO_ExternalPullUpConfig(GPIOB, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, DISABLE);
	//GPIO_Init(GPIOB, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 , GPIO_Mode_In_PU_No_IT);
	GPIO_ExternalPullUpConfig(RF_SDO_PORT, RF_SCLK_PIN|RF_SDI_PIN|RF_SDO_PIN, DISABLE);
	GPIO_Init(RF_SDO_PORT, RF_NSEL_PIN|RF_SCLK_PIN|RF_SDI_PIN|RF_SDO_PIN , GPIO_Mode_In_PU_No_IT);
}


u_char SPI_SendByteData(u_char ubData)  
{       
	/*!< Wait until the transmit buffer is empty */
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);

	/*!< Send the byte */
	SPI_SendData(SPI1, ubData);

	/*!< Wait to receive a byte*/
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET);

	/*!< Return the byte read from the SPI bus */
	return SPI_ReceiveData(SPI1);                       
}


/*!
 * This function is used to send data over SPI port (target: EzRadioPRO).no response expected.
 *
 *  @param[in] biDataInLength  The length of the data.
 *  @param[in] *pabiDataIn     Pointer to the first element of the data.
 *
 *  @return None
 */
void SpiWriteDataBurst(uint8_t biDataInLength, uint8_t *pabiDataIn)
{
	while (biDataInLength--) 
	{
		SPI_SendByteData(*pabiDataIn++);
	}
}


/*!
 * This function is used to read data from SPI port.(target: EzRadioPRO).
 *
 *  \param[in] biDataOutLength  The length of the data.
 *  \param[out] *paboDataOut    Pointer to the first element of the response.
 *
 *  \return None
 */
void SpiReadDataBurst(uint8_t biDataOutLength, uint8_t *paboDataOut)
{
	// send command and get response from the radio IC
	while (biDataOutLength--) 
	{
		*paboDataOut++ = SPI_SendByteData(0xff);
	}
}



