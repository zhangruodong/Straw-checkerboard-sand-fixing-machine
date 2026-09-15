# 草方格固沙机（Straw checkerboard sand-fixing machine）

基于 STM32F103ZE 的草方格固沙机控制程序。采用**非阻塞状态机**架构，主循环始终能响应串口指令，控制 4 路步进电机 + 4 路推杆/执行器，实现秸秆方格的铺设动作。

## 特性

- **非阻塞状态机**：所有电机动作都用时间戳（`timer_counter`）推进，不用 `Delay_ms` 死等，主循环始终能响应串口指令。
- **4 路步进电机**：TIM1/2/3/4 各驱动一路，PWM 脉冲（PUL）+ GPIO 方向（DIR），定时器溢出中断计数，发够脉冲自动停止。
- **双电机组联动**：电机 1+2、电机 3+4 可成组同步运动。
- **4 路推杆**：每个推杆双继电器（伸 + 收）互锁换向，高电平吸合，绝不同时导通。
- **4 路限位开关**：前/后/上升/下降到位，内部上拉，压到 = 低电平。
- **履带式差速转向**：电机 1=左前、2=右前、3=左后、4=右后，原地转向时左(1+3)、右(2+4)差速。
- **自动铺草流程**：`G` 指令启动，非阻塞状态机跑完「开沟→压草→覆沙→升起→前进→转向」循环。
- **OLED 实时显示**：软件 I2C 驱动，显示运行状态与系统节拍计数。
- **串口指令控制**：USART1 行缓冲接收，支持单字符指令和「字符+数字」标定指令（回车结束）。
- **独立看门狗 + 通信超时**：程序卡死约 2 秒自动复位；非等待状态 120 秒无指令自动停止。

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
| 步进电机 1（左前轮） | PA8 = PUL，PF8 = DIR | PWM + GPIO | PA8 = TIM1_CH1 |
| 步进电机 2（右前轮） | PA0 = PUL，PF0 = DIR | PWM + GPIO | PA0 = TIM2_CH1 |
| 步进电机 3（左后轮） | PA6 = PUL，PF6 = DIR | PWM + GPIO | PA6 = TIM3_CH1 |
| 步进电机 4（右后轮） | PB6 = PUL，PF2 = DIR | PWM + GPIO | PB6 = TIM4_CH1 |
| 推杆 1 伸/收继电器 | PB0 = 伸，PB1 = 收 | 输出 | 双继电器互锁，高电平吸合 |
| 推杆 2 伸/收继电器 | PB2 = 伸，PB3 = 收 | 输出 | 双继电器互锁，高电平吸合 |
| 推杆 3 伸/收继电器 | PB4 = 伸，PB5 = 收 | 输出 | 双继电器互锁，高电平吸合 |
| 推杆 4 伸/收继电器 | PB7 = 伸，PB14 = 收 | 输出 | 双继电器互锁，高电平吸合 |
| 前限位 / 后限位 | PB10 / PB11 | 输入 | 内部上拉，压到 = 低电平 |
| 上升到位 / 下降到位 | PB12 / PB13 | 输入 | 内部上拉，压到 = 低电平 |
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
| `T` | 急停：停止所有电机 + 全部推杆断电，回到等待状态 |
| `G` | 启动自动铺草流程（下降开沟 → 前进压草 → 覆沙 → 升起 → 前进一格 → 原地转向） |
| `A` / `a` | 推杆 1（PB0/PB1）伸出 / 收回 |
| `D` / `d` | 推杆 2（PB2/PB3）伸出 / 收回 |
| `E` / `e` | 推杆 3（PB4/PB5）伸出 / 收回 |
| `F` / `f` | 推杆 4（PB7/PB14）伸出 / 收回 |
| `P<num>` / `N<num>` | 标定：前进 / 后退 `num` 步（缺省用一格步数 `GRID_STEPS`） |
| `L<num>` / `R<num>` | 标定：左转 / 右转 `num` 步（缺省用 `TURN_STEPS`） |

