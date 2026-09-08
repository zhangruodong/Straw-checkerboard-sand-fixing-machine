#include "stm32f10x.h"
#include "system.h"
//#include "Delay.h"
//#include "OLED.h"
//#include "Timer.h"
//#include "StepMotor.h"
// 外部声明全局控制结构体
extern SystemCtrl sys_ctrl;
/*------------------------------------------------------TIM1-----------------------------------------------------------------------*/
	//TIM1时钟  步进电机1  PUL  PA8    DIR  PF8
 volatile uint32_t StepMotor_PUL_CNT1 = 0;
 volatile uint32_t StepMotor_PUL_SET1 = 0;
 
 /**TIM1
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
	void StepMotor1_Init(void)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure1;
		GPIO_InitStructure1.GPIO_Mode=GPIO_Mode_AF_PP;//PA8PUL
		GPIO_InitStructure1.GPIO_Pin=GPIO_Pin_8;
		GPIO_InitStructure1.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOA,&GPIO_InitStructure1);
		
		GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_Out_PP;//PF8  DIR
		GPIO_InitStructure1.GPIO_Pin = GPIO_Pin_8;
		GPIO_Init(GPIOF,&GPIO_InitStructure1);
		TIM1_Init();
	}
	
/**TIM1
  * 函    数：PWM设置CCR
  * 参    数：Compare 要写入的CCR的值，范围：0~100
  * 返 回 值：无
  * 注意事项：CCR和ARR共同决定占空比，此函数仅设置CCR的值，并不直接是占空比
  *           占空比Duty = CCR / (ARR + 1)
  */
void StepMotor1_SetCompare(uint16_t Compare)
{
	TIM_SetCompare1(TIM1, Compare);		//设置CCR1的值
}

void TIM1_UP_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        StepMotor_PUL_CNT1++;
        if (StepMotor_PUL_CNT1 >= StepMotor_PUL_SET1)
        {//TIM_CtrlPWMOutputs(TIM1, DISABLE);							//使能PWM输出 
		  	//TIM1->CCER &= ~(TIM_CCER_CC1E); // 清除CH1的输出使能位，禁用CH1
            TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
            StepMotor_PUL_CNT1 = 0;
        }
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}
void StepMotor1_SetPulse(uint16_t Pulse)
{
    StepMotor_PUL_SET1 = Pulse;
    StepMotor_PUL_CNT1 = 0;
	//TIM_CtrlPWMOutputs(TIM1, ENABLE);							//使能PWM输出 
	//TIM1->CCER |= TIM_CCER_CC1E;
    // 使用库函数替代寄存器操作 - 使能通道1输出
    TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
}
void StepMotor1_SetDirection(uint8_t Direction)
{
	if (Direction == 0)
	{
		GPIO_ResetBits(GPIOF, GPIO_Pin_8);		//设置PF8引脚为低电平，方向为正转
	}
	else
	{
		GPIO_SetBits(GPIOF,GPIO_Pin_8 );		//设置PF8引脚为高电平，方向为反转
	}
}


///**
//  * 函    数：设置步进电机转速
//  * 参    数：frequency - 期望的脉冲频率 (Hz)
//  * 返 回 值：无
//  */
//void StepMotor1_SetFrequency(uint32_t frequency)
//{
//    // 1. 计算ARR值 (ARR = 定时器时钟 / 频率 - 1)
//    uint32_t timer_clock = SystemCoreClock / (TIM1->PSC + 1); // 获取当前定时器实际时钟
//    uint32_t arr_value = (timer_clock / frequency) - 1;
//    
//    // 2. 设置ARR寄存器
//    TIM_SetAutoreload(TIM1, arr_value);
//    
//    // 3. 保持50%占空比 (CCR = ARR / 2)
//    TIM_SetCompare1(TIM1, arr_value / 2);
//}

