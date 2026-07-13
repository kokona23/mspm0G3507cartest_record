#ifndef ATK_BLE02_H
#define ATK_BLE02_H

#include <stdbool.h>
#include <stdint.h>

/* UART3 is used for the ATK-BLE02 (ATK-MW579) Bluetooth module. */
void AtkBle02_SendByte(uint8_t byte);
void AtkBle02_SendString(const char *text);
bool AtkBle02_ReadByte(uint8_t *byte);

#endif
