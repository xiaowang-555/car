#include "stm32f10x.h"                  // Device header
#include "Gray8.h"

const int8_t Weight[8] = {-7,-5,-3,-1,1,3,5,7};
volatile int8_t Location[8];
float Weight_Sum;
extern uint8_t Track_Count;

void Track_Init(void)
{
	Gray8_Init();
}

float Track_GetState(void)
{
	Track_Count = 0;
	Weight_Sum = 0;		//每次调用先清零,防止累加失真
	for(int i = 0;i < 8;i++)
			{
				Location[i] = Gray8_GetState(i);
			}
	for(int i = 0;i < 8;i++)
			{
				if(Location[i] == 1)
				{
					Weight_Sum += Weight[i];
					Track_Count++;
				}
			}
	if(Track_Count == 0)
	{
		return 0;
	}
	else{
	return Weight_Sum /Track_Count;	
	}		
}



//	switch (line)
//				{
//					case -7 :DifSpeed = 2.8f;
//						break;
//					case -6 :DifSpeed = 2.4f;
//						break;
//					case -5 :DifSpeed = 2.0f;
//						break;
//					case -4 :DifSpeed = 1.6f;
//						break;
//					case -3 :DifSpeed = 1.2f;
//						break;
//					case -2 :DifSpeed = 0.8f;
//						break;
//					case -1 :DifSpeed = 0.4f;
//						break;
//					case 0 :DifSpeed = 0;
//						break;
//					case 1 :DifSpeed = -0.4f;
//						break;
//					case 2 :DifSpeed = -0.8f;
//						break;
//					case 3 :DifSpeed = -1.2f;
//						break;
//					case 4 :DifSpeed = -1.6f;
//						break;
//					case 5 :DifSpeed = -2.0f;
//						break;
//					case 6 :DifSpeed = -2.4f;
//						break;
//					case 7 :DifSpeed = -2.8f;
//						break;
//					default : 
//						break;
//				}

