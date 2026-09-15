# 第三方代码声明

本仓库根目录的 [LICENSE](LICENSE)（MIT）**只覆盖本项目自有代码**：

```
Hardware/    User/    System/    README.md    Project.uvprojx
```

以下目录为**第三方代码**，版权归各自所有者，**不在本项目的 MIT 许可范围内**，按各自的原始许可使用。

## Library/ —— STM32F10x 标准外设库 V3.5.0

- 文件数：46
- 版权：© 2011 STMicroelectronics（MCD Application Team）
- 许可：MCD-ST Liberty SW License Agreement V2
- 本仓库**未包含**该协议原文，如需请从 ST 官方发布包获取

## Start/ —— 启动文件与内核文件

该目录混合了两个来源，需分开看。

### ST 部分（11 个文件）

- `startup_stm32f10x_*.s`（8 个）、`stm32f10x.h`、`system_stm32f10x.c`、`system_stm32f10x.h`
- 版权：© 2011 STMicroelectronics（MCD Application Team）
- 许可：同 `Library/`，MCD-ST Liberty SW License Agreement V2

### ARM 部分（2 个文件）

- `core_cm3.c`、`core_cm3.h` —— CMSIS Cortex-M3 Core Peripheral Access Layer，V1.30（2009-10-30）
- 版权：© 2009 ARM Limited. All rights reserved.
- 许可：随文件附带的 ARM 声明。**注意其中的分发范围限制**：

  > This file can be freely distributed within development tools that are supporting such ARM based processors.

  即允许随**支持 ARM 处理器的开发工具**自由分发。
- 该文件**不是** Apache-2.0 —— Apache-2.0 是 CMSIS 后续版本才采用的许可。

## 说明

- 以上第三方文件**均保留了原始的版权声明与免责声明**，未作删除或替换（已逐文件核对）。
- 本项目对第三方代码的使用方式是**引用编译**，未对其作修改后再分发。
- 注意：上述许可的**协议原文不在本仓库内**，仅在各文件头部有版权与免责声明。
