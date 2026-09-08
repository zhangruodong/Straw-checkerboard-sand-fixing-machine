#include "stm32f10x.h"                  // Device header
#include "system.h"

uint8_t cmd=0 ;                        // 接收到的命令变量
SystemCtrl sys_ctrl;                   // 系统控制结构体（全局）

volatile uint32_t timer_counter = 0;   // 全局计数器，用于记录定时器中断次数


/**
  * 函    数：硬件初始化
  * 参    数：无
  * 返 回 值：无
  */
void Hardware_Init(void)
{
	OLED_Init();                       // OLED 屏幕初始化
	Serial_Init();                     // 串口初始化
	PF13_Init();                       // PF13 引脚初始化
	PF14_Init();                       // PF14 引脚初始化
	PF15_Init();                       // PF15 引脚初始化
	PG1_Init();                        // PG1 引脚初始化
	TIM5_Init();                       // 定时器5初始化（系统节拍）
	GPIO1_Init();                      // GPIO1 初始化
	TIM5_UP_IRQHandler();              // 定时器5中断处理
	StepMotor1_Init();                 // 步进电机1初始化
	StepMotor2_Init();                 // 步进电机2初始化
	StepMotor3_Init();                 // 步进电机3初始化
	StepMotor4_Init();                 // 步进电机4初始化
}

/**
  * 函    数：获取系统节拍计数
  * 参    数：无
  * 返 回 值：定时器计数器当前值
  */
uint32_t GetTick(void) {
    return timer_counter;
}

/**
  * 函    数：处理串口命令
  * 参    数：无
  * 返 回 值：无
  * 说    明：读取串口1接收到的命令，直接转换为系统状态
  */
void ProcessCommand(void) {
    if(Serial1_GetRxFlag()) {
        uint8_t cmd = Serial1_GetRxData();
        sys_ctrl.last_cmd = cmd;
        sys_ctrl.state = (SystemState)cmd; // 直接将命令转换为状态
    }
}

/**
  * 函    数：系统状态初始化
  * 参    数：无
  * 返 回 值：无
  */
void System_Init(void) {
    memset(&sys_ctrl, 0, sizeof(sys_ctrl)); // 清零
    sys_ctrl.state = STATE_WAIT;

    // 设置默认参数
    sys_ctrl.motor1.pulse_interval = 10;
    sys_ctrl.motor2.pulse_interval = 10;
    sys_ctrl.motor3.pulse_interval = 10;
    sys_ctrl.motor4.pulse_interval = 10;
    sys_ctrl.group12.pulse_interval = 10;
    sys_ctrl.group34.pulse_interval = 10;
}

/**
  * 函    数：系统状态机
  * 参    数：无
  * 返 回 值：无
  * 说    明：非阻塞式轮询更新各步进电机，并根据当前状态执行对应动作
  */
void System_StateMachine(void) {
    uint32_t current_time = GetTick();

    // 更新各电机非阻塞运动
    StepMotor_UpdateNonBlocking();
    StepMotor_UpdateGroup12();
    StepMotor_UpdateGroup34();
    ProcessCommand();

    switch(sys_ctrl.state) {
        case STATE_WAIT:
            // 等待状态，不执行任何动作
            break;

        case STATE_TING:
            StopAllMotors();
            sys_ctrl.state = STATE_WAIT; // 停止后回到等待状态
            break;

        case STATE_MOTOR1_MOVE:
            if (!sys_ctrl.motor1.active) {
                // 开始电机1运动任务
                sys_ctrl.motor1.direction = 0; // 方向
                sys_ctrl.motor1.pulse = 500;   // 脉冲数
                sys_ctrl.motor1.repeat = 3;    // 重复次数
                sys_ctrl.motor1.current_repeat = 0;
                sys_ctrl.motor1.active = true;
                sys_ctrl.motor1.last_pulse_time = current_time;

                // 设置方向
                StepMotor1_SetDirection(sys_ctrl.motor1.direction);
            }
            break;

        case STATE_MOTOR2_MOVE:
            if (!sys_ctrl.motor2.active) {
                // 开始电机2运动任务
                sys_ctrl.motor2.direction = 1; // 方向
                sys_ctrl.motor2.pulse = 300;   // 脉冲数
                sys_ctrl.motor2.repeat = 5;    // 重复次数
                sys_ctrl.motor2.current_repeat = 0;
                sys_ctrl.motor2.active = true;
                sys_ctrl.motor2.last_pulse_time = current_time;

                // 设置方向
                StepMotor2_SetDirection(sys_ctrl.motor2.direction);
            }
            break;

        case STATE_GROUP12_MOVE:
            if (!sys_ctrl.group12.active) {
                // 开始电机1、2组运动任务
                sys_ctrl.group12.dir1 = 0;
                sys_ctrl.group12.dir2 = 1;
                sys_ctrl.group12.pulse1 = 500;
                sys_ctrl.group12.pulse2 = 500;
                sys_ctrl.group12.repeat = 1;
                sys_ctrl.group12.current_repeat = 0;
                sys_ctrl.group12.active = true;
                sys_ctrl.group12.last_pulse_time = current_time;

                // 设置方向
                StepMotor1_SetDirection(sys_ctrl.group12.dir1);
                StepMotor2_SetDirection(sys_ctrl.group12.dir2);
            }
            break;

        case STATE_GROUP34_MOVE:
            if (!sys_ctrl.group34.active) {
                // 开始电机3、4组运动任务
                sys_ctrl.group34.dir1 = 0;
                sys_ctrl.group34.dir2 = 0;
                sys_ctrl.group34.pulse1 = 300;
                sys_ctrl.group34.pulse2 = 300;
                sys_ctrl.group34.repeat = 3;
                sys_ctrl.group34.current_repeat = 0;
                sys_ctrl.group34.active = true;
                sys_ctrl.group34.last_pulse_time = current_time;

                // 设置方向
                StepMotor3_SetDirection(sys_ctrl.group34.dir1);
                StepMotor4_SetDirection(sys_ctrl.group34.dir2);
            }
            break;

        // 其他状态...
        default:
            // 未定义状态
            break;
    }
}
