#ifndef __TB6612_H
#define __TB6612_H
void TB6612_Init(void);
void Motor_SetLeftPWM(int16_t pwm);
void Motor_SetRightPWM(int16_t pwm);
void Motor_SetPWM(int16_t left,int16_t right);

#endif
