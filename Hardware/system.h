#ifndef __SYSTEM_H
#define __SYSTEM_H
#include "stm32f10x.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "string.h"// 字符串处理
#include "Delay.h"
#include "OLED.h"
#include "USART.h"
#include "Timer.h"
#include "tuigan.h"
#include "Driver.h"
#include "StepMotor.h"


/* 系统状态枚举，每个状态对应一个命令字符 */
typedef enum {
    STATE_WAIT = 'W',           // 等待状态
    STATE_MOTOR1_MOVE = '1',    // 电机1运动
    STATE_MOTOR2_MOVE = '2',    // 电机2运动
    STATE_GROUP12_MOVE = 'B',   // TIM1+TIM2 电机组运动
    STATE_GROUP34_MOVE = 'C',   // TIM3+TIM4 电机组运动
		STATE_TING='T'              // 停止状态
} SystemState;

/* 单个步进电机控制结构体 */
typedef struct {
    bool active;                // 电机激活状态（true运行，false停止）
    uint8_t direction;          // 电机方向
    uint16_t pulse;             // 脉冲数
    uint16_t repeat;            // 总重复次数
    uint16_t current_repeat;    // 当前已完成的重复次数
    uint32_t last_pulse_time;   // 上次发送脉冲的时间（毫秒）
    uint16_t pulse_interval;    // 脉冲间隔时间（毫秒）
} StepMotorCtrl;// 步进电机控制结构体


/* 双电机组控制结构体 */
typedef struct {
    bool active;                // 组激活状态（true运行，false停止）
    uint8_t dir1, dir2;         // 两个电机的方向（dir1电机1，dir2电机2）
    uint16_t pulse1, pulse2;    // 两个电机的脉冲数
    uint16_t repeat;            // 组重复次数
    uint16_t current_repeat;    // 当前已完成的重复次数
    uint32_t last_pulse_time;   // 上次发送脉冲的时间（毫秒）
    uint16_t pulse_interval;    // 脉冲间隔时间（毫秒）
} GroupMotorCtrl;

/* 系统控制结构体 */
typedef struct {
    SystemState state;          // 当前系统状态（枚举值）
    uint32_t state_timestamp;   // 状态切换时间（毫秒）
    uint8_t last_cmd;           // 最近接收的命令字节

    StepMotorCtrl motor1;
    StepMotorCtrl motor2;
    StepMotorCtrl motor3;
    StepMotorCtrl motor4;

    GroupMotorCtrl group12;     // TIM1+TIM2 电机组
    GroupMotorCtrl group34;     // TIM3+TIM4 电机组
} SystemCtrl;


extern SystemCtrl sys_ctrl;

void TIM5_Init(void);
void GPIO1_Init(void);
void System_StateMachine(void);
void TIM5_UP_IRQHandler(void);
uint32_t GetTick(void);
void Hardware_Init(void);
void ProcessCommand(void);
void System_Init(void);


#endif