void Integration1(uint16_t dir, uint16_t pul, uint16_t ring)   //,uint32_t rpm
{
//	// 根据RPM计算所需频率
//    // 假设电机步距角1.8°（200步/转）：
//    const uint32_t steps_per_rev = 200; 
//    uint32_t frequency = (rpm * steps_per_rev) / 60; // 频率 = (RPM×每转步数)/60
//    
//    // 设置转速
//    StepMotor1_SetFrequency(frequency);
    for(uint16_t i = 0; i < ring; i++)
    {
        StepMotor1_SetDirection(dir); // 设置方向
        StepMotor1_SetPulse(pul); // 设置脉冲

        // 将延迟降低到500ms
        Delay_ms(10); 
    }
}


/*------------------------------------------------------TIM2-----------------------------------------------------------------------*/
//TIM2时钟
 volatile uint32_t StepMotor_PUL_CNT2 = 0;
 volatile uint32_t StepMotor_PUL_SET2 = 0;


 /**TIM  步进电机2 TIM2 CH1  PA0   PUL      GPIOF  PIN  F0
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
	void StepMotor2_Init(void)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure2;
		GPIO_InitStructure2.GPIO_Mode=GPIO_Mode_AF_PP;//PA0PUL
		GPIO_InitStructure2.GPIO_Pin=GPIO_Pin_0;
		GPIO_InitStructure2.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOA,&GPIO_InitStructure2);
		
		GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_Out_PP;//PF0  DIR
		GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_0;
		GPIO_Init(GPIOF,&GPIO_InitStructure2);
		TIM2_Init();
	}
 void StepMotor2_SetCompare(uint16_t Compare)
{
	TIM_SetCompare1(TIM2, Compare);		//设置CCR1的值
}
void TIM2_IRQHandler(void)  // 更改中断处理函数名称
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  // 所有TIM1改为TIM2
    {
        StepMotor_PUL_CNT2++;
        if (StepMotor_PUL_CNT2>= StepMotor_PUL_SET2)
        {
            // 禁用TIM2的通道1输出
            TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Disable);
            StepMotor_PUL_CNT2 = 0;
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // 清除TIM2中断标志
    }
}

void StepMotor2_SetPulse(uint16_t Pulse)
{
    StepMotor_PUL_SET2 = Pulse;
    StepMotor_PUL_CNT2 = 0;
	TIM_CtrlPWMOutputs(TIM2, ENABLE);							//使能PWM输出 
	
}

void StepMotor2_SetDirection(uint8_t Direction)
{
	if (Direction == 0)
	{
		GPIO_ResetBits(GPIOF, GPIO_Pin_0);		//设置PF0引脚为低电平，方向为正转
	}
	else
	{
		GPIO_SetBits(GPIOF,GPIO_Pin_0 );		//设置PF0引脚为高电平，方向为反转
	}
}
void Integration2(uint16_t dir, uint16_t pul, uint16_t ring)   
{

    for(uint16_t i = 0; i < ring; i++)
    {
        StepMotor2_SetDirection(dir); // 设置方向
        StepMotor2_SetPulse(pul); // 设置脉冲

        // 将延迟降低到500ms
        Delay_ms(10); 
    }
}
/*------------------------------------------------------TIM3-----------------------------------------------------------------------*/
//TIM3时钟
 volatile uint32_t StepMotor_PUL_CNT3 = 0;
 volatile uint32_t StepMotor_PUL_SET3 = 0;
