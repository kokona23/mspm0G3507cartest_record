# MSPM0G3507 小车调试记录

基于 TI MSPM0G3507 (Cortex-M0+) 的四电机循迹小车开发项目。

## ⚠️ 项目状态：开发测试中

> **当前阶段**: 硬件配置和底层驱动开发  
> **测试进度**: 
> - ✅ MSPM0G3507 开发环境搭建完成
> - ✅ SysConfig 引脚配置完成（PWM、UART、SPI、I2C）
> - ⏳ ICM42688 SPI1 引脚已预留，底层驱动尚未接入
> - ✅ EIDE 工程配置和编译环境验证通过
> - 🔄 电机控制模块开发中
> - 🔄 循迹传感器接口调试中
> - ⏳ 编码器速度反馈功能待测试
> - ⏳ PID 控制算法待实现
> - ⏳ 整车联调待开始

**下一步计划**:
1. 完成电机 PWM 输出测试，验证 TB6612 驱动电路
2. 编写并接入 ICM42688 底层驱动，验证 SPI 通信和数据读取
3. 实现编码器脉冲计数和速度计算
4. 开发基础 PID 控制算法
5. 循迹传感器数据采集和处理

---

## 📋 项目概述

本项目为 2026 年电子设计竞赛准备，使用 MSPM0G3507 LQFP-48 封装芯片作为主控，实现四轮独立驱动的循迹小车。

### 硬件平台
- **主控芯片**: TI MSPM0G3507 (Cortex-M0+, 32MHz, 128KB Flash, 32KB RAM)
- **电机驱动**: TB6612FNG (4路电机 + PWM 控制)
- **传感器**:
  - ICM42688-P 六轴 IMU (陀螺仪 + 加速度计)
  - 编码器 x4 (速度反馈)
  - 循迹传感器模块
- **通信接口**: 
  - UART x3 (UART0 / UART1 / UART3，115200 bps)
  - SPI x1 (SPI1，1 MHz)
  - I2C x1 (100 kHz)

### 开发环境
- **IDE**: VSCode + Embedded IDE (EIDE)
- **工具链**: ARM Compiler 6 (AC6)
- **配置工具**: TI SysConfig 1.24.1
- **SDK**: MSPM0 SDK 2.05.01.00
- **调试器**: OpenOCD + CMSIS-DAP

## 🚀 快速开始

### 1. 环境准备

**必需软件**:
```
- VSCode
- EIDE 插件
- ARM Compiler 6
- TI MSPM0 SDK 2.05.01.00
- TI SysConfig 1.24.1
- OpenOCD (用于烧录和调试)
```

### 2. 克隆项目

```bash
git clone https://github.com/你的用户名/mspm0G3507小车调试记录.git
cd mspm0G3507小车调试记录
```

### 3. 打开项目

在 VSCode 中打开 `NewProject1.code-workspace` 工作区。

### 4. 编译

**方法一**: VSCode 任务
```
Ctrl+Shift+P → Tasks: Run Task → build
```

**方法二**: EIDE 插件
```
点击 EIDE 侧边栏 → 构建
```

### 5. 烧录

```
Ctrl+Shift+P → Tasks: Run Task → flash
```

或使用组合任务：
```
Ctrl+Shift+P → Tasks: Run Task → build and flash
```

## 📁 项目结构

```
NewProject1/
├── src/                          # 源文件
│   ├── main.c                   # 主程序
│   ├── motor.c                  # 电机控制
│   └── line_sensor.c            # 循迹传感器
├── inc/                          # 头文件
│   ├── motor.h
│   └── line_sensor.h
├── modules/                      # 外设驱动模块
│   └── ICM42688/                # ICM42688 预留接口 / 草稿
│       ├── README.md            # 接入状态说明
│       ├── inc/icm42688.h
│       └── src/icm42688.c
├── Debug/                        # SysConfig 生成文件
│   ├── ti_msp_dl_config.h       # 外设配置头文件
│   └── ti_msp_dl_config.c       # 外设配置实现
├── .eide/                        # EIDE 项目配置
│   └── eide.yml                 # 工程配置文件
├── .vscode/                      # VSCode 配置
│   ├── tasks.json               # 构建任务
│   ├── c_cpp_properties.json    # IntelliSense 配置
│   └── settings.json            # 编辑器设置
├── NewProject1.syscfg            # TI SysConfig 配置文件
├── WIRING_AND_SYSCONFIG.md       # 引脚定义和配置说明
├── mspm0g3507.sct                # Linker script
├── .gitignore                    # Git 忽略文件
└── README.md                     # 本文档
```

