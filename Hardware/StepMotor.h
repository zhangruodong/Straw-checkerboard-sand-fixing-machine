#ifndef __STEPMOTOR_H
#define __STEPMOTOR_H
#include "stm32f10x.h"                  // Device header
//#include <stdio.h>

void StepMotor1_Init(void);
void StepMotor2_Init(void);
void StepMotor3_Init(void);
void StepMotor4_Init(void);

void StepMotor1_SetDirection(uint8_t Direction);
void StepMotor2_SetDirection(uint8_t Direction);
void StepMotor3_SetDirection(uint8_t Direction);
void StepMotor4_SetDirection(uint8_t Direction);

void StepMotor1_SetPulse(uint16_t Pulse);
void StepMotor2_SetPulse(uint16_t Pulse);
void StepMotor3_SetPulse(uint16_t Pulse);
void StepMotor4_SetPulse(uint16_t Pulse);


void StepMotor_UpdateNonBlocking(void);
void StepMotor_UpdateGroup12(void);
void StepMotor_UpdateGroup34(void);
void StopAllMotors(void);
void Track_Go(int16_t steps);
void Track_Turn(int8_t dir, uint16_t steps);
#endif
