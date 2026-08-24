#include "stm32f4xx_hal.h"

static int GD32_HAL_RCC_DecodeAHB(uint32_t token, uint32_t *divider)
{
    if (divider == NULL) return -1;
    switch (token)
    {
        case RCC_SYSCLK_DIV1: *divider = 1U; break;
        case RCC_SYSCLK_DIV2: *divider = 2U; break;
        case RCC_SYSCLK_DIV4: *divider = 4U; break;
        case RCC_SYSCLK_DIV8: *divider = 8U; break;
        case RCC_SYSCLK_DIV16: *divider = 16U; break;
        case RCC_SYSCLK_DIV64: *divider = 64U; break;
        case RCC_SYSCLK_DIV128: *divider = 128U; break;
        case RCC_SYSCLK_DIV256: *divider = 256U; break;
        case RCC_SYSCLK_DIV512: *divider = 512U; break;
        default: return -1;
    }
    return 0;
}

static int GD32_HAL_RCC_DecodeAPB(uint32_t token, uint32_t *divider)
{
    if (divider == NULL) return -1;
    switch (token)
    {
        case RCC_HCLK_DIV1: *divider = 1U; break;
        case RCC_HCLK_DIV2: *divider = 2U; break;
        case RCC_HCLK_DIV4: *divider = 4U; break;
        case RCC_HCLK_DIV8: *divider = 8U; break;
        case RCC_HCLK_DIV16: *divider = 16U; break;
        default: return -1;
    }
    return 0;
}

static int GD32_HAL_RCC_DecodeSource(uint32_t token, uint32_t *source)
{
    if (source == NULL) return -1;
    switch (token)
    {
        case RCC_SYSCLKSOURCE_HSI: *source = GD32_HAL_RCC_SOURCE_IRC8M; break;
        case RCC_SYSCLKSOURCE_HSE: *source = GD32_HAL_RCC_SOURCE_HXTAL; break;
        case RCC_SYSCLKSOURCE_PLLCLK: *source = GD32_HAL_RCC_SOURCE_PLL; break;
        default: return -1;
    }
    return 0;
}

static uint32_t GD32_HAL_RCC_EncodeAHB(uint32_t divider)
{
    switch (divider)
    {
        case 2U: return RCC_SYSCLK_DIV2;
        case 4U: return RCC_SYSCLK_DIV4;
        case 8U: return RCC_SYSCLK_DIV8;
        case 16U: return RCC_SYSCLK_DIV16;
        case 64U: return RCC_SYSCLK_DIV64;
        case 128U: return RCC_SYSCLK_DIV128;
        case 256U: return RCC_SYSCLK_DIV256;
        case 512U: return RCC_SYSCLK_DIV512;
        case 1U:
        default:
            return RCC_SYSCLK_DIV1;
    }
}

static uint32_t GD32_HAL_RCC_EncodeAPB(uint32_t divider)
{
    switch (divider)
    {
        case 2U: return RCC_HCLK_DIV2;
        case 4U: return RCC_HCLK_DIV4;
        case 8U: return RCC_HCLK_DIV8;
        case 16U: return RCC_HCLK_DIV16;
        case 1U:
        default:
            return RCC_HCLK_DIV1;
    }
}

HAL_StatusTypeDef HAL_RCC_DeInit(void)
{
    GD32_HAL_RCC_DeInit();
    SystemCoreClockUpdate();
    return HAL_InitTick(uwTickPrio);
}