## 标定（免改代码重烧）

所有指令都要以**回车结束**（串口助手勾选「发送新行」）。`P/N/L/R` 后跟步数即可让车走固定脉冲，拿尺量完把数值填回 [Hardware/system.h](Hardware/system.h) 的宏里，不用反复改代码重烧。

1. **标方向**：发 `P100`，看车往前走还是后退。若后退，把 [Hardware/StepMotor.c](Hardware/StepMotor.c) 里的 `TRACK_FWD`/`TRACK_BWD` 值对调。发 `L100` 看是否原地左转，不对则调 `Track_Turn` 里左右分组的 `TRACK_FWD`/`TRACK_BWD`。
2. **标一格步数**：发 `P1000`，量前进距离，按比例算出走一个草方格边长的步数，填 `GRID_STEPS`。
3. **标转向步数**：发 `L500`，量转角，按比例算出转 90° 的步数，填 `TURN_STEPS`。
4. **标推杆时间**：发 `A`/`a` 等手动伸收，掐表量伸出/收回/压草耗时，填 `ROD_DOWN_MS`/`ROD_UP_MS`/`PRESS_MS`。限位开关的常开/常闭若接反，改 [Hardware/tuigan.h](Hardware/tuigan.h) 里 `Limit_Hit` 的 `== RESET` 判断。

> 说明：`TRACK_FWD/BWD` 方向、限位常开/常闭、`GRID_STEPS`/`TURN_STEPS`/`ROD_DOWN_MS`/`ROD_UP_MS`/`PRESS_MS` 这些值取决于机械安装，必须自己实测填。

## 编译

1. 用 Keil MDK 打开 `Project.uvprojx`。
2. 确认目标器件为 STM32F103ZE（512KB Flash）。
3. 编译（Build），通过 ST-Link / J-Link 下载。
4. 需要发布固件时：Keil 菜单 Options for Target → Output 勾选「Create HEX File」，编译后 `.hex` 生成在 `Objects/` 目录。

## 注意事项

- **PB3/PB4 已通过关闭 JTAG 释放给推杆继电器**，之后只能走 SWD（PA13/PA14）下载调试。
- 推杆继电器、限位开关、OLED 都接在 GPIOB，接线前对照接线表核对，避免与电机 PB6 冲突。
- 步进电机的 PUL 脚是定时器 PWM 复用输出，DIR 脚是普通 GPIO 推挽输出，方向 0/1 对应正反转。
- 状态机里各电机的脉冲数/重复次数当前是调试用测试值，正式使用时在 [Hardware/system.c](Hardware/system.c) 的 `System_StateMachine()` 里按需调整。

## 许可

本项目**自有代码**采用 MIT 许可，见 [LICENSE](LICENSE)，覆盖范围为：

| 目录/文件 | 内容 |
|---|---|
| `Hardware/` | 本项目外设驱动与业务逻辑 |
| `User/` | `main.c`、中断、`stm32f10x_conf.h` |
| `System/` | `Delay` 延时模块 |
| `Project.uvprojx`、`README.md` | 工程文件与文档 |

以下为**第三方代码，不在上述 MIT 许可范围内**，版权归各自所有者：

| 目录 | 内容 | 版权 | 许可 |
|---|---|---|---|
| `Library/`（46 个文件） | STM32F10x 标准外设库 V3.5.0 | © 2011 STMicroelectronics | MCD-ST Liberty SW License Agreement V2 |
| `Start/`（11 个文件） | 启动文件、`stm32f10x.h`、`system_stm32f10x.*` | © 2011 STMicroelectronics | 同上 |
| `Start/`（2 个文件） | `core_cm3.c` / `core_cm3.h`（CMSIS V1.30） | © 2009 ARM Limited | ARM 随附声明，限「随支持 ARM 处理器的开发工具分发」 |

> 上述第三方许可的**协议原文不在本仓库内**，各文件头部保留了原始版权与免责声明。逐文件明细见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