/**TIM  TIM3 CH1  PA6   PUL      GPIOF  PIN  F1
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
	void StepMotor3_Init(void)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure3;
		GPIO_InitStructure3.GPIO_Mode=GPIO_Mode_AF_PP;//PA6PUL
		GPIO_InitStructure3.GPIO_Pin=GPIO_Pin_6;
		GPIO_InitStructure3.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOA,&GPIO_InitStructure3);
		
		GPIO_InitStructure3.GPIO_Mode = GPIO_Mode_Out_PP;//PF0  DIR
		GPIO_InitStructure3.GPIO_Pin = GPIO_Pin_6;
		GPIO_Init(GPIOF,&GPIO_InitStructure3);
		TIM3_Init();
	}
 void StepMotor3_SetCompare(uint16_t Compare)
{
	TIM_SetCompare1(TIM3, Compare);		//设置CCR1的值
}
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  // 改为TIM3
    {
        StepMotor_PUL_CNT3++;
        if (StepMotor_PUL_CNT3 >= StepMotor_PUL_SET3)  // 修正变量名
        {
            // 禁用TIM3的通道1输出
            TIM_CCxCmd(TIM3, TIM_Channel_1, TIM_CCx_Disable);  // 改为TIM3
            StepMotor_PUL_CNT3 = 0;
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  // 清除TIM3中断标志
    }
}
void StepMotor3_SetPulse(uint16_t Pulse)
{
    StepMotor_PUL_SET3 = Pulse;
    StepMotor_PUL_CNT3 = 0;
	TIM_CtrlPWMOutputs(TIM3, ENABLE);							//使能PWM输出 
	
}

void StepMotor3_SetDirection(uint8_t Direction)
{
	if (Direction == 0)
	{
		GPIO_ResetBits(GPIOF, GPIO_Pin_6);		//设置PF6引脚为低电平，方向为正转
	}
	else
	{
		GPIO_SetBits(GPIOF,GPIO_Pin_6 );		//设置PF6引脚为高电平，方向为反转
	}
}
void Integration3(uint16_t dir, uint16_t pul, uint16_t ring)   
{

    for(uint16_t i = 0; i < ring; i++)
    {
        StepMotor3_SetDirection(dir); // 设置方向
        StepMotor3_SetPulse(pul); // 设置脉冲

        // 将延迟降低到500ms
        Delay_ms(10); 
    }
}
/*------------------------------------------------------TIM4-----------------------------------------------------------------------*/
//TIM4时钟
 volatile uint32_t StepMotor_PUL_CNT4 = 0;
 volatile uint32_t StepMotor_PUL_SET4 = 0;
/**TIM  TIM4 CH1  PB6   PUL      GPIOF  PIN  F2
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
	void StepMotor4_Init(void)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure4;
		GPIO_InitStructure4.GPIO_Mode=GPIO_Mode_AF_PP;//PB6PUL
		GPIO_InitStructure4.GPIO_Pin=GPIO_Pin_6;
		GPIO_InitStructure4.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure4);
		
		GPIO_InitStructure4.GPIO_Mode = GPIO_Mode_Out_PP;//PF2  DIR
		GPIO_InitStructure4.GPIO_Pin = GPIO_Pin_2;
		GPIO_Init(GPIOF,&GPIO_InitStructure4);
		TIM4_Init();
	}
 void StepMotor4_SetCompare(uint16_t Compare)
{
	TIM_SetCompare1(TIM4, Compare);		//设置CCR1的值
}
void TIM4_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  
    {
        StepMotor_PUL_CNT4++; 
        if (StepMotor_PUL_CNT4 >= StepMotor_PUL_SET4)  // 使用TIM4对应的设置值
        {
            // 禁用TIM4的通道1输出
            TIM_CCxCmd(TIM4, TIM_Channel_1, TIM_CCx_Disable);  
            StepMotor_PUL_CNT4 = 0;  // 清零TIM4的计数器
        }
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  // 清除TIM4中断标志
    }
}
void StepMotor4_SetPulse(uint16_t Pulse) 
{
    StepMotor_PUL_SET4 = Pulse;  
    StepMotor_PUL_CNT4 = 0;      
    TIM_CtrlPWMOutputs(TIM4, ENABLE);  
}

void StepMotor4_SetDirection(uint8_t Direction)
{
	if (Direction == 0)
	{
		GPIO_ResetBits(GPIOF, GPIO_Pin_2);		//设置PF2引脚为低电平，方向为正转
	}
	else
	{
		GPIO_SetBits(GPIOF,GPIO_Pin_2 );		//设置PF2引脚为高电平，方向为反转
	}
}
void Integration4(uint16_t dir, uint16_t pul, uint16_t ring)   
{

    for(uint16_t i = 0; i < ring; i++)
    {
        StepMotor4_SetDirection(dir); // 设置方向
        StepMotor4_SetPulse(pul); // 设置脉冲

        // 将延迟降低到500ms
        Delay_ms(10); 
    }
}


/*------------------------------------------------------非阻塞-----------------------------------------------------------------------*/

