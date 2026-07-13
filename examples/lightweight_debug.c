/**
 * @file lightweight_debug.c
 * @brief 轻量级循迹 + 最小化串口调试
 * @author kokona23
 */

#include "ti_msp_dl_config.h"
#include "motor.h"
#include "line_sensor.h"
#include <stdio.h>

/* 串口发送（阻塞式，简单可靠）*/
static void UART_SendChar(char c)
{
    DL_UART_transmitDataBlocking(UART0_INST, c);
}

static void UART_SendString(const char *str)
{
    while (*str) {
        UART_SendChar(*str++);
    }
}

/* ==================== 配置参数 ==================== */

static struct {
    int16_t baseSpeedStraight;
    int16_t baseSpeedCorner;
    int16_t Kp;  // 已放大100倍
    int16_t Kd;  // 已放大100倍
} g_config = {
    .baseSpeedStraight = 500,  // 保守起步
    .baseSpeedCorner = 350,
    .Kp = 3000,  // 30.0
    .Kd = 1500   // 15.0
};

/* 控制状态 */
static struct {
    int16_t error;
    int16_t lastError;
    int16_t leftSpeed;
    int16_t rightSpeed;
    uint16_t loopCount;
} g_state = {0};

/* ==================== 辅助函数 ==================== */

static inline int16_t Clamp(int32_t value, int16_t min, int16_t max)
{
    if (value > max) return max;
    if (value < min) return min;
    return (int16_t)value;
}

static inline int16_t FastAbs(int16_t x)
{
    return (x < 0) ? -x : x;
}

/* ==================== 简化PD控制 ==================== */

static void UpdateControl(void)
{
    /* 读取传感器 */
    uint16_t sensorMask = LineSensor_ReadMask();
    int16_t error;
    bool lineDetected = LineSensor_GetError(sensorMask, &error);
    
    if (!lineDetected) {
        /* 丢线：原地转向搜索 */
        if (g_state.lastError < 0) {
            Motor_SetSpeed(-200, 200);
        } else {
            Motor_SetSpeed(200, -200);
        }
        return;
    }
    
    g_state.error = error;
    
    /* PD计算（整数运算）*/
    int32_t P = (int32_t)g_config.Kp * error;
    int32_t D = (int32_t)g_config.Kd * (error - g_state.lastError);
    int16_t correction = (int16_t)((P + D) / 100);
    
    /* 速度计算（大误差=弯道，降速）*/
    int16_t baseSpeed = (FastAbs(error) > 200) ? 
                        g_config.baseSpeedCorner : g_config.baseSpeedStraight;
    
    g_state.leftSpeed  = Clamp(baseSpeed + correction, 200, 850);
    g_state.rightSpeed = Clamp(baseSpeed - correction, 200, 850);
    
    /* 驱动电机 */
    Motor_SetSpeed(g_state.leftSpeed, g_state.rightSpeed);
    
    g_state.lastError = error;
}

/* ==================== 串口命令处理（简化版）==================== */

static void ProcessCommand(char cmd)
{
    char buf[64];
    
    switch (cmd) {
        case 'h':
            UART_SendString("\r\n命令: h=帮助 p=参数 s=状态 1/2=速度 3/4=Kp 5/6=Kd 空格=停\r\n");
            break;
            
        case 'p':
            sprintf(buf, "\r\n速度:%d/%d Kp:%d Kd:%d\r\n", 
                    g_config.baseSpeedStraight, g_config.baseSpeedCorner,
                    g_config.Kp, g_config.Kd);
            UART_SendString(buf);
            break;
            
        case 's':
            sprintf(buf, "E:%d L:%d R:%d\r\n", 
                    g_state.error, g_state.leftSpeed, g_state.rightSpeed);
            UART_SendString(buf);
            break;
            
        case '1':
            g_config.baseSpeedStraight -= 50;
            g_config.baseSpeedCorner -= 30;
            sprintf(buf, "速度降低: %d/%d\r\n", 
                    g_config.baseSpeedStraight, g_config.baseSpeedCorner);
            UART_SendString(buf);
            break;
            
        case '2':
            g_config.baseSpeedStraight += 50;
            g_config.baseSpeedCorner += 30;
            sprintf(buf, "速度提高: %d/%d\r\n", 
                    g_config.baseSpeedStraight, g_config.baseSpeedCorner);
            UART_SendString(buf);
            break;
            
        case '3':
            g_config.Kp += 500;  // +5.0
            sprintf(buf, "Kp=%d\r\n", g_config.Kp);
            UART_SendString(buf);
            break;
            
        case '4':
            g_config.Kp -= 500;
            sprintf(buf, "Kp=%d\r\n", g_config.Kp);
            UART_SendString(buf);
            break;
            
        case '5':
            g_config.Kd += 500;
            sprintf(buf, "Kd=%d\r\n", g_config.Kd);
            UART_SendString(buf);
            break;
            
        case '6':
            g_config.Kd -= 500;
            sprintf(buf, "Kd=%d\r\n", g_config.Kd);
            UART_SendString(buf);
            break;
            
        case ' ':
            Motor_Stop();
            UART_SendString("停止\r\n");
            while(1);
            break;
    }
}

/* ==================== 主函数 ==================== */

int main(void)
{
    SYSCFG_DL_init();
    Motor_Init();
    
    UART_SendString("\r\n=== 轻量级循迹调试 ===\r\n");
    UART_SendString("按 h 查看命令\r\n\r\n");
    
    while (1) {
        /* 检查串口命令 */
        if (!DL_UART_isRXFIFOEmpty(UART0_INST)) {
            char cmd = DL_UART_receiveData(UART0_INST);
            ProcessCommand(cmd);
        }
        
        /* 控制更新 */
        UpdateControl();
        
        /* 简化的调试输出：每500ms输出一次（降低CPU负担）*/
        if ((g_state.loopCount % 100) == 0) {
            char buf[32];
            sprintf(buf, "E:%4d L:%3d R:%3d\r\n", 
                    g_state.error, g_state.leftSpeed, g_state.rightSpeed);
            UART_SendString(buf);
        }
        
        g_state.loopCount++;
        
        /* 5ms控制周期 */
        delay_cycles((CPUCLK_FREQ / 1000U) * 5);
    }
}
