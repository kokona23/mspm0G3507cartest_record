/**
 * @file main.c
 * @brief 仅用于硬件调试的测试程序。
 *
 * 本文件是临时测试入口，不是小车最终应用程序：只测试 UART0 调试串口、
 * ATK-BLE02 UART 回显和 TB6612 电机输出。正式的控制逻辑不得继续写在这里，
 * 后续应放入独立的应用模块。
 */

#include "atk_ble02.h"
#include "encoder.h"
#include "tb6612.h"
#include "ti_msp_dl_config.h"
#include <stdio.h>

#define MOTOR_TEST_SPEED       (400)
#define MOTOR_TEST_DURATION_MS (2000U)
#define BLUETOOTH_ECHO_BUFFER_SIZE 64U
#define BLUETOOTH_ECHO_IDLE_POLLS  20U

static uint8_t bluetoothEchoBuffer[BLUETOOTH_ECHO_BUFFER_SIZE];
static uint8_t bluetoothEchoLength;
static uint8_t bluetoothEchoIdlePolls;

/* UART0: PA10 = TX, PA11 = RX, 115200-8-N-1. */
static void UART_SendChar(char c)
{
    DL_UART_transmitDataBlocking(UART0_INST, c);
}

static void UART_SendString(const char *str)
{
    while (*str != '\0') {
        UART_SendChar(*str++);
    }
}

static void ShowHelp(void)
{
    UART_SendString("\r\n可用命令:\r\n");
    UART_SendString("  h - 显示帮助\r\n");
    UART_SendString("  v - 显示版本信息\r\n");
    UART_SendString("  t - CPU温度测试\r\n");
    UART_SendString("  i - 显示系统信息\r\n");
    UART_SendString("  r - 软件复位\r\n\r\n");
    UART_SendString("TB6612 电机测试（每次运行 2 秒后自动停止）:\r\n");
    UART_SendString("  1/2/3/4 - 对应电机正转\r\n");
    UART_SendString("  q/w/e/y - 对应电机反转\r\n");
    UART_SendString("  f/b     - 四路电机正转/反转\r\n");
    UART_SendString("  x       - 立即停止所有电机\r\n\r\n");
}

static void Motor_StartTest(int16_t m1Speed, int16_t m2Speed, int16_t m3Speed,
                            int16_t m4Speed, uint32_t *remainingMs)
{
    TB6612_SetStandby(true);
    TB6612_SetFour(m1Speed, m2Speed, m3Speed, m4Speed);
    *remainingMs = MOTOR_TEST_DURATION_MS;
}

/* 仅用于本测试程序的 UART 回显包装；ATK-BLE02 驱动本身保持通用。 */
static void Bluetooth_RunEchoTest(void)
{
    uint8_t byte;

    while (AtkBle02_ReadByte(&byte)) {
        if (bluetoothEchoLength < BLUETOOTH_ECHO_BUFFER_SIZE) {
            bluetoothEchoBuffer[bluetoothEchoLength++] = byte;
        }
        bluetoothEchoIdlePolls = 0U;
    }

    if (bluetoothEchoLength == 0U) {
        return;
    }
    if (bluetoothEchoIdlePolls < BLUETOOTH_ECHO_IDLE_POLLS) {
        bluetoothEchoIdlePolls++;
        return;
    }

    for (uint8_t index = 0U; index < bluetoothEchoLength; index++) {
        AtkBle02_SendByte(bluetoothEchoBuffer[index]);
    }
    bluetoothEchoLength = 0U;
    bluetoothEchoIdlePolls = 0U;
}

