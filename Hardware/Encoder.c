#include "Encoder.h"

/*方向校准开关:实测发现顺时针转动读数不是增大而是减小时,把对应值从0改成1即可翻转,
  不需要重新接线。装车后先测试这个再决定要不要改。*/
#define LEFT_ENCODER_INVERT   0
#define RIGHT_ENCODER_INVERT  0

void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;	//PA0=右编码器A PA1=右编码器B
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;	//PA6=左编码器A PA7=左编码器B
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /*顺时针增大、逆时针减小这个方向约定,实际由编码器AB两相接线顺序和这里的
      TIM_EncoderMode配置共同决定,具体哪个方向对应正数需要装车后实测确认,
      测完不对就改上面LEFT_ENCODER_INVERT/RIGHT_ENCODER_INVERT这两个开关*/
    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 10;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    TIM_ICInit(TIM3, &TIM_ICInitStructure);

    TIM_SetCounter(TIM2, 0);
    TIM_SetCounter(TIM3, 0);

    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

int16_t Encoder_GetRight(void)
{
    int16_t count = (int16_t)TIM_GetCounter(TIM3);
    TIM_SetCounter(TIM3, 0);
#if LEFT_ENCODER_INVERT
    count = -count;
#endif
    return count;
}

int16_t Encoder_GetLeft(void)
{
    int16_t count = (int16_t)TIM_GetCounter(TIM2);
    TIM_SetCounter(TIM2, 0);
#if RIGHT_ENCODER_INVERT
    count = -count;
#endif
    return count;
}
