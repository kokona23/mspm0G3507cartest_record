/**
 * @file lightweight_tracking.c
 * @brief 轻量级循迹控制 - 针对 MSPM0G3507 优化
 * @author kokona23
 * @note 避免浮点运算，使用定点数和查表法
 */

#include "ti_msp_dl_config.h"
#include "motor.h"
#include "line_sensor.h"

/* ==================== 配置参数（整数运算）==================== */

/* 速度参数 (PWM 占空比: 0~1000) */
#define BASE_SPEED_STRAIGHT     600     
#define BASE_SPEED_CORNER       400     
#define MAX_SPEED               850     
#define MIN_SPEED               200     

/* PID 参数（已放大100倍，避免小数）*/
#define KP_STRAIGHT             3000    // 实际 30.00
#define KD_STRAIGHT             1500    // 实际 15.00

#define KP_CORNER               5000    // 实际 50.00  
#define KD_CORNER               3000    // 实际 30.00

/* 加速度限制 */
#define MAX_ACCELERATION        50      

/* 误差滤波（定点数：0~255 表示 0.0~1.0）*/
#define ERROR_FILTER_SHIFT      6       // 右移6位 = 除以64
#define ERROR_FILTER_ALPHA      42      // 42/64 ≈ 0.656

/* 控制周期 */
#define CONTROL_PERIOD_MS       5       // 5ms (200Hz，M0+足够)

/* 丢线处理 */
#define LOST_STOP_COUNT         40      // 40*5ms = 200ms后停止

/* ==================== 状态变量 ==================== */

typedef struct {
    int16_t error;              // 当前误差
    int16_t lastError;          // 上次误差
    int16_t filteredError;      // 滤波后误差（放大64倍）
    
    int16_t leftSpeed;          // 左侧目标速度
    int16_t rightSpeed;         // 右侧目标速度
    
    int16_t leftCurrentSpeed;   // 左侧当前速度（用于平滑）
    int16_t rightCurrentSpeed;  // 右侧当前速度
    
    uint16_t lostCount;         // 丢线计数
    
} ControlState_t;

static ControlState_t g_state = {0};

/* ==================== 辅助函数（优化为宏/内联）==================== */

/**
 * @brief 限幅（编译器会内联）
 */
static inline int16_t Clamp(int32_t value, int16_t min, int16_t max)
{
    if (value > max) return max;
    if (value < min) return min;
    return (int16_t)value;
}

/**
 * @brief 快速绝对值
 */
static inline int16_t FastAbs(int16_t x)
{
    return (x < 0) ? -x : x;
}

/**
 * @brief 定点数低通滤波（避免浮点）
 * @param input 新输入（原始值 * 64）
 * @param last 上次输出（原始值 * 64）
 * @return 滤波结果（原始值 * 64）
 */
static inline int16_t FixedPointFilter(int16_t input, int16_t last)
{
    // output = alpha * input + (1 - alpha) * last
    // 使用定点数：alpha = 42/64
    int32_t result = ((int32_t)ERROR_FILTER_ALPHA * input + 
                      (64 - ERROR_FILTER_ALPHA) * last) >> ERROR_FILTER_SHIFT;
    return (int16_t)result;
}

/**
 * @brief 速度平滑过渡（限制加速度）
 */
static inline int16_t SmoothSpeed(int16_t target, int16_t current)
{
    int16_t diff = target - current;
    
    if (diff > MAX_ACCELERATION) {
        return current + MAX_ACCELERATION;
    } else if (diff < -MAX_ACCELERATION) {
        return current - MAX_ACCELERATION;
    }
    
    return target;
}

/* ==================== 核心控制算法 ==================== */

/**
 * @brief 简化PD控制器（纯整数运算）
 * @param error 当前位置误差
 * @return 控制输出（左右速度差）
 */