## 🔌 引脚配置

详细的引脚定义请查看 [WIRING_AND_SYSCONFIG.md](./WIRING_AND_SYSCONFIG.md)。

### ICM42688 陀螺仪 (SPI1)
| 功能 | GPIO | 位置 |
|------|------|------|
| SCLK | PA17 | H2-12 |
| MOSI | PA18 | H2-11 |
| MISO | PA16 | H2-13 |
| CS   | PB20 | H1-8  |

### 电机驱动 (TB6612)
| 电机 | PWM | IN1 | IN2 |
|------|-----|-----|-----|
| M1 | PA21 (TIMA0_CC0) | PB6 | PB7 |
| M2 | PA22 (TIMA0_CC1) | PB8 | PA7 |
| M3 | PA24 (TIMA1_CC1) | PA15 | PB9 |
| M4 | PA25 (TIMA0_CC3) | PB19 | PB24 |
| STBY | 硬件上拉至 3.3V | - | 不占用 MCU GPIO |

### 编码器
| 编码器 | A 相 | B 相 |
|--------|------|------|
| ENC1 | PA12 | PA13 |
| ENC2 | PA2 | PA26 |
| ENC3 | PA27 | PA28 |
| ENC4 | PA31 | PB18 |

## 🛠️ 修改 SysConfig

打开 SysConfig GUI：
```
Ctrl+Shift+P → Tasks: Run Task → sysconfig: open gui
```

修改后会自动重新生成 `Debug/ti_msp_dl_config.c` 和 `ti_msp_dl_config.h`。

## 📚 模块说明

### ICM42688 陀螺仪

SPI1、PB20 软件片选和接线已预留，但 ICM42688 底层驱动尚未完成验证，也未加入当前 EIDE 编译目标。现阶段不得在主程序中调用 ICM42688 API。

## 🐛 调试

### OpenOCD 调试配置

已配置 `.vscode/launch.json`，支持一键调试：
```
F5 启动调试
```

### 串口调试

UART0 已连接到板载 CH340，波特率 115200。

## 📝 开发日志

### 2026-07-12
- ⏳ 预留 ICM42688 SPI1 硬件接口
- ✅ 更新 SysConfig 配置（ICM42688 软件片选）
- ✅ 修复 EIDE 工程配置
- ✅ 添加 VSCode 任务配置

### 2026-07-10
- ✅ 完成电机控制模块
- ✅ 完成循迹传感器接口

### 2026-07-09
- ✅ 完成基础工程搭建
- ✅ 配置 SysConfig (PWM, UART, SPI, I2C)

## ⚠️ 注意事项

1. **芯片封装问题**: SysConfig 中选择 MSPM0G3507 会默认锁定 64-pin，但实际使用 48-pin。通过命令行参数 `--package "LQFP-48(PT)"` 强制指定。

2. **编译前自动生成**: 项目配置了 beforeBuildTasks，每次编译前会自动运行 SysConfig 生成配置文件。

3. **陀螺仪接入**: ICM42688 驱动完成并通过硬件验证后，再加入初始化和静态校准流程。

4. **引脚冲突**: 部分引脚已被板载外设占用（如 PB17 接 Flash CS），详见 WIRING_AND_SYSCONFIG.md。

## 📄 许可证

本项目仅用于学习和竞赛目的。

## 👥 贡献者

- 项目开发者：kokona23

## 🔗 相关链接

- [TI MSPM0 SDK 文档](https://www.ti.com/tool/MSPM0-SDK)
- [MSPM0G3507 数据手册](https://www.ti.com/product/MSPM0G3507)
- [ICM42688 数据手册](https://invensense.tdk.com/products/motion-tracking/6-axis/icm-42688-p/)