void StepMotor_UpdateNonBlocking(void) {
    uint32_t current_time = GetTick();
    
    // 更新单个电机状态
    for (int i = 1; i <= 4; i++) {
        StepMotorCtrl* motor = NULL;
        if (i == 1) motor = &sys_ctrl.motor1;
        else if (i == 2) motor = &sys_ctrl.motor2;
        else if (i == 3) motor = &sys_ctrl.motor3;
        else if (i == 4) motor = &sys_ctrl.motor4;
        
        if (motor->active) {
            // 检查是否需要发送新脉冲
            if (current_time - motor->last_pulse_time >= motor->pulse_interval) {
                motor->last_pulse_time = current_time;
                
                // 发送脉冲
                if (i == 1) StepMotor1_SetPulse(motor->pulse);
                else if (i == 2) StepMotor2_SetPulse(motor->pulse);
                else if (i == 3) StepMotor3_SetPulse(motor->pulse);
                else if (i == 4) StepMotor4_SetPulse(motor->pulse);
                
                motor->current_repeat++;
                
                // 检查是否完成所有重复
                if (motor->current_repeat >= motor->repeat) {
                    motor->active = false;
                    // 禁用电机输出
                    if (i == 1) TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
                    else if (i == 2) TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Disable);
                    else if (i == 3) TIM_CCxCmd(TIM3, TIM_Channel_1, TIM_CCx_Disable);
                    else if (i == 4) TIM_CCxCmd(TIM4, TIM_Channel_1, TIM_CCx_Disable);
                }
            }
        }
    }
}

// TIM1+TIM2分组控制
void StepMotor_UpdateGroup12(void) {
    if (!sys_ctrl.group12.active) return;
    
    uint32_t current_time = GetTick();
    
    if (current_time - sys_ctrl.group12.last_pulse_time >= sys_ctrl.group12.pulse_interval) {
        sys_ctrl.group12.last_pulse_time = current_time;
        
        // 发送脉冲到两个电机
        StepMotor1_SetPulse(sys_ctrl.group12.pulse1);
        StepMotor2_SetPulse(sys_ctrl.group12.pulse2);
        
        sys_ctrl.group12.current_repeat++;
        
        // 检查是否完成所有重复
        if (sys_ctrl.group12.current_repeat >= sys_ctrl.group12.repeat) {
            sys_ctrl.group12.active = false;
            // 禁用电机输出
            TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
            TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Disable);
        }
    }
}

// TIM3+TIM4分组控制
void StepMotor_UpdateGroup34(void) {
    if (!sys_ctrl.group34.active) return;
    
    uint32_t current_time = GetTick();
    
    if (current_time - sys_ctrl.group34.last_pulse_time >= sys_ctrl.group34.pulse_interval) {
        sys_ctrl.group34.last_pulse_time = current_time;
        
        // 发送脉冲到两个电机
        StepMotor3_SetPulse(sys_ctrl.group34.pulse1);
        StepMotor4_SetPulse(sys_ctrl.group34.pulse2);
        
        sys_ctrl.group34.current_repeat++;
        
        // 检查是否完成所有重复
        if (sys_ctrl.group34.current_repeat >= sys_ctrl.group34.repeat) {
            sys_ctrl.group34.active = false;
            // 禁用电机输出
            TIM_CCxCmd(TIM3, TIM_Channel_1, TIM_CCx_Disable);
            TIM_CCxCmd(TIM4, TIM_Channel_1, TIM_CCx_Disable);
        }
    }
}

// 停止所有电机
void StopAllMotors(void) {
    // 停止单个电机
    sys_ctrl.motor1.active = false;
    sys_ctrl.motor2.active = false;
    sys_ctrl.motor3.active = false;
    sys_ctrl.motor4.active = false;
    
    // 停止分组
    sys_ctrl.group12.active = false;
    sys_ctrl.group34.active = false;
    
    // 禁用所有PWM输出
    TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
    TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Disable);
    TIM_CCxCmd(TIM3, TIM_Channel_1, TIM_CCx_Disable);
    TIM_CCxCmd(TIM4, TIM_Channel_1, TIM_CCx_Disable);
}
