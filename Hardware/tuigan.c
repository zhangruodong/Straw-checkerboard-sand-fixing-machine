#include "stm32f10x.h"                  // Device header
#include "tuigan.h"

// 推杆继电器引脚（GPIOB，高电平吸合）
#define ROD1_EXT  GPIO_Pin_0    // 推杆1 伸
#define ROD1_RET  GPIO_Pin_1    // 推杆1 收
#define ROD2_EXT  GPIO_Pin_2    // 推杆2 伸
#define ROD2_RET  GPIO_Pin_3    // 推杆2 收
#define ROD3_EXT  GPIO_Pin_4    // 推杆3 伸
#define ROD3_RET  GPIO_Pin_5    // 推杆3 收
#define ROD4_EXT  GPIO_Pin_7    // 推杆4 伸
#define ROD4_RET  GPIO_Pin_14   // 推杆4 收

#define ROD_ALL_PIN (ROD1_EXT|ROD1_RET|ROD2_EXT|ROD2_RET|ROD3_EXT|ROD3_RET|ROD4_EXT|ROD4_RET)

void Rod_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
    // 关闭 JTAG、保留 SWD，释放 PB3/PB4 给推杆（之后只能用 SWD 下载）
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // 8 个继电器输出，初始全断开（低电平）
    GPIO_InitStructure.GPIO_Pin = ROD_ALL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, ROD_ALL_PIN);

    // 4 个限位输入（内部上拉）
    GPIO_InitStructure.GPIO_Pin = LIMIT_FRONT | LIMIT_BACK | LIMIT_UP | LIMIT_DOWN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// 双继电器互锁：伸/收/停，两继电器绝不同时吸合
static void Rod_Ctrl(uint16_t ext, uint16_t ret, RodState s) {
    if (s == ROD_EXTEND) {
        GPIO_ResetBits(GPIOB, ret);
        GPIO_SetBits(GPIOB, ext);
    } else if (s == ROD_RETRACT) {
        GPIO_ResetBits(GPIOB, ext);
        GPIO_SetBits(GPIOB, ret);
    } else {   // ROD_STOP
        GPIO_ResetBits(GPIOB, ext);
        GPIO_ResetBits(GPIOB, ret);
    }
}

void Rod1_Set(RodState s) { Rod_Ctrl(ROD1_EXT, ROD1_RET, s); }
void Rod2_Set(RodState s) { Rod_Ctrl(ROD2_EXT, ROD2_RET, s); }
void Rod3_Set(RodState s) { Rod_Ctrl(ROD3_EXT, ROD3_RET, s); }
void Rod4_Set(RodState s) { Rod_Ctrl(ROD4_EXT, ROD4_RET, s); }

uint8_t Limit_Hit(uint16_t pin) {
    return (GPIO_ReadInputDataBit(GPIOB, pin) == RESET);  // 上拉，压到 = 低电平
}
