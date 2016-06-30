#ifndef _BOARDIODEF_H
#define _BOARDIODEF_H

#include "stm8l15x_gpio.h"


#define SWIM_PIN	GPIO_Pin_0
#define SWIM_PORT	GPIOA

#define NSRT_PIN	GPIO_Pin_1
#define NSRT_PORT	GPIOA

//usart port
#define USART1_TX_PIN	GPIO_Pin_2
#define USART1_TX_PORT	GPIOA
#define USART1_RX_PIN	GPIO_Pin_3
#define USART1_RX_PORT	GPIOA

//led port
#define LED_PIN			GPIO_Pin_1
#define LED_PORT		GPIOD

//iic port
#define IIC_NSDN_PIN	GPIO_Pin_0
#define IIC_NSDN_PORT	GPIOB

#define IIC_SDA_PIN		GPIO_Pin_1
#define IIC_SDA_PORT	GPIOB

#define IIC_SCL_PIN		GPIO_Pin_2
#define IIC_SCL_PORT	GPIOB

//rf port
#define RF_SDN_PIN		GPIO_Pin_3
#define RF_SDN_PORT		GPIOB
#define RF_NSEL_PIN		GPIO_Pin_4
#define RF_NSEL_PORT	GPIOB
#define RF_SCLK_PIN		GPIO_Pin_5
#define RF_SCLK_PORT	GPIOB
#define RF_SDI_PIN		GPIO_Pin_6
#define RF_SDI_PORT		GPIOB
#define RF_SDO_PIN		GPIO_Pin_7
#define RF_SDO_PORT		GPIOB
#define RF_NIRQ_PIN		GPIO_Pin_4
#define RF_NIRQ_PORT	GPIOD



#define LED_SET(a)		a?(LED_PORT->ODR |= LED_PIN):(LED_PORT->ODR &= (uint8_t)(~LED_PIN))

#endif
