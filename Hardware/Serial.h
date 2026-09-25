#ifndef __SERIAL_H
#define __SERIAL_H
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendString(char *str);
void Serial_SendArray(uint8_t* Array , uint16_t length);
void Serial_SendNumber(uint32_t Number,uint8_t length);
void Serial_Printf(char *format, ...);
#endif
///*用vofa调pid*/		
//		if(RxFlag == 1)
//				{
//					const char *StrFlag = strtok(Str,":");
//					const char *StrNum = strtok(NULL,":");
//					if(StrFlag != NULL && StrNum != NULL)
//					{
//						float num = atof(StrNum);		
//						if(strcmp(StrFlag,"Kp") == 0)
//						{
//							Right_Speed_PID.Kp = num;
//						}
//						else if(strcmp(StrFlag,"Ki") == 0)
//						{
//							Right_Speed_PID.Ki = num;
//						}
//						else if(strcmp(StrFlag,"Kd") == 0)
//						{
//							Right_Speed_PID.Kd = num;
//						}
//						else if(strcmp(StrFlag,"Speed") == 0)
//						{
//							Right_Speed_PID.Tar = num;
//						}
//					}			
//					RxFlag = 0;
//			}
//				Serial_Printf("%f,%f,%f,%f\n",Right_Speed_PID.Act,Right_Speed_PID.Tar,Right_Speed_PID.Out,Right_Speed_PID.Tar);;
