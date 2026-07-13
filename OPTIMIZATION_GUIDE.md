# MSPM0G3507 性能优化说明

## ⚠️ 硬件限制

| 参数 | 规格 | 影响 |
|------|------|------|
| CPU | Cortex-M0+, 32MHz | 没有硬件乘法器和除法器 |
| FPU | **无** | 浮点运算极慢（软件模拟） |
| RAM | 32KB | 有限的栈和堆空间 |
| Flash | 128KB | 代码空间充足 |

## 🚫 应该避免的操作

### 1. 浮点运算
```c
// ❌ 避免
float angle = atan2(y, x);           // 超慢！
float filtered = 0.7 * input + 0.3 * last;  // 软件模拟浮点

// ✅ 改用定点数
int16_t filtered = (42 * input + 22 * last) >> 6;  // 整数运算
```

### 2. 三角函数
```c
// ❌ 避免
float sin_val = sinf(angle);         // 超级慢！

// ✅ 改用查表法
const int16_t sin_table[360] = {...};
int16_t sin_val = sin_table[angle];
```

### 3. 开方运算
```c
// ❌ 避免
float distance = sqrtf(x*x + y*y);   // 很慢

// ✅ 改用平方比较
if (x*x + y*y < threshold*threshold) { ... }
```

### 4. 除法运算
```c
// ❌ 慢
int result = value / 100;

// ✅ 快（如果除数是2的幂）
int result = value >> 6;  // 除以 64

// ✅ 快（预先乘倒数）
// 计算 value / 100 ≈ value * 0.01 ≈ value * 41 / 4096
int result = (value * 41) >> 12;
```

### 5. printf 大量使用
```c
// ❌ 避免在控制循环中频繁调用
while(1) {
    printf("E:%d L:%d R:%d\n", error, left, right);  // 每次都很慢
    delay_ms(5);
}

// ✅ 降低输出频率
if ((counter % 100) == 0) {  // 每500ms输出一次
    printf("E:%d L:%d R:%d\n", error, left, right);
}
```

---

## ✅ 优化后的代码特点

### 1. 纯整数运算
```c
// PID参数放大100倍存储
#define KP  3000    // 实际 30.0
#define KD  1500    // 实际 15.0

// 计算时用整数
int32_t P = KP * error;           // 整数乘法
int32_t D = KD * (error - last);  
int16_t output = (P + D) / 100;   // 最后除法恢复
```

### 2. 定点数滤波
```c
// 低通滤波：output = 0.656 * input + 0.344 * last
// 转换为：output = (42*input + 22*last) / 64

#define ALPHA 42
int16_t filtered = ((ALPHA * input + (64 - ALPHA) * last) >> 6);
```

### 3. 快速绝对值
```c
// 避免调用 abs() 库函数
static inline int16_t FastAbs(int16_t x) {
    return (x < 0) ? -x : x;
}
```

### 4. 内联函数
```c
// 小函数用 inline 减少函数调用开销
static inline int16_t Clamp(int32_t value, int16_t min, int16_t max) {
    if (value > max) return max;
    if (value < min) return min;
    return (int16_t)value;
}
```

### 5. 简化算法
```c
// ❌ 之前：复杂弯道检测（陀螺仪+卡尔曼滤波）
float gyroZ = ICM42688_Read().gyroZDps;
isCorner = KalmanFilter(gyroZ) > threshold;

// ✅ 现在：简单误差判断
isCorner = (FastAbs(error) > 200);  // 大误差=弯道
```

---

## 📊 性能对比

| 操作 | 优化前耗时 | 优化后耗时 | 提升 |
|------|-----------|-----------|------|
| 浮点PID | ~500 cycles | ~50 cycles | **10倍** |
| 浮点滤波 | ~300 cycles | ~20 cycles | **15倍** |
| 陀螺仪读取+滤波 | ~1000 cycles | 不使用陀螺仪 | **省略** |
| printf输出 | ~5000 cycles | 每500ms一次 | **100倍** |
| **总控制周期** | **10ms勉强** | **2-5ms轻松** | **2-5倍** |

---

## 🎯 三个版本对比

