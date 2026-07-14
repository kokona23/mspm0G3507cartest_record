#include "encoder.h"

#include "ti_msp_dl_config.h"

static volatile int32_t g_encoderCount[ENCODER_COUNT];
static uint8_t g_encoderState[ENCODER_COUNT];

/* Index is (previous AB state << 2) | current AB state. */
static const int8_t g_quadratureDelta[16] = {
    0, 1, -1, 0,
    -1, 0, 0, 1,
    1, 0, 0, -1,
    0, -1, 1, 0,
};

static uint8_t Encoder_ReadState(Encoder_Id encoder)
{
    uint32_t pinsA;
    uint32_t pinsB;

    switch (encoder) {
        case ENCODER_M1:
            pinsA = DL_GPIO_readPins(GPIOA, ENCODER_ENC1_A_PIN);
            pinsB = DL_GPIO_readPins(GPIOA, ENCODER_ENC1_B_PIN);
            break;
        case ENCODER_M2:
            pinsA = DL_GPIO_readPins(GPIOA, ENCODER_ENC2_A_PIN);
            pinsB = DL_GPIO_readPins(GPIOA, ENCODER_ENC2_B_PIN);
            break;
        case ENCODER_M3:
            pinsA = DL_GPIO_readPins(GPIOA, ENCODER_ENC3_A_PIN);
            pinsB = DL_GPIO_readPins(GPIOA, ENCODER_ENC3_B_PIN);
            break;
        case ENCODER_M4:
            pinsA = DL_GPIO_readPins(GPIOA, ENCODER_ENC4_A_PIN);
            pinsB = DL_GPIO_readPins(GPIOB, ENCODER_ENC4_B_PIN);
            break;
        default:
            return 0U;
    }

    return ((pinsA != 0U) ? 2U : 0U) | ((pinsB != 0U) ? 1U : 0U);
}

static void Encoder_Process(Encoder_Id encoder)
{
    uint8_t current = Encoder_ReadState(encoder);
    uint8_t previous = g_encoderState[encoder];

    g_encoderCount[encoder] += g_quadratureDelta[(previous << 2U) | current];
    g_encoderState[encoder] = current;
}

void Encoder_Init(void)
{
    uint8_t encoder;

    for (encoder = 0U; encoder < (uint8_t)ENCODER_COUNT; encoder++) {
        g_encoderCount[encoder] = 0;
        g_encoderState[encoder] = Encoder_ReadState((Encoder_Id)encoder);
    }

    NVIC_EnableIRQ(ENCODER_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(ENCODER_GPIOB_INT_IRQN);
}

int32_t Encoder_GetCount(Encoder_Id encoder)
{
    if ((uint8_t)encoder >= (uint8_t)ENCODER_COUNT) {
        return 0;
    }
    return g_encoderCount[encoder];
}

void Encoder_ResetCount(Encoder_Id encoder)
{
    if ((uint8_t)encoder >= (uint8_t)ENCODER_COUNT) {
        return;
    }
    g_encoderCount[encoder] = 0;
    g_encoderState[encoder] = Encoder_ReadState(encoder);
}

void GPIOA_IRQHandler(void)
{
    const uint32_t encoderPins = ENCODER_ENC1_A_PIN | ENCODER_ENC1_B_PIN |
                                 ENCODER_ENC2_A_PIN | ENCODER_ENC2_B_PIN |
                                 ENCODER_ENC3_A_PIN | ENCODER_ENC3_B_PIN |
                                 ENCODER_ENC4_A_PIN;
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(GPIOA, encoderPins);

    DL_GPIO_clearInterruptStatus(GPIOA, status);
    if ((status & (ENCODER_ENC1_A_PIN | ENCODER_ENC1_B_PIN)) != 0U) Encoder_Process(ENCODER_M1);
    if ((status & (ENCODER_ENC2_A_PIN | ENCODER_ENC2_B_PIN)) != 0U) Encoder_Process(ENCODER_M2);
    if ((status & (ENCODER_ENC3_A_PIN | ENCODER_ENC3_B_PIN)) != 0U) Encoder_Process(ENCODER_M3);
    if ((status & ENCODER_ENC4_A_PIN) != 0U) Encoder_Process(ENCODER_M4);
}

void GPIOB_IRQHandler(void)
{
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(GPIOB, ENCODER_ENC4_B_PIN);

    DL_GPIO_clearInterruptStatus(GPIOB, status);
    if ((status & ENCODER_ENC4_B_PIN) != 0U) Encoder_Process(ENCODER_M4);
}
