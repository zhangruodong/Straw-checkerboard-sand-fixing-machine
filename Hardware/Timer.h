#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"                  // Device header

void TIM1_Init(void);

void TIM2_Init(void);

void TIM3_Init(void);

void TIM4_Init(void);

void TIM5_Init(void);

void GPIO1_Init(void);

void TIM5_UP_IRQHandler(void); 


#endif
