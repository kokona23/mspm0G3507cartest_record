# MSPM0G3507 LQFP-48 引脚配置说明

本文件以 `NewProject1.syscfg` 和 SysConfig 生成的 `Debug/ti_msp_dl_config.h` 为准，已用 SysConfig 1.24.1 / MSPM0 SDK 2.05 验证生成。

配套总览图：[MSPM0G3507_PINOUT_SUMMARY.png](docs/MSPM0G3507_PINOUT_SUMMARY.png)。

## 配置总览

| 模块 | 当前配置 | 资源 |
|---|---|---|
| SPI | SPI1 Controller，1 MHz，Mode 0，8 bit，MOTO3 | PA16 / PA17 / PA18 + PB20 软件 CS；DMA_CH2 / DMA_CH3 |
| I2C | I2C0 Controller，100 kHz | PA0 / PA1 |
| UART | UART0、UART1、UART3，均为 115200 | 6 个 GPIO |
| 电机 | TB6612 四电机 | 4 PWM、8 DIR；STBY 由硬件上拉 |
| 编码器 | 四路正交编码器 | 8 路 GPIO 双边沿中断 |
| 调试 | SWD / NRST | PA19 / PA20 / NRST |

SPI0、UART2 未启用：SPI0 的 PA4/PA5/PA6 与核心板时钟网络共用；UART2 的 PB15/PB16 与板载 Flash 网络共用。

## 通信与调试

| 功能 | GPIO | 核心板位置 | PINCM / 复用 | 接线说明 |
|---|---|---|---|---|
| I2C0_SDA | PA0 | H1-1 | PINCM1 / I2C0_SDA | OLED 等 I2C 外设；总线使用 3.3 V 上拉 |
| I2C0_SCL | PA1 | H1-2 | PINCM2 / I2C0_SCL | OLED 等 I2C 外设 |
| SPI1_POCI | PA16 | H2-13 | PINCM38 / SPI1_POCI | 为后续 ICM42688 预留的 SDO / MISO |
| SPI1_SCLK | PA17 | H2-12 | PINCM39 / SPI1_SCLK | 为后续 ICM42688 预留的 SCLK |
| SPI1_PICO | PA18 | H2-11 | PINCM40 / SPI1_PICO | 为后续 ICM42688 预留的 SDI / MOSI；与板载 KEY 共用，SPI 工作时不得按 KEY |
| SPI1 软件 CS | PB20 | H1-8 | PINCM48 / GPIO 输出 | 为后续 ICM42688 预留；空闲高、低有效，原理图网络名 `SPI1_CS0` |
| UART0_TX / RX | PA10 / PA11 | 板载 CH340 | PINCM21 / PINCM22 | 调试串口 |
| UART1_TX / RX | PA8 / PA9 | H1-14 / H1-15 | PINCM19 / PINCM20 | Servo1 / Servo2 UART 接口 |
| UART3_TX / RX | PB2 / PB3 | H1-12 / H1-13 | PINCM15 / PINCM16 | RDK / 蓝牙通信接口 |
| SWDIO / SWCLK | PA19 / PA20 | 调试接口 | DEBUGSS | 保留给下载和调试 |
| NRST | NRST | H1-5 | 复位功能 | 保留复位按键和测试点 |

## TB6612 四电机

| 功能 | GPIO | 核心板位置 | PINCM / 方式 |
|---|---|---|---|
| M1_PWM | PA21 | H2-8 | TIMA0_CCP0 / PINCM46 |
| M2_PWM | PA22 | H2-7 | TIMA0_CCP1 / PINCM47 |
| M3_PWM | PA24 | H2-5 | TIMA1_CCP1 / PINCM54 |
| M4_PWM | PA25 | H2-4 | TIMA0_CCP3 / PINCM55 |
| M1_IN1 / M1_IN2 | PB6 / PB7 | H1-16 / H1-17 | GPIO / PINCM23 / PINCM24 |
| M2_IN1 / M2_IN2 | PB8 / PA7 | H2-10 / H1-11 | GPIO / PINCM25 / PINCM14 |
| M3_IN1 / M3_IN2 | PA15 / PB9 | H2-14 / H2-9 | GPIO / PINCM37 / PINCM26 |
| M4_IN1 / M4_IN2 | PB19 / PB24 | H1-9 / H1-7 | GPIO / PINCM45 / PINCM52 |
PWM 参数：32 MHz / 1000 = 32 kHz，初始占空比 0%。TB6612 的 STBY 由硬件上拉到 3.3 V，PA23 未在 SysConfig 中启用。

## 编码器输入

| 编码器 | A 相 | B 相 | 核心板位置 | 方式 |
|---|---|---|---|---|
| ENC1 | PA12 | PA13 | H2-17 / H2-16 | GPIO 双边沿中断 |
| ENC2 | PA2 | PA26 | H1-6 / H2-3 | GPIO 双边沿中断 |
| ENC3 | PA27 | PA28 | H2-2 / H1-3 | GPIO 双边沿中断 |
| ENC4 | PA31 | PB18 | H1-4 / H1-10 | GPIO 双边沿中断 |

`ENC2_A` 已从 PA14 移到 PA2，避开核心板板载 LED。扩展板应将 TB6612 左侧 H1-9 的 `ENC2_A` 连接到 U7/H5-6（PA2）；U8/H6-6 不再连接 `ENC2_A`，可保留为标注为 `PA14_LED_SHARED` 的备用引出脚。

`src/encoder.c` 提供 `GPIOA_IRQHandler` 和 `GPIOB_IRQHandler`：读取双相状态、按四倍频正交码表更新四组计数，并清除对应中断标志。主程序在 `SYSCFG_DL_init()` 后调用 `Encoder_Init()`，建立初始相位并使能两个 GPIO NVIC 中断。

## 扩展板原理图网络规则

- U7/H5-6：网络名为 `ENC2_A`，对应 PA2。
- U8/H6-6：不得标为 `ENC2_A`；若保留引出，使用 `PA14_LED_SHARED`，提示该脚连接板载 LED。
- U7/H5-8：网络名为 `SPI1_CS0`，对应 PB20 软件片选；不可误改为 `SPI1_CS`。
- U8/H6-10：网络名为 `SPI1_PICO`，对应 PA18；板载 KEY 不使用时可保持当前连接。
- 所有 MCU 信号均为 3.3 V 逻辑。5 V 只可作电源，不能直接接 MCU GPIO。
- ICM42688 目前仅保留硬件引脚和 SPI1 配置，尚未接入已验证的底层驱动与应用调用。

## 未启用或不可用的芯片引脚

| 引脚 | 原因 | 处理 |
|---|---|---|
| PA4 / PA5 / PA6 | 核心板晶振网络 | 不启用 SPI0，不接外设 |
| PB15 / PB16 / PB17 | 板载 Flash 网络 | 不启用 UART2，不接外设 |
| PA14 | 板载 LED | 不用于编码器；仅作为带 LED 负载的备用引出 |
| PA18 | 板载 KEY | 保留 SPI1_PICO；SPI 工作时不得按 KEY |
| PA19 / PA20 | SWD 调试 | 保留，不分配给外设 |

## 循迹模块

当前完整方案没有 12 路独立 GPIO 给数字循迹模块。`line_sensor.c` 在没有 `LINE_SENSOR` SysConfig 实例时会编译为安全空实现。若需要 12 路循迹，建议使用 `74HC165` 或 MCP23017 等扩展器，通过 SPI1 或 I2C0 读取。
