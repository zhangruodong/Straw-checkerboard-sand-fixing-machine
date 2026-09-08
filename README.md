# 草方格固沙机（Straw checkerboard sand-fixing machine）

基于 STM32F103ZE 的草方格固沙机控制程序。采用**非阻塞状态机**架构，主循环始终能响应串口指令，控制 4 路步进电机 + 4 路推杆/执行器，实现秸秆方格的铺设动作。

## 特性

- **非阻塞状态机**：所有电机动作都用时间戳（`timer_counter`）推进，不用 `Delay_ms` 死等，主循环始终能响应串口指令。
- **4 路步进电机**：TIM1/2/3/4 各驱动一路，PWM 脉冲（PUL）+ GPIO 方向（DIR），定时器溢出中断计数，发够脉冲自动停止。
- **双电机组联动**：电机 1+2、电机 3+4 可成组同步运动。
- **4 路推杆/执行器输出**：推挽输出，驱动气缸/电磁阀等执行机构。
- **OLED 实时显示**：软件 I2C 驱动，显示运行状态与系统节拍计数。
- **串口指令控制**：USART1 接收单字符指令，直接切换状态机状态。

## 硬件平台

| 项目 | 参数 |
|---|---|
| MCU | STM32F103ZE（大容量，512KB Flash，72MHz） |
| 开发环境 | Keil MDK（ARMCC） |
| 串口 | USART1 + USART2，9600 波特率，8N1 |

## 目录结构

```
Straw checkerboard sand-fixing machine/
├── Hardware/    # 外设驱动（步进电机、OLED、串口、定时器、推杆、状态机）
├── User/        # main.c、stm32f10x_it.c、stm32f10x_conf.h
├── System/      # Delay 延时
├── Library/     # STM32F10x 标准外设库
├── Start/       # 启动文件、system_stm32f10x、core_cm3
└── Listings/    # 编译产物（.map 等，可忽略）
```

**核心逻辑在 [Hardware/system.c](Hardware/system.c)**：`System_StateMachine()` 是主状态机，`ProcessCommand()` 处理串口指令。

## 接线表

| 外设 | 引脚 | 类型 | 说明 |
|---|---|---|---|
| 步进电机 1 | PA8 = PUL，PF8 = DIR | PWM + GPIO | PA8 = TIM1_CH1 |
| 步进电机 2 | PA0 = PUL，PF0 = DIR | PWM + GPIO | PA0 = TIM2_CH1 |
| 步进电机 3 | PA6 = PUL，PF6 = DIR | PWM + GPIO | PA6 = TIM3_CH1 |
| 步进电机 4 | PB6 = PUL，PF2 = DIR | PWM + GPIO | PB6 = TIM4_CH1 |
| 推杆/执行器 1 | PF13 | 输出 | 推挽输出，初始低电平 |
| 推杆/执行器 2 | PF14 | 输出 | 推挽输出，初始低电平 |
| 推杆/执行器 3 | PF15 | 输出 | 推挽输出，初始低电平 |
| 推杆/执行器 4 | PG1 | 输出 | 推挽输出，初始低电平 |
| 串口 1（指令） | PA9 = TX，PA10 = RX | UART | 状态机指令输入 |
| 串口 2 | PA2 = TX，PA3 = RX | UART | 预留 |
| OLED 屏 | PB8 = SCL，PB9 = SDA | 软件 I2C | 从机地址 0x78 |

## 串口指令

| 指令 | 功能 |
|---|---|
| `W` | 等待 |
| `1` | 电机 1 运动 |
| `2` | 电机 2 运动 |
| `B` | 电机 1 + 2 组联动 |
| `C` | 电机 3 + 4 组联动 |
| `T` | 停止所有电机，回到等待状态 |

## 编译

1. 用 Keil MDK 打开 `Project.uvprojx`。
2. 确认目标器件为 STM32F103ZE（512KB Flash）。
3. 编译（Build），通过 ST-Link / J-Link 下载。
4. 需要发布固件时：Keil 菜单 Options for Target → Output 勾选「Create HEX File」，编译后 `.hex` 生成在 `Objects/` 目录。

## 注意事项

- **PA13（SWDIO）/ PA14（SWCLK）保留给 SWD 下载调试**，不要接外设。
- 步进电机的 PUL 脚是定时器 PWM 复用输出，DIR 脚是普通 GPIO 推挽输出，方向 0/1 对应正反转。
- 状态机里各电机的脉冲数/重复次数当前是调试用测试值，正式使用时在 [Hardware/system.c](Hardware/system.c) 的 `System_StateMachine()` 里按需调整。