HAL_StatusTypeDef HAL_RCC_OscConfig(const RCC_OscInitTypeDef *RCC_OscInitStruct)
{
    int result;

    if ((RCC_OscInitStruct == NULL) ||
        (IS_RCC_OSCILLATORTYPE(RCC_OscInitStruct->OscillatorType) == 0U) ||
        (IS_RCC_PLL(RCC_OscInitStruct->PLL.PLLState) == 0U))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG, 0U);
        return HAL_ERROR;
    }

    /* RTC 尚未进入兼容范围，LSE/LSI 不能只改 RCU 而忽略备份域语义。 */
    if ((RCC_OscInitStruct->OscillatorType &
         (RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_LSI)) != 0U)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG,
                           RCC_OscInitStruct->OscillatorType);
        return HAL_ERROR;
    }

    if ((RCC_OscInitStruct->OscillatorType & RCC_OSCILLATORTYPE_HSE) != 0U)
    {
        uint32_t state;
        if (IS_RCC_HSE(RCC_OscInitStruct->HSEState) == 0U)
        {
            return HAL_ERROR;
        }
        state = (RCC_OscInitStruct->HSEState == RCC_HSE_BYPASS) ?
                GD32_HAL_RCC_HXTAL_BYPASS :
                ((RCC_OscInitStruct->HSEState == RCC_HSE_ON) ?
                 GD32_HAL_RCC_HXTAL_ON : GD32_HAL_RCC_HXTAL_OFF);
        result = GD32_HAL_RCC_SetHXTAL(state);
        if (result != 0)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG,
                               RCC_OscInitStruct->HSEState);
            return (result == -2) ? HAL_BUSY : HAL_ERROR;
        }
    }

    if ((RCC_OscInitStruct->OscillatorType & RCC_OSCILLATORTYPE_HSI) != 0U)
    {
        if ((IS_RCC_HSI(RCC_OscInitStruct->HSIState) == 0U) ||
            (RCC_OscInitStruct->HSICalibrationValue > 0x1FU))
        {
            return HAL_ERROR;
        }
        result = GD32_HAL_RCC_SetIRC8M(
            RCC_OscInitStruct->HSIState == RCC_HSI_ON,
            RCC_OscInitStruct->HSICalibrationValue);
        if (result != 0)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG,
                               RCC_OscInitStruct->HSIState);
            return (result == -2) ? HAL_BUSY : HAL_ERROR;
        }
    }

    if (RCC_OscInitStruct->PLL.PLLState == RCC_PLL_ON)
    {
        /* STM32 PLLM/N/P/Q 无法无损转换为 GD32 PREDV/PLLMF/CK48M。 */
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_PLL_MODEL_UNSUPPORTED,
                           RCC_OscInitStruct->PLL.PLLN);
        return HAL_ERROR;
    }
    if (RCC_OscInitStruct->PLL.PLLState == RCC_PLL_OFF)
    {
        result = GD32_HAL_RCC_SetPLL(0);
        if (result != 0)
        {
            return (result == -2) ? HAL_BUSY : HAL_ERROR;
        }
    }

    return HAL_OK;
}

HAL_StatusTypeDef HAL_RCC_ClockConfig(const RCC_ClkInitTypeDef *RCC_ClkInitStruct,
                                      uint32_t FLatency)
{
    uint32_t source;
    uint32_t ahb_divider;
    uint32_t apb1_divider;
    uint32_t apb2_divider;
    uint32_t source_frequency;
    uint32_t target_hclk;
    uint32_t current_hclk;

    if ((RCC_ClkInitStruct == NULL) || (FLatency > 2U) ||
        (RCC_ClkInitStruct->ClockType == 0U) ||
        ((RCC_ClkInitStruct->ClockType & ~0x0FU) != 0U))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG, FLatency);
        return HAL_ERROR;
    }

    source = GD32_HAL_RCC_GetSystemSource();
    ahb_divider = GD32_HAL_RCC_GetAHBDivider();
    apb1_divider = GD32_HAL_RCC_GetAPB1Divider();
    apb2_divider = GD32_HAL_RCC_GetAPB2Divider();

    if (((RCC_ClkInitStruct->ClockType & RCC_CLOCKTYPE_SYSCLK) != 0U) &&
        (GD32_HAL_RCC_DecodeSource(RCC_ClkInitStruct->SYSCLKSource, &source) != 0))
    {
        return HAL_ERROR;
    }
    if (((RCC_ClkInitStruct->ClockType & RCC_CLOCKTYPE_HCLK) != 0U) &&
        (GD32_HAL_RCC_DecodeAHB(RCC_ClkInitStruct->AHBCLKDivider, &ahb_divider) != 0))
    {
        return HAL_ERROR;
    }
    if (((RCC_ClkInitStruct->ClockType & RCC_CLOCKTYPE_PCLK1) != 0U) &&
        (GD32_HAL_RCC_DecodeAPB(RCC_ClkInitStruct->APB1CLKDivider, &apb1_divider) != 0))
    {
        return HAL_ERROR;
    }
    if (((RCC_ClkInitStruct->ClockType & RCC_CLOCKTYPE_PCLK2) != 0U) &&
        (GD32_HAL_RCC_DecodeAPB(RCC_ClkInitStruct->APB2CLKDivider, &apb2_divider) != 0))
    {
        return HAL_ERROR;
    }

    source_frequency = GD32_HAL_RCC_GetSourceFrequency(source);
    if (source_frequency == 0U)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_PLL_MODEL_UNSUPPORTED, source);
        return HAL_ERROR;
    }
    target_hclk = source_frequency / ahb_divider;
    if ((target_hclk > 168000000UL) ||
        ((target_hclk / apb1_divider) > 84000000UL) ||
        ((target_hclk / apb2_divider) > 168000000UL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_CLOCK_LIMIT, target_hclk);
        return HAL_ERROR;
    }

    current_hclk = HAL_RCC_GetHCLKFreq();
    if (target_hclk > current_hclk)
    {
        GD32_HAL_RCC_SetFlashWaitState(FLatency);
    }
    if (GD32_HAL_RCC_ApplyClockConfig(source,
                                      ahb_divider,
                                      apb1_divider,
                                      apb2_divider,
                                      target_hclk > current_hclk) != 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG, source);
        return HAL_ERROR;
    }
    if (target_hclk <= current_hclk)
    {
        GD32_HAL_RCC_SetFlashWaitState(FLatency);
    }

    SystemCoreClockUpdate();
    return HAL_InitTick(uwTickPrio);
}

