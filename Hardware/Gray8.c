#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void Gray8_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					//OUT数字量输出,上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;					//AD0地址位,推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;		//AD1 AD2地址位,推挽输出
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

uint8_t Gray8_GetState(uint8_t channel)
{
	/*把channel的bit0/bit1/bit2分别写到AD0(PA5)/AD1(PB0)/AD2(PB1)*/
	GPIO_WriteBit(GPIOA,GPIO_Pin_5,(BitAction)(channel & 0x01));
	GPIO_WriteBit(GPIOB,GPIO_Pin_0,(BitAction)((channel >> 1) & 0x01));
	GPIO_WriteBit(GPIOB,GPIO_Pin_1,(BitAction)((channel >> 2) & 0x01));
	Delay_us(20);												//等电路稳定
	return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);				//读PA4电平返回
}
