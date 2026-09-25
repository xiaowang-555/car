#include "stm32f10x.h"                  // Device header

void TB6612_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	//关闭JTAG,释放PA15/PB3/PB4作为普通GPIO

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;		//PB6=左电机PWM(TIM4_CH1),PB7=右电机PWM(TIM4_CH2)
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;	//PB3=BIN1 PB4=AIN1 PB5=AIN2
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;					//PA15=BIN2
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	TIM_InternalClockConfig(TIM4);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1 ;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 8 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0 ;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);

	TIM_OCInitTypeDef TIM_OC1InitStructure;
	TIM_OCStructInit(&TIM_OC1InitStructure);
	TIM_OC1InitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OC1InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1InitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC1InitStructure.TIM_Pulse = 0;
	TIM_OC1Init(TIM4,&TIM_OC1InitStructure);

	TIM_OCInitTypeDef TIM_OC2InitStructure;
	TIM_OCStructInit(&TIM_OC2InitStructure);
	TIM_OC2InitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OC2InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2InitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC2InitStructure.TIM_Pulse = 0;
	TIM_OC2Init(TIM4,&TIM_OC2InitStructure);

	TIM_Cmd(TIM4,ENABLE);
}

/*左轮速度,PB4=AIN1 PB5=AIN2,PB6=TIM4_CH1*/
void Motor_SetLeftPWM(int16_t pwm)
{
	if(pwm > 0)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_4);
		GPIO_ResetBits(GPIOB,GPIO_Pin_5);
	}
	else if(pwm < 0)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_5);
		GPIO_ResetBits(GPIOB,GPIO_Pin_4);
		pwm = -pwm;
	}
	else
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_4);
		GPIO_SetBits(GPIOB,GPIO_Pin_5);
	}
	TIM_SetCompare1(TIM4,pwm);
}

/*右轮速度,PB3=BIN1 PA15=BIN2,PB7=TIM4_CH2*/
void Motor_SetRightPWM(int16_t pwm)
{
	if(pwm > 0)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_3);
		GPIO_ResetBits(GPIOA,GPIO_Pin_15);
	}
	else if(pwm < 0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_15);
		GPIO_ResetBits(GPIOB,GPIO_Pin_3);
		pwm = -pwm;
	}
	else
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_3);
		GPIO_SetBits(GPIOA,GPIO_Pin_15);
	}
	TIM_SetCompare2(TIM4,pwm);
}

void Motor_SetPWM(int16_t left,int16_t right)
{
	Motor_SetLeftPWM(left);
	Motor_SetRightPWM(right);
}
