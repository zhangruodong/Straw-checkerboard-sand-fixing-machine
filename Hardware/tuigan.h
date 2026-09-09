#ifndef __TUIGAN_H
#define __TUIGAN_H

#include "stm32f10x.h"                  // Device header

/* 推杆状态：双继电器互锁 */
typedef enum {
    ROD_STOP = 0,     // 停止（两继电器都断开）
    ROD_EXTEND = 1,   // 伸出
    ROD_RETRACT = 2   // 收回
} RodState;

/* 限位开关引脚（GPIOB 输入上拉，压到 = 低电平） */
#define LIMIT_FRONT   GPIO_Pin_10   // 前限位 PB10
#define LIMIT_BACK    GPIO_Pin_11   // 后限位 PB11
#define LIMIT_UP      GPIO_Pin_12   // 上升到位 PB12
#define LIMIT_DOWN    GPIO_Pin_13   // 下降到位 PB13

void Rod_Init(void);               // 推杆继电器 + 限位开关初始化
void Rod1_Set(RodState s);         // 推杆1（开沟犁）
void Rod2_Set(RodState s);         // 推杆2（压草）
void Rod3_Set(RodState s);         // 推杆3（覆沙）
void Rod4_Set(RodState s);         // 推杆4（备用）
uint8_t Limit_Hit(uint16_t pin);   // 限位是否压到（1=压到）

#endif
