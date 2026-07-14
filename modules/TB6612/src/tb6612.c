#include "tb6612.h"

#include "ti_msp_dl_config.h"

#define MOTOR_PWM_PERIOD (1000U)

static int16_t TB6612_ClampSpeed(int16_t speed)
{
    if (speed > TB6612_SPEED_MAX) return TB6612_SPEED_MAX;
    if (speed < -TB6612_SPEED_MAX) return -TB6612_SPEED_MAX;
    return speed;
}

static void TB6612_WritePin(GPIO_Regs *port, uint32_t pin, bool high)
{
    if (high) {
        DL_GPIO_setPins(port, pin);
    } else {
        DL_GPIO_clearPins(port, pin);
    }
}

static void TB6612_SetOne(int16_t speed, bool reversed, GPIO_Regs *in1Port, uint32_t in1Pin,
                         GPIO_Regs *in2Port, uint32_t in2Pin, GPTIMER_Regs *pwm,
                         DL_TIMER_CC_INDEX pwmIndex)
{
    uint32_t duty;
    bool forward;

    speed = TB6612_ClampSpeed(speed);
    forward = (speed >= 0);
    if (reversed) forward = !forward;

    if (speed < 0) {
        duty = (uint32_t)(-speed);
    } else {
        duty = (uint32_t)speed;
    }

    if (duty == 0U) {
        TB6612_WritePin(in1Port, in1Pin, false);
        TB6612_WritePin(in2Port, in2Pin, false);
    } else {
        TB6612_WritePin(in1Port, in1Pin, forward);
        TB6612_WritePin(in2Port, in2Pin, !forward);
    }

    /* SysConfig uses non-inverted edge PWM: compare = period - duty. */
    DL_TimerA_setCaptureCompareValue(pwm, MOTOR_PWM_PERIOD - duty, pwmIndex);
}

void TB6612_Init(void)
{
    TB6612_Stop();
}

void TB6612_SetStandby(bool enabled)
{
    /* STBY is pulled up to 3.3 V by the motor-driver hardware. */
    (void)enabled;
}

void TB6612_SetFour(int16_t m1Speed, int16_t m2Speed, int16_t m3Speed, int16_t m4Speed)
{
    TB6612_SetOne(m1Speed, TB6612_M1_REVERSED != 0, MOTOR_DIR_M1_IN1_PORT, MOTOR_DIR_M1_IN1_PIN,
                 MOTOR_DIR_M1_IN2_PORT, MOTOR_DIR_M1_IN2_PIN, MOTOR_PWM_A_INST,
                 GPIO_MOTOR_PWM_A_C0_IDX);
    TB6612_SetOne(m2Speed, TB6612_M2_REVERSED != 0, MOTOR_DIR_M2_IN1_PORT, MOTOR_DIR_M2_IN1_PIN,
                 MOTOR_DIR_M2_IN2_PORT, MOTOR_DIR_M2_IN2_PIN, MOTOR_PWM_A_INST,
                 GPIO_MOTOR_PWM_A_C1_IDX);
    TB6612_SetOne(m3Speed, TB6612_M3_REVERSED != 0, MOTOR_DIR_M3_IN1_PORT, MOTOR_DIR_M3_IN1_PIN,
                 MOTOR_DIR_M3_IN2_PORT, MOTOR_DIR_M3_IN2_PIN, MOTOR_PWM_B_INST,
                 GPIO_MOTOR_PWM_B_C1_IDX);
    TB6612_SetOne(m4Speed, TB6612_M4_REVERSED != 0, MOTOR_DIR_M4_IN1_PORT, MOTOR_DIR_M4_IN1_PIN,
                 MOTOR_DIR_M4_IN2_PORT, MOTOR_DIR_M4_IN2_PIN, MOTOR_PWM_A_INST,
                 GPIO_MOTOR_PWM_A_C3_IDX);
}

void TB6612_SetSpeed(int16_t leftSpeed, int16_t rightSpeed)
{
    TB6612_SetFour(leftSpeed, leftSpeed, rightSpeed, rightSpeed);
}

void TB6612_Stop(void)
{
    TB6612_SetSpeed(0, 0);
}
