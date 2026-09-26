#include "stm32f10x.h"                  // Device header
#include "Timer.h"
#include "OLED.h"
#include "Encoder.h"
#include "PID.h"
#include "Track.h"
#include "Key.h"
#include "SmartCar.h"
#include "Serial.h"

#define BASE_SPEED    60.0				//基础速度(速度环目标:编码器脉冲增量/20ms)
#define TURN_SPEED    300				//直角弯原地转向PWM占空比
#define LOST_TIMEOUT  75				//单次丢线超时,75×20ms≈1.5s
#define LOSS_DEBOUNCE 6			//丢线去抖:连续5×20ms=100ms才判定为真弯道
#define TARGET_TURNS  3					//走完3个弯,第4个弯停车

/*编码器换算常数*/
#define WHEEL_DIAMETER_MM      65.0f
#define WHEEL_CIRCUMFERENCE_MM (3.14159265f * WHEEL_DIAMETER_MM)
#define ENCODER_CPR       13
#define GEAR_RATIO        28
#define PULSES_PER_REV    (ENCODER_CPR * 4 * GEAR_RATIO)
#define MM_PER_PULSE      (WHEEL_CIRCUMFERENCE_MM / PULSES_PER_REV)
#define TRACK_WIDTH_MM    119.2f
#define MM_PER_DEGREE     ((3.14159265f/180.0f) * (TRACK_WIDTH_MM/2.0f))

/*定速PID参数*/
PID_t Left_Speed_PID = {
	.Kp = 3.50f,
	.Ki = 0.75f,
	.Kd = 0.20f,
	.OutMax = 1000.0f,
	.OutMin = -1000.0f,
	.ErrorIntMax = 1000.0f
	};

PID_t Right_Speed_PID = {
	.Kp = 3.50f,
	.Ki = 0.75f,
	.Kd = 0.20f,
	.OutMax = 1000.0f,
	.OutMin = -1000.0f,
	.ErrorIntMax = 1000.0f
	};

float Left_Speed,Right_Speed;
int16_t LeftPulse,RightPulse;
/*循迹PID参数*/
PID_t Line = {
	.Tar = 0,
	.Kp = 4,
	.Ki = 0,
	.Kd = 18,
	.OutMax = 50,
	.OutMin = -50,
	.ErrorIntMax = 75
	};
float DifSpeed;	
	
 /*判断小车状态*/	
volatile uint8_t Track_Count,Stop_Count;
volatile uint16_t LostCount = 0;		//本次连续丢线已持续的20ms周期数(看到线即清零)
volatile uint8_t Turn_Done = 0;			//本次丢线是否已计过弯,保证一个弯只计一次
volatile uint8_t Finished = 0;			//已走完目标弯数,永久停车
volatile float TotalDistance_mm = 0;			//小车累计走过的距离(单位mm),左右轮编码器增量的平均值累加
extern volatile int8_t Location[8];				//Track.c里的8路传感器状态,OLED显示用

int main(void)
{
	OLED_Init();
	SmartCar_Init();
	Track_Init();
	Encoder_Init();
	Timer_Init();	
	Serial_Init();
	
	while(1)
	{

OLED_ShowSignedNum(3,1,LostCount,5);	/*显示本次连续丢线周期数,便于调去抖阈值*/
OLED_ShowSignedNum(4,1,Stop_Count,5);
OLED_ShowSignedNum(1,1,Left_Speed_PID.Act,3);
OLED_ShowSignedNum(2,1,Right_Speed_PID.Act,3);
OLED_ShowSignedNum(3,7,Line.Out,3);
OLED_ShowSignedNum(4,7,Line.Act,3);		
Serial_Printf("%f,%f,%f,%f\n",Line.Act,Line.Out,Line.Error0,Line.ErrorInt);		
	}
}
/*TIM1更新中断,每20ms触发一次,巡线核心逻辑在这里执行*/
void TIM1_UP_IRQHandler(void)
{	
	static int count;
	static float step_mm;	//小车前进的距离(mm)
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		count++;
		if(count >=20){
			count = 0;					
			/*每20ms只读一次编码器,速度PID和测距共用这份增量*/
			LeftPulse = Encoder_GetLeft();
			RightPulse = Encoder_GetRight();
			
			Line.Act = Track_GetState();

			if(Finished)							/*已走完目标弯数,永久停车*/
			{
				Move_Stop();
			}
			else if(Track_Count > 0)				/*正常巡线*/
			{
				LostCount = 0;						/*重新看到线:丢线计数清零*/
				Turn_Done = 0;						/*解除本次丢线的已计弯标记*/

				PID_Update(&Line);
				DifSpeed = Line.Out;
			
				Left_Speed = BASE_SPEED - DifSpeed;
				Right_Speed = BASE_SPEED + DifSpeed;
				
				/*PID定速调控*/
				Left_Speed_PID.Act = LeftPulse;
				Right_Speed_PID.Act = RightPulse;
			
				Left_Speed_PID.Tar = Left_Speed;
				Right_Speed_PID.Tar = Right_Speed;
			
				PID_Update(&Left_Speed_PID);
				PID_Update(&Right_Speed_PID);
				Move_SetSpeed(Left_Speed_PID.Out,Right_Speed_PID.Out);
			}
			
			else							/*8路全丢线*/
			{
				Line.ErrorInt = 0; /*丢线期间清零*/
				LostCount++;

				if(LostCount < LOSS_DEBOUNCE)		/*去抖:刚丢线的60ms内维持原速冲过去*/
				{
					/*不发新指令,电机保持上一周期PWM,避免瞬时丢线被误判成直角弯*/
				}
				else if(LostCount < LOST_TIMEOUT)	/*确认是直角弯,开始原地转向*/
				{
					if(!Turn_Done)					/*边沿触发:一次丢线只计一个弯*/
					{
						Turn_Done = 1;
						Stop_Count++;
						if(Stop_Count > TARGET_TURNS)	/*已走完3个弯,这是第4个*/
						{
							Finished = 1;
						}
					}

					if(Finished)
					{
						Move_Stop();				/*第4个弯:停车*/
					}
					else if(Line.Error0 < 0)		/*线最后出现在右侧*/
					{
						Move_SetSpeed(TURN_SPEED,-TURN_SPEED);	/*原地右转*/
					}
					else							/*线最后出现在左侧*/
					{
						Move_SetSpeed(-TURN_SPEED,TURN_SPEED);	/*原地左转*/
					}
				}
				else
				{
					Move_Stop();  /*单次丢线超时,防止无限打转*/
				}
			}

//			/*编码器测距:使用本轮开头读到的脉冲增量*/
//			/*左右轮脉冲增量取平均,乘以MM_PER_PULSE,得到本轮前进的距离(mm)*/
//			step_mm = (LeftPulse + RightPulse) / 2.0f * MM_PER_PULSE;
//			/*累加进总里程*/
//			TotalDistance_mm += step_mm;
	}
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
