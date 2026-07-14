# ICM42688 预留接口

当前主工程只完成 ICM42688 的硬件引脚预留，尚未接入已验证的底层驱动。

## 预留接线

| 信号 | MSPM0G3507 | 核心板位置 |
|---|---|---|
| SCLK | PA17 | H2-12 |
| SDI / MOSI | PA18 | H2-11 |
| SDO / MISO | PA16 | H2-13 |
| CS | PB20 | H1-8 |

SPI1 已配置为 Controller、Mode 0、8 bit、MSB first、1 MHz，PB20 为低有效的软件片选。PA18 与核心板 KEY 共用，SPI 通信期间不得按 KEY。

`src/icm42688.c` 和 `inc/icm42688.h` 仅作为未验证草稿，未加入 EIDE 编译源目录，也不得由当前主程序调用。完成底层驱动、读写验证和实机校准后，再将其纳入构建与应用流程。