int main(void)
{
    uint32_t runtimeSeconds = 0U;
    uint32_t elapsedMs = 0U;
    uint32_t motorTestRemainingMs = 0U;
    char buffer[64];

    SYSCFG_DL_init();
    Encoder_Init();
    TB6612_Init();

    UART_SendString("\r\n");
    UART_SendString("========================================\r\n");
    UART_SendString("  MSPM0G3507 串口测试程序\r\n");
    UART_SendString("  波特率: 115200\r\n");
    UART_SendString("  CPU频率: 32MHz\r\n");
    UART_SendString("========================================\r\n\r\n");
    UART_SendString("系统启动成功！\r\n");
    UART_SendString("发送任意字符进行回显测试\r\n\r\n");
    AtkBle02_SendString("\r\nATK-BLE02 UART3 echo test ready\r\n");
    AtkBle02_SendString("Send text from the BLE app; this board will echo it.\r\n");

    sprintf(buffer, "[心跳] 运行时间: %u 秒\r\n", (unsigned int)runtimeSeconds);
    UART_SendString(buffer);

    while (1) {
        Bluetooth_RunEchoTest();

        if (!DL_UART_isRXFIFOEmpty(UART0_INST)) {
            uint8_t rxData = DL_UART_receiveData(UART0_INST);

            sprintf(buffer, "收到: '%c' (0x%02X)\r\n", (char)rxData, (unsigned int)rxData);
            UART_SendString(buffer);

            switch (rxData) {
                case 'h':
                case 'H':
                    ShowHelp();
                    break;
                case 'v':
                case 'V':
                    UART_SendString("\r\n版本信息:\r\n");
                    UART_SendString("  项目: MSPM0G3507 小车调试记录\r\n");
                    UART_SendString("  版本: v0.1.0-test\r\n");
                    UART_SendString("  开发者: kokona23\r\n\r\n");
                    break;
                case 't':
                case 'T':
                    UART_SendString("\r\nCPU温度测试未配置传感器，无法读取。\r\n\r\n");
                    break;
                case 'i':
                case 'I':
                    UART_SendString("\r\n系统信息:\r\n");
                    sprintf(buffer, "  CPU频率: %u Hz\r\n", (unsigned int)CPUCLK_FREQ);
                    UART_SendString(buffer);
                    sprintf(buffer, "  运行时间: %u 秒\r\n", (unsigned int)runtimeSeconds);
                    UART_SendString(buffer);
                    UART_SendString("  串口: UART0 @ 115200 bps\r\n\r\n");
                    break;
                case 'r':
                case 'R':
                    UART_SendString("\r\n系统将在 3 秒后复位...\r\n");
                    delay_cycles(CPUCLK_FREQ * 3U);
                    NVIC_SystemReset();
                    break;
                case '1':
                    Motor_StartTest(MOTOR_TEST_SPEED, 0, 0, 0, &motorTestRemainingMs);
                    UART_SendString("M1 正转测试\r\n");
                    break;
                case '2':
                    Motor_StartTest(0, MOTOR_TEST_SPEED, 0, 0, &motorTestRemainingMs);
                    UART_SendString("M2 正转测试\r\n");
                    break;
                case '3':
                    Motor_StartTest(0, 0, MOTOR_TEST_SPEED, 0, &motorTestRemainingMs);
                    UART_SendString("M3 正转测试\r\n");
                    break;
                case '4':
                    Motor_StartTest(0, 0, 0, MOTOR_TEST_SPEED, &motorTestRemainingMs);
                    UART_SendString("M4 正转测试\r\n");
                    break;
                case 'q':
                case 'Q':
                    Motor_StartTest(-MOTOR_TEST_SPEED, 0, 0, 0, &motorTestRemainingMs);
                    UART_SendString("M1 反转测试\r\n");
                    break;
                case 'w':
                case 'W':
                    Motor_StartTest(0, -MOTOR_TEST_SPEED, 0, 0, &motorTestRemainingMs);
                    UART_SendString("M2 反转测试\r\n");
                    break;
                case 'e':
                case 'E':
                    Motor_StartTest(0, 0, -MOTOR_TEST_SPEED, 0, &motorTestRemainingMs);
                    UART_SendString("M3 反转测试\r\n");
                    break;
                case 'y':
                case 'Y':
                    Motor_StartTest(0, 0, 0, -MOTOR_TEST_SPEED, &motorTestRemainingMs);
                    UART_SendString("M4 反转测试\r\n");
                    break;
                case 'f':
                case 'F':
                    Motor_StartTest(MOTOR_TEST_SPEED, MOTOR_TEST_SPEED, MOTOR_TEST_SPEED,
                                    MOTOR_TEST_SPEED, &motorTestRemainingMs);
                    UART_SendString("四路电机正转测试\r\n");
                    break;
                case 'b':
                case 'B':
                    Motor_StartTest(-MOTOR_TEST_SPEED, -MOTOR_TEST_SPEED, -MOTOR_TEST_SPEED,
                                    -MOTOR_TEST_SPEED, &motorTestRemainingMs);
                    UART_SendString("四路电机反转测试\r\n");
                    break;
                case 'x':
                case 'X':
                    TB6612_Stop();
                    motorTestRemainingMs = 0U;
                    UART_SendString("所有电机已停止\r\n");
                    break;
                default:
                    break;
            }
        }

        delay_cycles(CPUCLK_FREQ / 1000U);
        elapsedMs++;

        if (motorTestRemainingMs > 0U) {
            motorTestRemainingMs--;
            if (motorTestRemainingMs == 0U) {
                TB6612_Stop();
                UART_SendString("电机测试结束，已自动停止\r\n");
            }
        }

        if (elapsedMs >= 1000U) {
            elapsedMs = 0U;
            runtimeSeconds++;
            sprintf(buffer, "[心跳] 运行时间: %u 秒\r\n", (unsigned int)runtimeSeconds);
            UART_SendString(buffer);
            AtkBle02_SendString("[BLE test alive]\r\n");
        }
    }
}