static int16_t SimplePD(int16_t error)
{
    int16_t Kp, Kd;
    int16_t baseSpeed;
    
    /* 简化弯道检测：大误差 = 弯道 */
    bool isCorner = (FastAbs(error) > 200);
    
    if (isCorner) {
        Kp = KP_CORNER;
        Kd = KD_CORNER;
        baseSpeed = BASE_SPEED_CORNER;
    } else {
        Kp = KP_STRAIGHT;
        Kd = KD_STRAIGHT;
        baseSpeed = BASE_SPEED_STRAIGHT;
    }
    
    /* 误差滤波（输入放大64倍）*/
    int16_t errorScaled = error << ERROR_FILTER_SHIFT;
    g_state.filteredError = FixedPointFilter(errorScaled, g_state.filteredError);
    int16_t smoothError = g_state.filteredError >> ERROR_FILTER_SHIFT;
    
    /* PD计算（所有参数已放大100倍）*/
    int32_t proportional = (int32_t)Kp * smoothError;
    int32_t derivative = (int32_t)Kd * (error - g_state.lastError);
    
    /* 除以100恢复真实值 */
    int16_t correction = (int16_t)((proportional + derivative) / 100);
    
    return correction;
}

/**
 * @brief 差速控制
 */
static void DifferentialControl(int16_t correction)
{
    /* 简化版：不考虑弯道，统一用直道速度 */
    int16_t baseSpeed = BASE_SPEED_STRAIGHT;
    
    /* 如果误差很大，降低速度 */
    if (FastAbs(g_state.error) > 200) {
        baseSpeed = BASE_SPEED_CORNER;
    }
    
    /* 计算目标速度 */
    int16_t targetLeft  = Clamp(baseSpeed + correction, MIN_SPEED, MAX_SPEED);
    int16_t targetRight = Clamp(baseSpeed - correction, MIN_SPEED, MAX_SPEED);
    
    /* 速度平滑 */
    g_state.leftSpeed  = SmoothSpeed(targetLeft,  g_state.leftCurrentSpeed);
    g_state.rightSpeed = SmoothSpeed(targetRight, g_state.rightCurrentSpeed);
    
    /* 更新当前速度 */
    g_state.leftCurrentSpeed  = g_state.leftSpeed;
    g_state.rightCurrentSpeed = g_state.rightSpeed;
}

/* ==================== 主控制循环 ==================== */

/**
 * @brief 初始化
 */
void TrackingControl_Init(void)
{
    SYSCFG_DL_init();
    Motor_Init();
    
    /* 初始化状态 */
    g_state.leftCurrentSpeed = BASE_SPEED_STRAIGHT;
    g_state.rightCurrentSpeed = BASE_SPEED_STRAIGHT;
    g_state.filteredError = 0;
    g_state.lostCount = 0;
}

/**
 * @brief 主控制循环
 */
void TrackingControl_Update(void)
{
    /* 读取循迹传感器 */
    const uint16_t sensorMask = LineSensor_ReadMask();
    int16_t error;
    bool lineDetected = LineSensor_GetError(sensorMask, &error);
    
    if (lineDetected) {
        /* 找到线 */
        g_state.error = error;
        
        /* PD控制 */
        int16_t correction = SimplePD(error);
        
        /* 差速控制 */
        DifferentialControl(correction);
        
        /* 驱动电机 */
        Motor_SetSpeed(g_state.leftSpeed, g_state.rightSpeed);
        
        /* 更新历史 */
        g_state.lastError = error;
        
        /* 重置丢线计数 */
        g_state.lostCount = 0;
        
    } else {
        /* 丢线处理 */
        if (g_state.lostCount < LOST_STOP_COUNT) {
            /* 根据上次误差方向搜索 */
            if (g_state.lastError < 0) {
                Motor_SetSpeed(-200, 200);  // 左转搜索
            } else {
                Motor_SetSpeed(200, -200);  // 右转搜索
            }
            g_state.lostCount++;
        } else {
            /* 超时停止 */
            Motor_Stop();
        }
    }
}

/**
 * @brief 主函数
 */
int main(void)
{
    TrackingControl_Init();
    
    while (1) {
        TrackingControl_Update();
        
        /* 5ms控制周期 */
        delay_cycles((CPUCLK_FREQ / 1000U) * CONTROL_PERIOD_MS);
    }
}
