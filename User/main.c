#include "stm32f10x.h"
#include "system.h"

extern volatile uint32_t timer_counter;

int main(void){
    Hardware_Init();
    System_Init();

    OLED_ShowString(1, 1, "GuShaJi");

    while(1){
        System_StateMachine();
        IWDG_ReloadCounter();                 // feed watchdog

        // refresh OLED every 100ms (software I2C, avoid full-screen refresh each loop)
        static uint32_t last_oled = 0;
        if (timer_counter - last_oled >= 100) {
            last_oled = timer_counter;
            OLED_ShowNum(2, 1, timer_counter, 8);
            OLED_ShowChar(3, 1, (char)sys_ctrl.state);   // show current state char
        }
    }
}
