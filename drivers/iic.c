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
void iicBusInit(void)//���߳�ʼ�� �����߶�����һ�ͷ�����  ���������ź�ǰ��Ҫ�ȳ�ʼ�����ߡ������м�⵽���߿��вſ�ʼ���������ź�  
{  
    SCL_OUT();     //SDA��SCL��ʼ������Ϊ���
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
U8 iicBusStart(void)  //��ʼ�ź� SCL�ڸߵ�ƽ�ڼ䣬SDAһ���½������ʾ�����ź�  
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
void iicBusStop(void)   //ֹͣ SCL�ڸߵ�ƽ�ڼ䣬SDAһ�����������ʾֹͣ�ź�  
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
U8 iicWaitACK(void)  //Ӧ�� SCL�ڸߵ�ƽ�ڼ䣬SDA�����豸��Ϊ�͵�ƽ��ʾӦ��  
{  
	U8 i=0;  
	SDA_HIGH();
	//����ȴ�250��CPUʱ������
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
			return 1;	   //û�յ�Ӧ���ź�
		}
	}
	delay_xus(SCL_HALF_PERIOD/2);
	SCL_LOW();
	SDA_OUT();
	//    SDA_LOW;
	SDA_HIGH();
	return 0;  		  //�յ�Ӧ���ź�
} 


//***************************************************************************
void iicAck(void)  		 //Ӧ���ź�
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
void iicNoAck(void)		 //��Ӧ���ź�
{
	SCL_LOW();
	SDA_HIGH();
	delay_xus(5);
	SCL_HIGH();
	delay_xus(5);
	SCL_LOW();
}

//***************************************************************************  
void iicWriteByte(U8 data) //дһ���ֽ�  
{  
	U8 i=0,temp=0;  
	temp=data;  
	for(i=0;i<8;i++)  
	{    
		SCL_LOW();//����SCL����Ϊֻ����ʱ���ź�Ϊ�͵�ƽ�ڼ䰴�������ϵĸߵ͵�ƽ״̬������仯�����ڴ�ʱ����һ��ѭ����scl=1һ���γ�һ��������  
		if(temp&0x80)
		{
		    SDA_HIGH();
		}
		else
		{
		    SDA_LOW();
		}  
		delay_xus(SCL_HALF_PERIOD);  
		SCL_HIGH();//����SCL����ʱSDA�ϵ������ȶ�
		temp=temp<<1;
		delay_xus(SCL_HALF_PERIOD);
	}  
	//    DELAY_X_US(SCL_HALF_PERIOD);
	SCL_LOW();//����SCL��Ϊ�´����ݴ�������׼��  
	SDA_HIGH();//�ͷ�SDA���ߣ��������ɴ��豸���ƣ�������豸���������ݺ���SCLΪ��ʱ������SDA��ΪӦ���ź�
	delay_xus(SCL_HALF_PERIOD);  
} 


//***************************************************************************  
U8 iicReadByte(void)//��һ���ֽ�  
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
	if(ERROR == iicBusStart())            //��ʼ�ź�
	{
		return state;
	}
	delay_xus(5);
	for(i=0;i<4;i++)		  //����4��CMD
	{
		iicWriteByte(in_buff[i]);
		ACK_Flag = iicWaitACK();
		if(ACK_Flag)		  //��Ӧ�����������Ӧ�����˳��Ӻ���
		{
			return state;
		}
	}
	SCL_OUT();
	for(i=0;i<4;i++)		  //�Ӵӻ���ȡ4���ֽڵ����ݣ�����data_buff��
	{
		data_buff[i]=iicReadByte();
		iicAck();				  //Ӧ���ź� 
	}
	iicBusStop( );					  //�����ź�
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



