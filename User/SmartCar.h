#ifndef __SMARTCAR_H
#define __SMARTCAR_H
void SmartCar_Init(void);
void Move_Forward(int8_t Speed);
void Move_Backward(int8_t Speed);
void Move_Stop(void);
void Move_TurnLeft(int8_t Speed);
void Move_TurnRight(int8_t Speed);
void Move_SetSpeed(int16_t left, int16_t right);
#endif
