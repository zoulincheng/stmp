#include "stm8l15x.h"
#include "basictype.h"
#include "sysprintf.h"
#include "stm8l15x_it.h"
#include "boardiodef.h"


static void delay_xus(U16 us_time)
{
	U16 i;
	for(i = 0; i < us_time; i++)
	{
		//    delay_1us();
		asm("NOP");
		asm("NOP");
		asm("NOP");
		asm("NOP");
		asm("NOP");
		asm("NOP");
	}
}

static void delay_1ms(u16 time)
{
	unsigned int i,k;
	for(k = 0; k< time; k++)
	{
		for(i = 0; i<3190; i++)
		{
		}
	}		
}


void iicPinInit(void)
{
	//  GPIO_DeInit(GPIOB);
	/*
	GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_In_PU_IT);
	GPIO_Init(GPIOB, GPIO_Pin_1, GPIO_Mode_Out_PP_High_Fast);
	GPIO_Init(GPIOB, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast);
	GPIOB->CR2 &= (uint8_t)(~(GPIO_Pin_1));
	GPIOB->CR2 &= (uint8_t)(~(GPIO_Pin_2));
	/*/
	GPIO_Init(IIC_NSDN_PORT, IIC_NSDN_PIN, GPIO_Mode_In_PU_IT);
	GPIO_Init(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_Mode_Out_PP_High_Fast);
	GPIO_Init(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_Mode_Out_PP_High_Fast);
	GPIOB->CR2 &= (uint8_t)(~(IIC_SDA_PIN));
	GPIOB->CR2 &= (uint8_t)(~(IIC_SCL_PIN));	
	EXTI_SetPinSensitivity(EXTI_Pin_0, EXTI_Trigger_Falling);
}


//***************************************************************************  
void iicBusInit(void)//总线初始化 将总线都拉高一释放总线  发送启动信号前，要先初始化总线。即总有检测到总线空闲才开始发送启动信号  
{  
    SCL_OUT();     //SDA、SCL初始化设置为输出
    SDA_OUT();
    nSDN_IN();
    SDA_HIGH(); 
	SDA_HIGH(); 
    //DELAY_X_US(5);  
    SCL_HIGH();
	SCL_HIGH();
    //DELAY_X_US(5);  
}

//***************************************************************************  
U8 iicBusStart(void)  //开始信号 SCL在高电平期间，SDA一个下降沿则表示启动信号  
{     
    unsigned long time_cnt = SCL_WAIT_TIME;
	while(!(SDA_OUT_READ() && SCL_OUT_READ()))             
	{
		if(!(--time_cnt))
		{
			return ERROR;
		}
	}
    SDA_LOW();  
	SDA_LOW();
    //DELAY_X_US(5);
    return SUCCESS;
} 


//***************************************************************************  
void iicBusStop(void)   //停止 SCL在高电平期间，SDA一个上升沿则表示停止信号  
{  
	SDA_LOW();  
	SDA_LOW();
	//DELAY_X_US(5);  
	SCL_HIGH();
	SCL_HIGH();
	//DELAY_X_US(5);
	SDA_HIGH(); 
}


//***************************************************************************  
U8 iicWaitACK(void)  //应答 SCL在高电平期间，SDA被从设备拉为低电平表示应答  
{  
	U8 i=0;  
	SDA_HIGH();
	//至多等待250个CPU时钟周期
	SDA_IN();
	SCL_HIGH(); 
	delay_xus(SCL_HALF_PERIOD/2);
	while(SDA_IN_READ())
	{
		i++;
		if(i>250)
		{
			SDA_OUT();
			iicBusStop();
			return 1;	   //没收到应答信号
		}
	}
	delay_xus(SCL_HALF_PERIOD/2);
	SCL_LOW();
	SDA_OUT();
	//    SDA_LOW;
	SDA_HIGH();
	return 0;  		  //收到应答信号
} 


