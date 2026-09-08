
#include "stm32f10x.h"
#include "system.h"
extern  uint8_t cmd;
extern volatile uint32_t timer_counter;
int main(void){
		Hardware_Init();
		System_Init();
    TIM5_Init();
		OLED_ShowString(1, 1, "HelloWorld!");
		OLED_ShowNum(2,1,timer_counter,8);
	 while(1){	
   System_StateMachine();

	 }
 }
