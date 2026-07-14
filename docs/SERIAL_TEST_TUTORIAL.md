# 完整串口测试教程

## 📋 目录

1. [准备工作](#第一步准备工作)
2. [安装CH340驱动](#第二步安装ch340驱动)
3. [准备测试程序](#第三步准备测试程序)
4. [编译和烧录](#第四步编译和烧录)
5. [连接串口工具](#第五步连接串口工具)
6. [测试功能](#第六步测试功能)
7. [常见问题](#第七步常见问题)

---

## 第一步：准备工作

### 1.1 检查硬件

- [ ] MSPM0G3507 核心板 × 1
- [ ] USB数据线（支持数据传输） × 1
- [ ] 电脑（Windows 10/11）× 1

### 1.2 检查软件环境

打开 VSCode，确认已安装：
- [ ] VSCode
- [ ] EIDE 插件
- [ ] ARM Compiler 6 或 GCC
- [ ] OpenOCD（用于烧录）

检查方法：
```
1. 打开 VSCode
2. 查看左侧是否有 EIDE 图标（🔧）
3. Ctrl+Shift+P → 输入 "EIDE"，看是否有相关命令
```

---

## 第二步：安装CH340驱动

### 2.1 为什么需要驱动？

CH340 是 USB 转串口芯片，Windows 需要安装驱动才能识别。

### 2.2 检查是否已安装驱动

**步骤**：

1. **插入USB线**，连接核心板到电脑

2. **打开设备管理器**
   - 按 `Win + X`
   - 选择 "设备管理器"

3. **查看端口**
   - 展开 "端口 (COM 和 LPT)"
   
4. **判断情况**：

   **情况A：已安装驱动** ✅
   ```
   端口 (COM 和 LPT)
   ├─ USB-SERIAL CH340 (COM3)  ← 看到这个
   ```
   **记住端口号（如 COM3），跳到第三步**

   **情况B：未安装驱动** ❌
   ```
   其他设备
   ├─ USB2.0-Serial  ← 黄色感叹号
   ```
   **继续下面的安装步骤**

### 2.3 下载并安装驱动

**方法一：自动安装（推荐）**

1. 访问：http://www.wch.cn/downloads/CH341SER_EXE.html
2. 下载 `CH341SER.EXE`
3. 双击运行，点击 "安装"
4. 等待提示 "驱动安装成功"
5. **重新插拔USB线**

**方法二：手动安装**

1. 右键 "USB2.0-Serial" → 更新驱动程序
2. 选择 "浏览我的电脑..."
3. 选择 "让我从计算机上的可用驱动程序列表中选取"
4. 选择 "端口 (COM 和 LPT)"
5. 厂商选择 "WCH"，型号选择 "USB-SERIAL CH340"
6. 点击 "下一步" 安装

### 2.4 验证安装

重新打开设备管理器，应该看到：
```
端口 (COM 和 LPT)
├─ USB-SERIAL CH340 (COM3)  ← 成功！
```

**记住你的端口号，例如：COM3**

---

## 第三步：准备测试程序

### 3.1 备份原程序

打开 VSCode，在项目目录下执行：

**方式一：使用终端**
```powershell
# 在 VSCode 中按 Ctrl + `（反引号）打开终端

# 切换到 src 目录
cd C:\Users\35336\Desktop\desk\2026diansai\project\test\NewProject1\src

# 备份原 main.c
ren main.c main_backup.c
```

**方式二：手动操作**
```
1. 打开文件资源管理器
2. 进入：C:\Users\35336\Desktop\desk\2026diansai\project\test\NewProject1\src
3. 找到 main.c
4. 右键 → 重命名 → main_backup.c
```

### 3.2 使用串口测试程序

**方式一：使用终端**
```powershell
# 将 serial_test.c 重命名为 main.c
ren serial_test.c main.c
```

**方式二：手动操作**
```
1. 在 src 目录找到 serial_test.c
2. 右键 → 重命名 → main.c
```

### 3.3 验证文件

确认 `src` 目录下现在有：
- [x] main.c（串口测试程序）
- [x] main_backup.c（原程序备份）
- [x] motor.c
- [x] line_sensor.c
- [x] lightweight_tracking.c
- [x] lightweight_debug.c

---

## 第四步：编译和烧录

### 4.1 打开项目

1. 打开 VSCode
2. 文件 → 打开文件夹
3. 选择：`C:\Users\35336\Desktop\desk\2026diansai\project\test\NewProject1`
4. 或者直接双击 `NewProject1.code-workspace`

### 4.2 编译项目

**方法一：使用 VSCode 任务**

1. 按 `Ctrl + Shift + P`
2. 输入 `Tasks: Run Task`
3. 选择 `build`
4. 等待编译完成

**方法二：使用 EIDE 插件**

1. 点击左侧 EIDE 图标
2. 在项目树中找到 "构建" 按钮（🔨）
3. 点击构建

**编译成功的标志**：

终端输出类似：
```
Build finished
Program Size: Code=xxxx RO-data=xxxx RW-data=xxxx ZI-data=xxxx
✓ Build Success!
```

**如果编译失败**：
- 检查是否缺少 `ti_msp_dl_config.h`
- 运行 SysConfig 生成配置文件（`Tasks: Run Task` → `sysconfig: generate`）

### 4.3 连接烧录器

**重要**：确保核心板已通过 USB 连接到电脑

检查连接：
1. 设备管理器中能看到 CH340 串口
2. 核心板上的电源指示灯亮起

### 4.4 烧录程序

**方法一：使用 VSCode 任务（推荐）**

1. 按 `Ctrl + Shift + P`
2. 输入 `Tasks: Run Task`
3. 选择 `flash`
4. 等待烧录完成

**方法二：一键编译+烧录**

1. 按 `Ctrl + Shift + P`
2. 输入 `Tasks: Run Task`
3. 选择 `build and flash`

**烧录成功的标志**：

终端输出类似：
```
Open On-Chip Debugger 0.12.0
...
** Programming Started **
** Programming Finished **
** Verify Started **
** Verified OK **
shutdown command invoked
```

**如果烧录失败**：

常见错误：
```
Error: Can't find a board config
```
**解决**：检查 OpenOCD 配置文件路径

```
Error: No device found
```
**解决**：
1. 重新插拔USB
2. 检查是否有CMSIS-DAP调试器连接
3. 如果用的是DAPLink，确认驱动正常

---

## 第五步：连接串口工具

烧录完成后，程序会自动运行。现在需要串口工具来查看输出。

### 5.1 方法一：使用 VSCode 串口监视器（推荐）

#### 安装插件

1. 按 `Ctrl + Shift + X` 打开扩展
2. 搜索 `Serial Monitor`
3. 安装 "Serial Monitor" 插件（作者：Microsoft）
4. 重启 VSCode

#### 打开串口

1. 按 `Ctrl + Shift + P`
2. 输入 `Serial Monitor: Focus on Serial Monitor View`
3. 或者点击左侧活动栏的串口图标

#### 配置连接

在串口监视器面板：

1. **选择端口**：
   - 点击 "Select Port"
   - 选择你的端口（如 COM3）

2. **设置波特率**：
   - 点击波特率下拉框
   - 选择 `115200`

3. **点击 "Start Monitoring"**

**如果成功，你会看到**：
```
========================================
  MSPM0G3507 串口测试程序
  波特率: 115200
  CPU频率: 32MHz
========================================

系统启动成功！
发送任意字符进行回显测试

[心跳] 运行时间: 0 秒
[心跳] 运行时间: 1 秒
[心跳] 运行时间: 2 秒
...
```

---

### 5.2 方法二：使用 PuTTY

#### 下载安装

1. 访问：https://www.putty.org/
2. 下载 `putty-64bit-0.xx-installer.msi`
3. 双击安装

#### 配置连接

1. 打开 PuTTY
2. 左侧选择 "Session"
3. 配置参数：
   ```
   Connection type: Serial
   Serial line: COM3  （你的端口号）
   Speed: 115200
   ```
4. 点击 "Open"

**成功标志**：黑色窗口显示串口输出

#### PuTTY 设置优化

配置完善的显示：

1. **终端设置**：
   - Category → Terminal
   - ✓ Implicit CR in every LF（自动换行）

2. **字体设置**：
   - Category → Window → Appearance
   - Font: Consolas, 10-point

3. **保存配置**：
   - 回到 Session
   - Saved Sessions 输入：MSPM0 Serial
   - 点击 Save

---

### 5.3 方法三：使用 Arduino IDE 串口监视器

如果你装了 Arduino IDE：

1. 打开 Arduino IDE
2. 工具 → 串口监视器（Ctrl + Shift + M）
3. 右下角设置：
   - 波特率：115200
   - 换行：CR+LF 或 Both NL & CR
4. 端口：选择 COM3

---

### 5.4 方法四：使用串口调试助手（推荐新手）

#### 下载工具

搜索 "串口调试助手" 或使用：
- XCOM（推荐，界面简洁）
- 格西烽火串口助手
- SSCOM（免费，功能强大）

#### 配置步骤（以SSCOM为例）

1. 打开 SSCOM
2. 配置参数：
   ```
   端口号：COM3
   波特率：115200
   数据位：8
   停止位：1
   校验位：None
   ```
3. 点击 "打开串口"
4. 右侧接收窗口会显示数据

---

## 第六步：测试功能

### 6.1 观察启动信息

连接串口后，**按核心板上的复位按钮**，你应该看到：

```
========================================
  MSPM0G3507 串口测试程序
  波特率: 115200
  CPU频率: 32MHz
========================================

系统启动成功！
发送任意字符进行回显测试

[心跳] 运行时间: 0 秒
```

✅ **如果看到以上输出，说明串口工作正常！**

---

### 6.2 测试回显功能

在串口工具的**发送框**中输入任意字符，例如：`a`

**预期输出**：
```
收到: 'a' (0x61)
```

**测试更多字符**：
```
发送: hello
输出:
收到: 'h' (0x68)
收到: 'e' (0x65)
收到: 'l' (0x6C)
收到: 'l' (0x6C)
收到: 'o' (0x6F)
```

---

### 6.3 测试命令功能

发送 `h`（帮助命令）：

**预期输出**：
```
可用命令:
  h - 显示帮助
  v - 显示版本信息
  t - CPU温度测试
  i - 显示系统信息
  r - 软件复位
```

---

### 6.4 测试各个命令

#### 测试 v（版本信息）

**发送**：`v`

**预期输出**：
```
版本信息:
  项目: MSPM0G3507 小车调试记录
  版本: v0.1.0-test
  开发者: kokona23
```

#### 测试 i（系统信息）

**发送**：`i`

**预期输出**：
```
系统信息:
  CPU频率: 32000000 Hz
  运行计数: 12345
  串口: UART0 @ 115200 bps
```

#### 测试 r（软件复位）

**发送**：`r`

**预期输出**：
```
系统将在 3 秒后复位...
```

**3秒后芯片复位，重新显示启动信息**

---

## 第七步：常见问题

### 问题1：没有看到任何输出

**可能原因及解决**：

1. **端口号错误**
   - 检查：设备管理器确认端口号
   - 解决：重新选择正确的COM口

2. **波特率不对**
   - 检查：串口工具是否设置为 115200
   - 解决：修改为 115200

3. **程序未运行**
   - 检查：烧录是否成功
   - 解决：重新烧录，按复位按钮

4. **USB线问题**
   - 检查：换一根数据线
   - 解决：使用手机原装数据线

---

### 问题2：输出乱码

**显示类似**：
```
㊣㊣㊣㊣㊣㊣㊣㊣㊣
�����������
```

**原因**：波特率不匹配

**解决**：
1. 串口工具设置为 115200
2. 如果还乱码，检查代码中 UART 初始化

---

### 问题3：只有部分字符

**显示类似**：
```
========
  MSP
  
系统
```

**原因**：缓冲区溢出或串口工具问题

**解决**：
1. 关闭串口，重新打开
2. 清空接收缓冲区
3. 按复位按钮重新启动

---

### 问题4：设备管理器找不到CH340

**显示**：
- 端口 (COM 和 LPT) 下什么都没有
- 或者 "其他设备" 有黄色感叹号

**解决**：
1. **重新安装驱动**（参考第二步）
2. **换USB口**（有些USB 3.0口不兼容）
3. **检查USB线**（必须是数据线）
4. **重启电脑**

---

### 问题5：无法烧录程序

**错误提示**：
```
Error: Can't find a board config
```

**解决**：
1. 检查 `.vscode/tasks.json` 中 OpenOCD 配置路径
2. 确认 OpenOCD 已安装
3. 检查是否有调试器连接（DAPLink/J-Link）

---

### 问题6：编译错误

**错误提示**：
```
fatal error: ti_msp_dl_config.h: No such file or directory
```

**解决**：
1. 运行 SysConfig 生成配置：
   ```
   Ctrl + Shift + P → Tasks: Run Task → sysconfig: generate
   ```
2. 检查 `Debug/` 目录是否有生成的文件

---

## 第八步：高级测试

### 8.1 性能测试

修改 `serial_test.c`，添加性能测试：

```c
// 在 main 函数中添加
uint32_t startTime = DL_TimerG_getCaptureCompareValue(TIMER_0_INST);

// 执行1000次循环
for (int i = 0; i < 1000; i++) {
    counter++;
}

uint32_t endTime = DL_TimerG_getCaptureCompareValue(TIMER_0_INST);
uint32_t elapsed = endTime - startTime;

printf("1000次循环耗时: %lu cycles\n", elapsed);
```

### 8.2 GPIO测试

添加LED闪烁：

```c
// 假设 LED 连接到 PA0
while (1) {
    DL_GPIO_togglePins(GPIOA, DL_GPIO_PIN_0);
    UART_SendString("LED Toggle\r\n");
    delay_cycles(CPUCLK_FREQ / 2);  // 0.5秒
}
```

---

## 附录：完整测试检查清单

### 硬件检查
- [ ] USB线已连接
- [ ] 核心板电源灯亮
- [ ] 设备管理器能看到CH340

### 软件检查
- [ ] 驱动已安装（COM口正常）
- [ ] 代码已编译成功
- [ ] 程序已烧录成功

### 串口检查
- [ ] 端口号正确
- [ ] 波特率 115200
- [ ] 能看到启动信息
- [ ] 回显测试正常
- [ ] 命令测试正常

### 全部通过 ✅
恭喜！串口测试完成！

---

## 下一步

串口测试通过后，可以：
1. 测试循迹传感器
2. 测试电机驱动
3. ICM42688 底层驱动完成后再进行实机测试
4. 调试 PID 参数

需要帮助请参考：
- `TUNING_GUIDE.md` - 参数调试指南
- `OPTIMIZATION_GUIDE.md` - 性能优化指南
