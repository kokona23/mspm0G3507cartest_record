#include "atk_ble02.h"
#include "ti_msp_dl_config.h"

void AtkBle02_SendByte(uint8_t byte)
{
    while (DL_UART_Main_isTXFIFOFull(UART3_INST)) {
    }
    DL_UART_Main_transmitData(UART3_INST, byte);
}

void AtkBle02_SendString(const char *text)
{
    while (*text != '\0') {
        AtkBle02_SendByte((uint8_t)*text);
        text++;
    }
}

bool AtkBle02_ReadByte(uint8_t *byte)
{
    if (DL_UART_Main_isRXFIFOEmpty(UART3_INST)) {
        return false;
    }

    *byte = DL_UART_Main_receiveData(UART3_INST);
    return true;
}