uint32_t HAL_RCC_GetSysClockFreq(void)
{
    return GD32_HAL_RCC_GetClockFrequency(GD32_HAL_RCC_CLOCK_SYS);
}

uint32_t HAL_RCC_GetHCLKFreq(void)
{
    return GD32_HAL_RCC_GetClockFrequency(GD32_HAL_RCC_CLOCK_AHB);
}

uint32_t HAL_RCC_GetPCLK1Freq(void)
{
    return GD32_HAL_RCC_GetClockFrequency(GD32_HAL_RCC_CLOCK_APB1);
}

uint32_t HAL_RCC_GetPCLK2Freq(void)
{
    return GD32_HAL_RCC_GetClockFrequency(GD32_HAL_RCC_CLOCK_APB2);
}

void HAL_RCC_GetOscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct)
{
    uint32_t state;

    if (RCC_OscInitStruct == NULL) return;
    state = GD32_HAL_RCC_GetOscillatorState();
    RCC_OscInitStruct->OscillatorType = RCC_OSCILLATORTYPE_HSE |
                                        RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct->HSIState = ((state & GD32_HAL_RCC_OSC_IRC8M_ON) != 0U) ?
                                  RCC_HSI_ON : RCC_HSI_OFF;
    RCC_OscInitStruct->HSICalibrationValue = GD32_HAL_RCC_GetIRC8MCalibration();
    if ((state & GD32_HAL_RCC_OSC_HXTAL_ON) == 0U)
    {
        RCC_OscInitStruct->HSEState = RCC_HSE_OFF;
    }
    else
    {
        RCC_OscInitStruct->HSEState =
            ((state & GD32_HAL_RCC_OSC_HXTAL_BYPASS) != 0U) ?
            RCC_HSE_BYPASS : RCC_HSE_ON;
    }
    RCC_OscInitStruct->LSEState = 0U;
    RCC_OscInitStruct->LSIState = 0U;
    RCC_OscInitStruct->PLL.PLLState =
        ((state & GD32_HAL_RCC_OSC_PLL_ON) != 0U) ? RCC_PLL_ON : RCC_PLL_OFF;
    RCC_OscInitStruct->PLL.PLLSource = 0U;
    RCC_OscInitStruct->PLL.PLLM = 0U;
    RCC_OscInitStruct->PLL.PLLN = 0U;
    RCC_OscInitStruct->PLL.PLLP = 0U;
    RCC_OscInitStruct->PLL.PLLQ = 0U;
}

void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct,
                            uint32_t *pFLatency)
{
    uint32_t source;

    if ((RCC_ClkInitStruct == NULL) || (pFLatency == NULL)) return;
    source = GD32_HAL_RCC_GetSystemSource();
    RCC_ClkInitStruct->ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct->SYSCLKSource =
        (source == GD32_HAL_RCC_SOURCE_HXTAL) ? RCC_SYSCLKSOURCE_HSE :
        ((source == GD32_HAL_RCC_SOURCE_PLL) ? RCC_SYSCLKSOURCE_PLLCLK :
         RCC_SYSCLKSOURCE_HSI);
    RCC_ClkInitStruct->AHBCLKDivider =
        GD32_HAL_RCC_EncodeAHB(GD32_HAL_RCC_GetAHBDivider());
    RCC_ClkInitStruct->APB1CLKDivider =
        GD32_HAL_RCC_EncodeAPB(GD32_HAL_RCC_GetAPB1Divider());
    RCC_ClkInitStruct->APB2CLKDivider =
        GD32_HAL_RCC_EncodeAPB(GD32_HAL_RCC_GetAPB2Divider());
    *pFLatency = GD32_HAL_RCC_GetFlashWaitState();
}

void HAL_RCC_EnableCSS(void)
{
    GD32_HAL_RCC_EnableClockMonitor(1);
}

void HAL_RCC_DisableCSS(void)
{
    GD32_HAL_RCC_EnableClockMonitor(0);
}

void HAL_RCC_NMI_IRQHandler(void)
{
    if (GD32_HAL_RCC_GetClockMonitorInterrupt() != 0U)
    {
        GD32_HAL_RCC_ClearClockMonitorInterrupt();
        HAL_RCC_CSSCallback();
    }
}
