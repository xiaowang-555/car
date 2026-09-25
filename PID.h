#ifndef __PID_H
#define __PID_H
typedef struct {
	float Tar;
	float Act;
	float Out;
	
	float Kp;
	float Ki;
	float Kd;
	
	float Error0;
	float Error1;
	float ErrorInt;
	
	float OutMax;
	float OutMin;
	
	float ErrorIntMax;/*积分限幅*/
	} PID_t ; 

void PID_Update(PID_t * p);

#endif
