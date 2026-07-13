#ifndef TB6612_H_
#define TB6612_H_

#include <stdbool.h>
#include <stdint.h>

#define TB6612_SPEED_MAX (1000)

/* Change an individual value to 1 if that motor runs backwards. */
#define TB6612_M1_REVERSED (0)
#define TB6612_M2_REVERSED (0)
#define TB6612_M3_REVERSED (0)
#define TB6612_M4_REVERSED (0)

void TB6612_Init(void);
void TB6612_SetStandby(bool enabled);
void TB6612_SetFour(int16_t m1Speed, int16_t m2Speed, int16_t m3Speed, int16_t m4Speed);
void TB6612_SetSpeed(int16_t leftSpeed, int16_t rightSpeed);
void TB6612_Stop(void);

#endif
