#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

uint8_t Serial_RxData1;		// 定义串口1接收的数据变量
uint8_t Serial_RxFlag1;
char Serial1_RxLine[16];            // 命令行缓冲（带数字的标定命令）
volatile uint8_t Serial1_LineLen = 0;    // 已接收字节数
volatile uint8_t Serial1_LineReady = 0;  // 完整命令行就绪

uint8_t Serial_RxData2;		// 定义串口2接收的数据变量
uint8_t Serial_RxFlag2;		// 定义串口2接收的标志位（USART2）

/**
  * 函    数：串口初始化
  * 参    数：无
  * 返 回 值：无
  */
void Serial_Init(void)
{
    /* 开启时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); // USART1 在 APB2 上
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);                        // USART2 在 APB1 上

    /* GPIO初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;

    // USART1 的 TX(PA9) 和 USART2 的 TX(PA2) 配置为复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_9;            // PA2 (USART2_TX), PA9 (USART1_TX)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART1 的 RX(PA10) 和 USART2 的 RX(PA3) 配置为浮空输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;           // 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_10;           // PA3 (USART2_RX), PA10 (USART1_RX)
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART 初始化 */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure); // 初始化 USART1
    USART_Init(USART2, &USART_InitStructure); // 初始化 USART2

    /* 中断配置 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 使能 USART1 接收中断
    // USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 串口2暂未使用，接收中断暂不开启

    /* NVIC 配置 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;

    // USART1 中断通道
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // USART2 中断通道
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级，可自行调整
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        // 响应优先级，与串口1区分
    NVIC_Init(&NVIC_InitStructure);

    /* 使能 USART */
    USART_Cmd(USART1, ENABLE); // 使能 USART1
    USART_Cmd(USART2, ENABLE); // 使能 USART2
}




void Serial1_SendByte(uint8_t Byte)// 通过USART1发送一个字节
{
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	// 等待发送完成
}

void Serial2_SendByte(uint8_t Byte)// 通过USART2发送一个字节
{
	USART_SendData(USART2, Byte);		// 把字节写入发送数据寄存器，USART自动开始发送
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);	// 等待发送完成
	// 下次写入数据寄存器时会自动清除发送完成标志位，故此处循环等待发送标志位
}

/**
  * 函    数：计算 X 的 Y 次方（供串口发送函数内部使用，USART1/USART2 共用）
  * 参    数：X 底数，Y 指数
  * 返 回 值：结果值（X 的 Y 次方）
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	// 结果初值置为1
	while (Y --)			// 循环执行 Y 次
	{
		Result *= X;		// 把 X 累乘到结果
	}
	return Result;
}

/**
  * 函    数：串口发送数字
  * 参    数：Number 要发送的数字，范围：0~4294967295
  * 参    数：Length 要发送数字的长度，范围：0~10
  * 返 回 值：无
  */
void Serial1_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		// 按数字长度循环，逐位发送
	{
		Serial1_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');	// 依次调用 Serial1_SendByte 发送每位数字（USART1）
	}
}

void Serial2_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		// 按数字长度循环，逐位发送
	{
		Serial2_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');	// 通过 USART2 发送每位数字
	}
}



/**
  * 函    数：获取串口接收标志位
  * 参    数：无
  * 返 回 值：串口接收标志位，范围：0~1，接收到数据后标志位置1，读取后标志位自动清零
  */
uint8_t Serial1_GetRxFlag(void)
{
	if (Serial_RxFlag1 == 1)			// 如果标志位为1
	{
		Serial_RxFlag1 = 0;
		return 1;					// 则返回1，并自动清零标志位
	}
	return 0;						// 如果标志位为0，则返回0
}

uint8_t Serial2_GetRxFlag(void)
{
	if (Serial_RxFlag2 == 1)			// 如果标志位为1
	{
		Serial_RxFlag2 = 0;
		return 1;					// 则返回1，并自动清零标志位
	}
	return 0;						// 如果标志位为0，则返回0
}


/**
  * 函    数：获取串口接收到的数据
  * 参    数：无
  * 返 回 值：接收到的数据，范围：0~255
  */
uint8_t Serial1_GetRxData(void)
{
	return Serial_RxData1;			// 返回接收到的数据变量（USART1）
}

uint8_t Serial2_GetRxData(void)
{
	return Serial_RxData2;			// 通过 USART2 返回
}

/**
  * 函    数：USART1 中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *          函数名为预定义名称，可从启动文件复制
  *          请确保函数名完全正确，不能有任何差异，否则中断函数将无法进入
  */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) // 判断是否 USART1 的接收事件
    {
        uint8_t c = USART_ReceiveData(USART1);           // 读取数据寄存器
        if (c == '\r' || c == '\n') {                  // 回车/换行 = 一条命令结束
            if (Serial1_LineLen > 0) {
                Serial1_RxLine[Serial1_LineLen] = 0;     // 补字符串结尾
                Serial1_LineReady = 1;                   // 命令就绪
            }
            Serial1_LineLen = 0;
        } else if (Serial1_LineLen < 15) {               // 缓冲最多 15 字节
            Serial1_RxLine[Serial1_LineLen++] = c;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);  // 清 RXNE 标志位
    }
}

/**
  * 函    数：USART2 中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  */
void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)		// 判断是否是 USART2 的接收事件触发的中断
	{
		Serial_RxData2 = USART_ReceiveData(USART2);			// 读取数据寄存器，存储到接收数据变量
		Serial_RxFlag2 = 1;									// 置接收标志位为1
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);		// 清除 USART2 的 RXNE 标志位
	}
}