//***************************************************************************
void iicAck(void)  		 //应答信号
{
	SCL_LOW();
	SDA_OUT();
	SDA_HIGH();
	delay_xus(SCL_HALF_PERIOD);
	SDA_LOW();
	SCL_HIGH();
	delay_xus(SCL_HALF_PERIOD);
	SCL_LOW();
	SDA_HIGH();
 /*
	SCL_LOW;
	SDA_OUT;
	SDA_LOW;
	DELAY_X_US(SCL_HALF_PERIOD);
	SCL_HIGH;
	DELAY_X_US(SCL_HALF_PERIOD);
	SCL_LOW;
	SDA_HIGH;
	DELAY_X_US(4);
  */
}


//***************************************************************************
void iicNoAck(void)		 //非应答信号
{
	SCL_LOW();
	SDA_HIGH();
	delay_xus(5);
	SCL_HIGH();
	delay_xus(5);
	SCL_LOW();
}

//***************************************************************************  
void iicWriteByte(U8 data) //写一个字节  
{  
	U8 i=0,temp=0;  
	temp=data;  
	for(i=0;i<8;i++)  
	{    
		SCL_LOW();//拉低SCL，因为只有在时钟信号为低电平期间按数据线上的高低电平状态才允许变化；并在此时和上一个循环的scl=1一起形成一个上升沿  
		if(temp&0x80)
		{
		    SDA_HIGH();
		}
		else
		{
		    SDA_LOW();
		}  
		delay_xus(SCL_HALF_PERIOD);  
		SCL_HIGH();//拉高SCL，此时SDA上的数据稳定
		temp=temp<<1;
		delay_xus(SCL_HALF_PERIOD);
	}  
	//    DELAY_X_US(SCL_HALF_PERIOD);
	SCL_LOW();//拉低SCL，为下次数据传输做好准备  
	SDA_HIGH();//释放SDA总线，接下来由从设备控制，比如从设备接收完数据后，在SCL为高时，拉低SDA作为应答信号
	delay_xus(SCL_HALF_PERIOD);  
} 


//***************************************************************************  
U8 iicReadByte(void)//读一个字节  
{  
	U8 i=0,temp=0;  
	unsigned long time_cnt = SCL_WAIT_TIME;
	//    SCL_LOW;
	//    delay_us(100);
	//    SDA_HIGH;  
	//    delay();

	for(i=0;i<8;i++)  
	{  
		delay_xus(SCL_HALF_PERIOD/2);
		SDA_IN();
		delay_xus(SCL_HALF_PERIOD/2);
		SCL_HIGH();
		SCL_IN();
		while((--time_cnt))
		{
			if( SCL_IN_READ() )
			break;
		}
		if( SCL_IN_READ() )
		{
			delay_xus(SCL_HALF_PERIOD/2);
			temp<<=1; 
			if(SDA_IN_READ())
			{
				temp|=0x01;
			}
			delay_xus(SCL_HALF_PERIOD/2);
			SCL_OUT();
			SCL_LOW();
		}
	} 
	return temp;  
} 

//***************************************************************************  
U8 Data_Protocol(U8 *in_buff, U8 *data_buff)
{
	U8 state = ERROR;
	unsigned long time_cnt = SCL_WAIT_TIME;
	U8 i=0;
	static U8 ACK_Flag=0;

	delay_1ms(10);
	if(ERROR == iicBusStart())            //开始信号
	{
		return state;
	}
	delay_xus(5);
	for(i=0;i<4;i++)		  //发送4个CMD
	{
		iicWriteByte(in_buff[i]);
		ACK_Flag = iicWaitACK();
		if(ACK_Flag)		  //有应答，则继续；无应答，则退出子函数
		{
			return state;
		}
	}
	SCL_OUT();
	for(i=0;i<4;i++)		  //从从机读取4个字节的数据，存入data_buff中
	{
		data_buff[i]=iicReadByte();
		iicAck();				  //应答信号 
	}
	iicBusStop( );					  //结束信号
	state = SUCCESS;
	return state;
}



/**
  * @brief External IT PIN0 Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTI0_IRQHandler, 8)
{
	/* In order to detect unexpected events during development,
	it is recommended to set a breakpoint on the following instruction.
	*/
	EXTI_ClearITPendingBit(EXTI_IT_Pin0);
	//Set_Int_Event(I2C_INT);
}



