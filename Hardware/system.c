#include "stm32f10x.h"                  // Device header
#include "system.h"

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
		Rod_Init();                       // 推杆继电器 + 限位开关初始化
	TIM5_Init();                       // 定时器5初始化（系统节拍）
	GPIO1_Init();                      // GPIO1 初始化
	StepMotor1_Init();                 // 步进电机1初始化
	StepMotor2_Init();                 // 步进电机2初始化
	StepMotor3_Init();                 // 步进电机3初始化
	StepMotor4_Init();                 // 步进电机4初始化
	IWDG_Init();                       // 独立看门狗（约2秒超时）
}

/**
  * 函    数：获取系统节拍计数
  * 参    数：无
  * 返 回 值：定时器计数器当前值
  */
/**
  * 函数名称：独立看门狗初始化
  * 函数作用：程序跑飞/卡死时约 2 秒自动复位兜底
  * 说明：调试时用 DBGMCU 冻结，断点不会触发复位
  */
void IWDG_Init(void) {
    DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);  // 调试时冻结看门狗
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);     // LSI 40kHz / 64 = 625Hz
    IWDG_SetReload(1250);                     // 625 / 1250 ≈ 2 秒超时
    IWDG_ReloadCounter();
    IWDG_Enable();
}

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
    // 命令：单字符或「字符+数字」，回车结束（串口助手勾选“发送新行”）
    if (Serial1_LineReady) {
        Serial1_LineReady = 0;
        char cmd = Serial1_RxLine[0];
        int num = atoi(Serial1_RxLine + 1);
        if (num < 0) num = 0;
        if (num > 30000) num = 30000;
        sys_ctrl.last_cmd = cmd;

        switch (cmd) {
            case 'T':                       // 急停：停电机 + 推杆全停
                sys_ctrl.state = STATE_TING;
                sys_ctrl.state_timestamp = GetTick();
                Rod1_Set(ROD_STOP); Rod2_Set(ROD_STOP);
                Rod3_Set(ROD_STOP); Rod4_Set(ROD_STOP);
                break;

            case 'G':                       // 启动自动铺草
                sys_ctrl.auto_step = 0;
                sys_ctrl.auto_count = 0;
                sys_ctrl.state = STATE_AUTO;
                sys_ctrl.state_timestamp = GetTick();
                break;

            case 'W': case '1': case '2': case 'B': case 'C':
                sys_ctrl.state = (SystemState)cmd;
                sys_ctrl.state_timestamp = GetTick();
                break;

            // 推杆：即时输出（伸/收），手动标定时用
            case 'A': Rod1_Set(ROD_EXTEND); break;
            case 'a': Rod1_Set(ROD_RETRACT); break;
            case 'D': Rod2_Set(ROD_EXTEND); break;
            case 'd': Rod2_Set(ROD_RETRACT); break;
            case 'E': Rod3_Set(ROD_EXTEND); break;
            case 'e': Rod3_Set(ROD_RETRACT); break;
            case 'F': Rod4_Set(ROD_EXTEND); break;
            case 'f': Rod4_Set(ROD_RETRACT); break;

            // 标定：P 前进 / N 后退 / L 左转 / R 右转，数字=脉冲数，缺省用宏值
            case 'P': Track_Go(num > 0 ? num : GRID_STEPS); break;
            case 'N': Track_Go(-(num > 0 ? num : GRID_STEPS)); break;
            case 'L': Track_Turn(1, num > 0 ? num : TURN_STEPS); break;
            case 'R': Track_Turn(-1, num > 0 ? num : TURN_STEPS); break;

            default:                        // 非法指令，忽略
                break;
        }
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
/**
  * 函数名称：自动铺草流程
  * 函数作用：用 auto_step 状态机循环「下降开沟→前进压草→覆沙→升起→前进一格→原地转向」
  */
void AutoPave(void) {
    uint32_t now = GetTick();

    switch (sys_ctrl.auto_step) {
        case 0:   // 开沟犁下降
            Rod1_Set(ROD_EXTEND);
            sys_ctrl.step_start = now;
            sys_ctrl.auto_step = 1;
            break;

        case 1:   // 等下降到位或超时 → 前进开沟 + 压草轮下降
            if (Limit_Hit(LIMIT_DOWN) || (now - sys_ctrl.step_start >= ROD_DOWN_MS)) {
                Rod1_Set(ROD_STOP);
                Track_Go(GRID_STEPS);          // 前进开沟
                Rod2_Set(ROD_EXTEND);          // 压草轮下降
                sys_ctrl.step_start = now;
                sys_ctrl.auto_step = 2;
            }
            break;

        case 2:   // 前进 + 压草保持
            if (now - sys_ctrl.step_start >= GRID_STEPS + PRESS_MS) {
                Rod3_Set(ROD_EXTEND);          // 覆沙板动作
                sys_ctrl.step_start = now;
                sys_ctrl.auto_step = 3;
            }
            break;

        case 3:   // 覆沙后推杆升起
            if (now - sys_ctrl.step_start >= PRESS_MS) {
                Rod1_Set(ROD_RETRACT);
                Rod2_Set(ROD_RETRACT);
                Rod3_Set(ROD_RETRACT);
                sys_ctrl.step_start = now;
                sys_ctrl.auto_step = 4;
            }
            break;

        case 4:   // 等上升到位 → 前进到下一格
            if (Limit_Hit(LIMIT_UP) || (now - sys_ctrl.step_start >= ROD_UP_MS)) {
                Rod1_Set(ROD_STOP); Rod2_Set(ROD_STOP); Rod3_Set(ROD_STOP);
                Track_Go(GRID_STEPS);          // 前进一个格子间距
                sys_ctrl.auto_count++;
                sys_ctrl.step_start = now;
                sys_ctrl.auto_step = 5;
            }
            break;

        case 5:   // 前进完成 → 判断是否铺满一行
            if (now - sys_ctrl.step_start >= GRID_STEPS) {
                if (sys_ctrl.auto_count >= GRID_PER_ROW) {
                    Track_Turn(1, TURN_STEPS); // 原地转向90°
                    sys_ctrl.auto_count = 0;
                    sys_ctrl.auto_step = 6;
                } else {
                    sys_ctrl.auto_step = 0;
                }
                sys_ctrl.step_start = now;
            }
            break;

        case 6:   // 转向完成 → 继续铺
            if (now - sys_ctrl.step_start >= TURN_STEPS) {
                sys_ctrl.auto_step = 0;
            }
            break;
    }
}

void System_StateMachine(void) {
    uint32_t current_time = GetTick();

    // 更新各电机非阻塞运动
    StepMotor_UpdateNonBlocking();
    StepMotor_UpdateGroup12();
    StepMotor_UpdateGroup34();
    ProcessCommand();

    // 通信超时保护：非等待/非铺草状态 120 秒没收到新指令，自动停止
    if (sys_ctrl.state != STATE_WAIT && sys_ctrl.state != STATE_AUTO &&
        current_time - sys_ctrl.state_timestamp >= COMM_TIMEOUT_MS) {
        StopAllMotors();
        Rod1_Set(ROD_STOP); Rod2_Set(ROD_STOP);
        Rod3_Set(ROD_STOP); Rod4_Set(ROD_STOP);
        sys_ctrl.state = STATE_WAIT;
    }

    switch(sys_ctrl.state) {
        case STATE_WAIT:
            // 等待状态，不执行任何动作
            break;

        case STATE_TING:
            StopAllMotors();
            Rod1_Set(ROD_STOP); Rod2_Set(ROD_STOP);
            Rod3_Set(ROD_STOP); Rod4_Set(ROD_STOP);
            sys_ctrl.state = STATE_WAIT; // 停止后回到等待状态
            break;

        case STATE_MOTOR1_MOVE:
            if (!sys_ctrl.motor1.started) {
                sys_ctrl.motor1.direction = 0;
                sys_ctrl.motor1.pulse = 500;
                sys_ctrl.motor1.repeat = 3;
                sys_ctrl.motor1.current_repeat = 0;
                sys_ctrl.motor1.active = true;
                sys_ctrl.motor1.started = true;
                sys_ctrl.motor1.last_pulse_time = current_time;

                StepMotor1_SetDirection(sys_ctrl.motor1.direction);
            } else if (!sys_ctrl.motor1.active) {
                sys_ctrl.motor1.started = false;
                sys_ctrl.state = STATE_WAIT;
            }
            break;

        case STATE_MOTOR2_MOVE:
            if (!sys_ctrl.motor2.started) {
                sys_ctrl.motor2.direction = 1;
                sys_ctrl.motor2.pulse = 300;
                sys_ctrl.motor2.repeat = 5;
                sys_ctrl.motor2.current_repeat = 0;
                sys_ctrl.motor2.active = true;
                sys_ctrl.motor2.started = true;
                sys_ctrl.motor2.last_pulse_time = current_time;

                StepMotor2_SetDirection(sys_ctrl.motor2.direction);
            } else if (!sys_ctrl.motor2.active) {
                sys_ctrl.motor2.started = false;
                sys_ctrl.state = STATE_WAIT;
            }
            break;

        case STATE_GROUP12_MOVE:
            if (!sys_ctrl.group12.started) {
                sys_ctrl.group12.dir1 = 0;
                sys_ctrl.group12.dir2 = 1;
                sys_ctrl.group12.pulse1 = 500;
                sys_ctrl.group12.pulse2 = 500;
                sys_ctrl.group12.repeat = 1;
                sys_ctrl.group12.current_repeat = 0;
                sys_ctrl.group12.active = true;
                sys_ctrl.group12.started = true;
                sys_ctrl.group12.last_pulse_time = current_time;

                StepMotor1_SetDirection(sys_ctrl.group12.dir1);
                StepMotor2_SetDirection(sys_ctrl.group12.dir2);
            } else if (!sys_ctrl.group12.active) {
                sys_ctrl.group12.started = false;
                sys_ctrl.state = STATE_WAIT;
            }
            break;

        case STATE_GROUP34_MOVE:
            if (!sys_ctrl.group34.started) {
                sys_ctrl.group34.dir1 = 0;
                sys_ctrl.group34.dir2 = 0;
                sys_ctrl.group34.pulse1 = 300;
                sys_ctrl.group34.pulse2 = 300;
                sys_ctrl.group34.repeat = 3;
                sys_ctrl.group34.current_repeat = 0;
                sys_ctrl.group34.active = true;
                sys_ctrl.group34.started = true;
                sys_ctrl.group34.last_pulse_time = current_time;

                StepMotor3_SetDirection(sys_ctrl.group34.dir1);
                StepMotor4_SetDirection(sys_ctrl.group34.dir2);
            } else if (!sys_ctrl.group34.active) {
                sys_ctrl.group34.started = false;
                sys_ctrl.state = STATE_WAIT;
            }
            break;
        case STATE_AUTO:
            AutoPave();
            break;

        // 其他状态...
        default:
            // 未定义状态
            break;
    }
}
