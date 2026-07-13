#include "ti_msp_dl_config.h"
#include <stdio.h>

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
}

int main(void)
{
    uint32_t runtimeSeconds = 0U;
    uint32_t elapsedMs = 0U;
    char buffer[64];

    SYSCFG_DL_init();

    UART_SendString("\r\n");
    UART_SendString("========================================\r\n");
    UART_SendString("  MSPM0G3507 串口测试程序\r\n");
    UART_SendString("  波特率: 115200\r\n");
    UART_SendString("  CPU频率: 32MHz\r\n");
    UART_SendString("========================================\r\n\r\n");
    UART_SendString("系统启动成功！\r\n");
    UART_SendString("发送任意字符进行回显测试\r\n\r\n");

    sprintf(buffer, "[心跳] 运行时间: %u 秒\r\n", (unsigned int)runtimeSeconds);
    UART_SendString(buffer);

    while (1) {
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
                default:
                    break;
            }
        }

        delay_cycles(CPUCLK_FREQ / 1000U);
        elapsedMs++;

        if (elapsedMs >= 1000U) {
            elapsedMs = 0U;
            runtimeSeconds++;
            sprintf(buffer, "[心跳] 运行时间: %u 秒\r\n", (unsigned int)runtimeSeconds);
            UART_SendString(buffer);
        }
    }
}