| 版本 | 文件名 | 特点 | 适用场景 |
|------|--------|------|---------|
| **轻量级（推荐）** | `lightweight_tracking.c` | 纯整数运算<br>5ms控制周期<br>不使用陀螺仪 | **正式比赛**<br>基本要求+发挥(1)(2) |
| **轻量级调试** | `lightweight_debug.c` | 轻量级+串口调参<br>最小化输出 | **参数调试**<br>性能测试 |
| **增强版（不推荐）** | `enhanced_tracking.c` | 浮点运算<br>陀螺仪融合<br>卡尔曼滤波 | **仅供参考**<br>性能不足 |

---

## 🔧 推荐配置

### 基本要求阶段（速度优先）
```c
BASE_SPEED_STRAIGHT = 600
BASE_SPEED_CORNER = 400
KP = 3000  (30.0)
KD = 1500  (15.0)
CONTROL_PERIOD_MS = 5
```

### 发挥部分(3)（稳定性优先）
```c
BASE_SPEED_STRAIGHT = 450  // 降低速度
BASE_SPEED_CORNER = 350
KP = 3500  (35.0)          // 增强响应
KD = 2000  (20.0)
CONTROL_PERIOD_MS = 5
```

---

## 🚀 进一步优化建议

### 1. 如果还需要陀螺仪（发挥部分3）

**简化版陀螺仪使用**：
```c
// ✅ 只读Z轴，不做复杂滤波
int16_t gyroZ = ICM42688_ReadGyroZ_Fast();  // 单轴读取

// 简单限幅即可
if (gyroZ > 500) gyroZ = 500;
if (gyroZ < -500) gyroZ = -500;

// 直接用于修正
correction -= (gyroZ >> 4);  // 除以16
```

### 2. 编码器闭环（如果需要）

```c
// ✅ 简单的速度PID
int16_t speedError = targetSpeed - actualSpeed;
static int16_t lastSpeedError = 0;

int16_t speedCorrection = (20 * speedError + 10 * (speedError - lastSpeedError)) / 10;
motorPWM += speedCorrection;

lastSpeedError = speedError;
```

### 3. 位置估计（发挥部分3）

```c
// ✅ 简化的里程计（避免三角函数）
// 假设小车主要沿轨迹运动，使用近似
static int32_t distance = 0;  // 累积距离（单位：mm）

// 每个控制周期累加
int16_t avgSpeed = (leftSpeed + rightSpeed) / 2;
distance += avgSpeed * 5;  // 5ms周期

// 根据距离判断位置（100cm轨迹 = 400cm周长）
uint8_t position = (distance / 10) % 400;  // 0~399 cm
```

---

## 📝 调试技巧

### 1. 使用GPIO输出观察时序
```c
// 用示波器观察控制周期
DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_0);   // 设置高电平
UpdateControl();  // 执行控制
DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_0); // 设置低电平
```

### 2. 统计CPU占用率
```c
uint32_t start = DL_Timer_getCaptureCompareValue(TIMER_0_INST);
UpdateControl();
uint32_t end = DL_Timer_getCaptureCompareValue(TIMER_0_INST);
uint32_t cycles = end - start;

// 控制周期5ms = 160000 cycles (32MHz)
// CPU占用率 = cycles / 160000 * 100%
```

### 3. 简化版串口输出（二进制）
```c
// 用二进制发送数据（比printf快10倍）
struct {
    int16_t error;
    int16_t leftSpeed;
    int16_t rightSpeed;
} __attribute__((packed)) data;

data.error = error;
data.leftSpeed = leftSpeed;
data.rightSpeed = rightSpeed;

// 发送6字节
DL_UART_transmitDataBlocking(UART0_INST, (uint8_t*)&data, 6);

// 上位机解析二进制数据
```

---

## ⚡ 最终建议

1. **基本要求和发挥(1)(2)**：使用 `lightweight_tracking.c`
2. **参数调试**：使用 `lightweight_debug.c`
3. **发挥(3)画圆**：需要额外实现位置估计和云台同步，但仍然避免浮点运算

**核心原则**：
- ✅ 整数运算
- ✅ 查表法
- ✅ 简化算法
- ✅ 降低输出频率
- ❌ 避免浮点
- ❌ 避免三角函数
- ❌ 避免频繁串口输出
