#include "gd32_hal_port.h"
#include "gd32f403_timer.h"
#include <stddef.h>

typedef struct
{
    uint32_t timer;
    uint8_t channels;
    uint8_t bidirectional;
    uint8_t repetition;
    uint8_t one_pulse;
    uint8_t master_slave;
} TIMER_Capability;

typedef struct
{
    uint32_t timer;
    GD32_HAL_TIMERDMARequest request;
    uint32_t channel;
    uint32_t token;
} TIMER_DMAMap;

static const TIMER_Capability timerCapabilities[] =
{
    {GD32_HAL_TIMER0_ADDRESS, 4U, 1U, 1U, 1U, 1U},
    {GD32_HAL_TIMER1_ADDRESS, 4U, 1U, 0U, 1U, 1U},
    {GD32_HAL_TIMER2_ADDRESS, 4U, 1U, 0U, 1U, 1U},
    {GD32_HAL_TIMER3_ADDRESS, 4U, 1U, 0U, 1U, 1U},
    {GD32_HAL_TIMER4_ADDRESS, 4U, 1U, 0U, 1U, 1U},
    {GD32_HAL_TIMER5_ADDRESS, 0U, 0U, 0U, 1U, 0U},
    {GD32_HAL_TIMER6_ADDRESS, 0U, 0U, 0U, 1U, 0U},
    {GD32_HAL_TIMER7_ADDRESS, 4U, 1U, 1U, 1U, 1U},
    {GD32_HAL_TIMER8_ADDRESS, 2U, 0U, 0U, 1U, 1U},
    {GD32_HAL_TIMER9_ADDRESS, 1U, 0U, 0U, 0U, 0U},
    {GD32_HAL_TIMER10_ADDRESS, 1U, 0U, 0U, 0U, 0U},
    {GD32_HAL_TIMER11_ADDRESS, 2U, 0U, 0U, 1U, 1U},
    {GD32_HAL_TIMER12_ADDRESS, 1U, 0U, 0U, 0U, 0U},
    {GD32_HAL_TIMER13_ADDRESS, 1U, 0U, 0U, 0U, 0U}
};

#define TIMER_DMA_ENTRY(TIMER_VALUE, REQUEST_VALUE, CONTROLLER, CHANNEL, SLOT) \
    {(TIMER_VALUE), (REQUEST_VALUE), \
     GD32_HAL_DMA_CHANNEL_ADDRESS(((CONTROLLER) == 0U) ? GD32_HAL_DMA0_ADDRESS : GD32_HAL_DMA1_ADDRESS, (CHANNEL)), \
     GD32_HAL_DMA_REQUEST_ENCODE((CONTROLLER), (CHANNEL), (SLOT))}

static const TIMER_DMAMap timerDMAMap[] =
{
    TIMER_DMA_ENTRY(GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER_DMA_CC1, 0U, 1U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER_DMA_CC2, 0U, 2U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER_DMA_CC3, 0U, 5U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER_DMA_CC4, 0U, 3U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER_DMA_UPDATE, 0U, 4U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER_DMA_TRIGGER, 0U, 3U, 4U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER_DMA_COM, 0U, 3U, 5U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER_DMA_CC1, 0U, 4U, 4U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER_DMA_CC2, 0U, 6U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER_DMA_CC3, 0U, 0U, 1U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER_DMA_CC4, 0U, 6U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER_DMA_UPDATE, 0U, 1U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER_DMA_CC1, 0U, 5U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER_DMA_CC3, 0U, 1U, 4U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER_DMA_CC4, 0U, 2U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER_DMA_UPDATE, 0U, 2U, 4U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER3_ADDRESS, GD32_HAL_TIMER_DMA_CC1, 0U, 0U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER3_ADDRESS, GD32_HAL_TIMER_DMA_CC2, 0U, 3U, 6U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER3_ADDRESS, GD32_HAL_TIMER_DMA_CC3, 0U, 4U, 5U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER3_ADDRESS, GD32_HAL_TIMER_DMA_UPDATE, 0U, 6U, 4U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER_DMA_CC1, 1U, 4U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER_DMA_CC2, 1U, 3U, 1U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER_DMA_CC3, 1U, 1U, 1U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER_DMA_CC4, 1U, 0U, 1U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER_DMA_UPDATE, 1U, 1U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER_DMA_TRIGGER, 1U, 0U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER5_ADDRESS, GD32_HAL_TIMER_DMA_UPDATE, 1U, 2U, 1U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER6_ADDRESS, GD32_HAL_TIMER_DMA_UPDATE, 1U, 3U, 2U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER7_ADDRESS, GD32_HAL_TIMER_DMA_CC1, 1U, 2U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER7_ADDRESS, GD32_HAL_TIMER_DMA_CC2, 1U, 4U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER7_ADDRESS, GD32_HAL_TIMER_DMA_CC3, 1U, 0U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER7_ADDRESS, GD32_HAL_TIMER_DMA_CC4, 1U, 1U, 3U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER7_ADDRESS, GD32_HAL_TIMER_DMA_UPDATE, 1U, 0U, 4U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER7_ADDRESS, GD32_HAL_TIMER_DMA_TRIGGER, 1U, 1U, 4U),
    TIMER_DMA_ENTRY(GD32_HAL_TIMER7_ADDRESS, GD32_HAL_TIMER_DMA_COM, 1U, 1U, 5U)
};

static const TIMER_Capability *TIMER_FindCapability(uint32_t timer)
{
    uint32_t i;
    for (i = 0U; i < (uint32_t)(sizeof(timerCapabilities) / sizeof(timerCapabilities[0])); ++i)
    {
        if (timerCapabilities[i].timer == timer)
        {
            return &timerCapabilities[i];
        }
    }
    return NULL;
}

static uint16_t TIMER_Channel(uint8_t channel)
{
    static const uint16_t channels[] = {TIMER_CH_0, TIMER_CH_1, TIMER_CH_2, TIMER_CH_3};
    return channels[channel];
}

static uint16_t TIMER_OCMode(GD32_HAL_TIMEROCMode mode)
{
    static const uint16_t modes[] =
    {
        TIMER_OC_MODE_TIMING,
        TIMER_OC_MODE_ACTIVE,
        TIMER_OC_MODE_INACTIVE,
        TIMER_OC_MODE_TOGGLE,
        TIMER_OC_MODE_LOW,
        TIMER_OC_MODE_HIGH,
        TIMER_OC_MODE_PWM0,
        TIMER_OC_MODE_PWM1
    };
    return modes[(uint32_t)mode];
}

int GD32_HAL_TIMER_IsInstance(uint32_t timer_address)
{
    return TIMER_FindCapability(timer_address) != NULL;
}

int GD32_HAL_TIMER_IsChannelValid(uint32_t timer_address, uint8_t channel)
{
    const TIMER_Capability *capability = TIMER_FindCapability(timer_address);
    return (capability != NULL) && (channel < capability->channels);
}

int GD32_HAL_TIMER_IsOnePulseCapable(uint32_t timer_address)
{
    const TIMER_Capability *capability = TIMER_FindCapability(timer_address);
    return (capability != NULL) && (capability->one_pulse != 0U);
}

int GD32_HAL_TIMER_IsMasterSlaveCapable(uint32_t timer_address)
{
    const TIMER_Capability *capability = TIMER_FindCapability(timer_address);
    return (capability != NULL) && (capability->master_slave != 0U);
}

int GD32_HAL_TIMER_ConfigureBase(uint32_t timer_address,
                                 const GD32_HAL_TIMERBaseConfig *config)
{
    const TIMER_Capability *capability = TIMER_FindCapability(timer_address);
    timer_parameter_struct parameters;
    static const uint16_t alignments[] =
    {
        TIMER_COUNTER_EDGE,
        TIMER_COUNTER_CENTER_DOWN,
        TIMER_COUNTER_CENTER_UP,
        TIMER_COUNTER_CENTER_BOTH
    };
    uint16_t clock_division;

    if ((capability == NULL) || (config == NULL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, timer_address);
        return -1;
    }
    if ((config->alignment > GD32_HAL_TIMER_ALIGNMENT_CENTER_BOTH) ||
        (config->direction > GD32_HAL_TIMER_DIRECTION_DOWN) ||
        ((config->clock_division != 1U) && (config->clock_division != 2U) &&
         (config->clock_division != 4U)))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG, timer_address);
        return -1;
    }
    if (((config->alignment != GD32_HAL_TIMER_ALIGNMENT_EDGE) ||
         (config->direction != GD32_HAL_TIMER_DIRECTION_UP)) &&
        (capability->bidirectional == 0U))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_COUNTER_MODE_UNSUPPORTED,
                           timer_address);
        return -1;
    }
    if ((config->repetition != 0U) && (capability->repetition == 0U))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_REPETITION_UNSUPPORTED,
                           timer_address);
        return -1;
    }

    clock_division = (config->clock_division == 1U) ? TIMER_CKDIV_DIV1 :
                     (config->clock_division == 2U) ? TIMER_CKDIV_DIV2 :
                                                      TIMER_CKDIV_DIV4;
    timer_struct_para_init(&parameters);
    parameters.prescaler = config->prescaler;
    parameters.alignedmode = alignments[(uint32_t)config->alignment];
    parameters.counterdirection = (config->direction == GD32_HAL_TIMER_DIRECTION_DOWN) ?
                                  TIMER_COUNTER_DOWN : TIMER_COUNTER_UP;
    parameters.clockdivision = clock_division;
    parameters.period = config->period;
    parameters.repetitioncounter = config->repetition;
    timer_update_source_config(timer_address, TIMER_UPDATE_SRC_REGULAR);
    timer_init(timer_address, &parameters);
    if (config->auto_reload_preload != 0U)
    {
        timer_auto_reload_shadow_enable(timer_address);
    }
    else
    {
        timer_auto_reload_shadow_disable(timer_address);
    }
    timer_flag_clear(timer_address, TIMER_FLAG_UP);
    return 0;
}

void GD32_HAL_TIMER_DeInit(uint32_t timer_address)
{
    if (GD32_HAL_TIMER_IsInstance(timer_address) != 0)
    {
        timer_deinit(timer_address);
    }
}

void GD32_HAL_TIMER_Enable(uint32_t timer_address)
{
    if (GD32_HAL_TIMER_IsInstance(timer_address) != 0)
    {
        timer_enable(timer_address);
    }
}

void GD32_HAL_TIMER_Disable(uint32_t timer_address)
{
    if (GD32_HAL_TIMER_IsInstance(timer_address) != 0)
    {
        timer_disable(timer_address);
    }
}

void GD32_HAL_TIMER_DisableIfIdle(uint32_t timer_address)
{
    if ((GD32_HAL_TIMER_IsInstance(timer_address) != 0) &&
        ((TIMER_CHCTL2(timer_address) &
          (TIMER_CHCTL2_CH0EN | TIMER_CHCTL2_CH1EN |
           TIMER_CHCTL2_CH2EN | TIMER_CHCTL2_CH3EN)) == 0U))
    {
        timer_disable(timer_address);
    }
}

int GD32_HAL_TIMER_IsEnabled(uint32_t timer_address)
{
    return (GD32_HAL_TIMER_IsInstance(timer_address) != 0) &&
           ((TIMER_CTL0(timer_address) & TIMER_CTL0_CEN) != 0U);
}

int GD32_HAL_TIMER_IsTriggerSlaveMode(uint32_t timer_address)
{
    return (GD32_HAL_TIMER_IsMasterSlaveCapable(timer_address) != 0) &&
           ((TIMER_SMCFG(timer_address) & TIMER_SMCFG_SMC) ==
            TIMER_SLAVE_MODE_EVENT);
}

void GD32_HAL_TIMER_SetInternalClock(uint32_t timer_address)
{
    if (GD32_HAL_TIMER_IsInstance(timer_address) != 0)
    {
        timer_internal_clock_config(timer_address);
    }
}

int GD32_HAL_TIMER_ConfigureOutput(uint32_t timer_address,
                                   uint8_t channel,
                                   const GD32_HAL_TIMEROCConfig *config)
{
    timer_oc_parameter_struct output;
    uint16_t native_channel;

    if ((config == NULL) ||
        (config->mode > GD32_HAL_TIMER_OC_PWM2) ||
        (config->polarity > GD32_HAL_TIMER_POLARITY_FALLING_LOW) ||
        (GD32_HAL_TIMER_IsChannelValid(timer_address, channel) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_CHANNEL_UNSUPPORTED,
                           timer_address | channel);
        return -1;
    }

    native_channel = TIMER_Channel(channel);
    timer_channel_output_struct_para_init(&output);
    output.outputstate = TIMER_CCX_DISABLE;
    output.outputnstate = TIMER_CCXN_DISABLE;
    output.ocpolarity = (config->polarity == GD32_HAL_TIMER_POLARITY_FALLING_LOW) ?
                        TIMER_OC_POLARITY_LOW : TIMER_OC_POLARITY_HIGH;
    output.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
    output.ocidlestate = (config->idle_high != 0U) ?
                         TIMER_OC_IDLE_STATE_HIGH : TIMER_OC_IDLE_STATE_LOW;
    output.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(timer_address, native_channel, &output);
    timer_channel_output_mode_config(timer_address,
                                     native_channel,
                                     TIMER_OCMode(config->mode));
    timer_channel_output_pulse_value_config(timer_address,
                                            native_channel,
                                            config->pulse);
    timer_channel_output_fast_config(timer_address,
                                     native_channel,
                                     (config->fast != 0U) ?
                                     TIMER_OC_FAST_ENABLE : TIMER_OC_FAST_DISABLE);
    timer_channel_output_shadow_config(timer_address,
                                       native_channel,
                                       (config->preload != 0U) ?
                                       TIMER_OC_SHADOW_ENABLE : TIMER_OC_SHADOW_DISABLE);
    return 0;
}

int GD32_HAL_TIMER_ConfigureInput(uint32_t timer_address,
                                  uint8_t channel,
                                  const GD32_HAL_TIMERICConfig *config)
{
    timer_ic_parameter_struct input;
    uint16_t native_channel;
    static const uint16_t selections[] =
    {
        TIMER_IC_SELECTION_DIRECTTI,
        TIMER_IC_SELECTION_INDIRECTTI,
        TIMER_IC_SELECTION_ITS
    };
    static const uint16_t prescalers[] =
    {
        TIMER_IC_PSC_DIV1,
        TIMER_IC_PSC_DIV2,
        TIMER_IC_PSC_DIV4,
        TIMER_IC_PSC_DIV8
    };

    if ((config == NULL) ||
        (config->selection > GD32_HAL_TIMER_IC_TRIGGER) ||
        (config->polarity > GD32_HAL_TIMER_POLARITY_BOTH) ||
        (config->prescaler > 3U) || (config->filter > 15U) ||
        ((channel == 3U) && (config->polarity == GD32_HAL_TIMER_POLARITY_BOTH)) ||
        (GD32_HAL_TIMER_IsChannelValid(timer_address, channel) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_CHANNEL_UNSUPPORTED,
                           timer_address | channel);
        return -1;
    }

    native_channel = TIMER_Channel(channel);
    timer_channel_input_struct_para_init(&input);
    input.icpolarity = (config->polarity == GD32_HAL_TIMER_POLARITY_RISING_HIGH) ?
                       TIMER_IC_POLARITY_RISING :
                       (config->polarity == GD32_HAL_TIMER_POLARITY_FALLING_LOW) ?
                       TIMER_IC_POLARITY_FALLING :
                       (uint16_t)(TIMER_IC_POLARITY_FALLING | 0x0008U);
    input.icselection = selections[(uint32_t)config->selection];
    input.icprescaler = prescalers[config->prescaler];
    input.icfilter = config->filter;
    timer_input_capture_config(timer_address, native_channel, &input);
    timer_channel_output_state_config(timer_address, native_channel, TIMER_CCX_DISABLE);
    return 0;
}

void GD32_HAL_TIMER_SetChannel(uint32_t timer_address, uint8_t channel, int enable)
{
    if (GD32_HAL_TIMER_IsChannelValid(timer_address, channel) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_CHANNEL_UNSUPPORTED,
                           timer_address | channel);
        return;
    }
    timer_channel_output_state_config(timer_address,
                                      TIMER_Channel(channel),
                                      (enable != 0) ? TIMER_CCX_ENABLE : TIMER_CCX_DISABLE);
    if ((enable != 0) &&
        (GD32_HAL_TIMER_IsChannelInput(timer_address, channel) == 0) &&
        ((timer_address == GD32_HAL_TIMER0_ADDRESS) ||
         (timer_address == GD32_HAL_TIMER7_ADDRESS)))
    {
        timer_primary_output_config(timer_address, ENABLE);
    }
}

int GD32_HAL_TIMER_IsChannelInput(uint32_t timer_address, uint8_t channel)
{
    if (GD32_HAL_TIMER_IsChannelValid(timer_address, channel) == 0)
    {
        return 0;
    }
    if (channel == 0U) { return (TIMER_CHCTL0(timer_address) & TIMER_CHCTL0_CH0MS) != 0U; }
    if (channel == 1U) { return (TIMER_CHCTL0(timer_address) & TIMER_CHCTL0_CH1MS) != 0U; }
    if (channel == 2U) { return (TIMER_CHCTL1(timer_address) & TIMER_CHCTL1_CH2MS) != 0U; }
    return (TIMER_CHCTL1(timer_address) & TIMER_CHCTL1_CH3MS) != 0U;
}

void GD32_HAL_TIMER_SetOnePulse(uint32_t timer_address, int single)
{
    if (GD32_HAL_TIMER_IsOnePulseCapable(timer_address) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG, timer_address);
        return;
    }
    timer_single_pulse_mode_config(timer_address,
                                   (single != 0) ?
                                   TIMER_SP_MODE_SINGLE : TIMER_SP_MODE_REPETITIVE);
}

int GD32_HAL_TIMER_ConfigureMaster(uint32_t timer_address,
                                   uint8_t trigger_output,
                                   int master_slave_enable)
{
    static const uint32_t outputs[] =
    {
        TIMER_TRI_OUT_SRC_RESET,
        TIMER_TRI_OUT_SRC_ENABLE,
        TIMER_TRI_OUT_SRC_UPDATE,
        TIMER_TRI_OUT_SRC_CH0,
        TIMER_TRI_OUT_SRC_O0CPRE,
        TIMER_TRI_OUT_SRC_O1CPRE,
        TIMER_TRI_OUT_SRC_O2CPRE,
        TIMER_TRI_OUT_SRC_O3CPRE
    };

    if ((trigger_output > 7U) ||
        (GD32_HAL_TIMER_IsMasterSlaveCapable(timer_address) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_MASTER_SLAVE_UNSUPPORTED,
                           timer_address);
        return -1;
    }
    timer_master_output_trigger_source_select(timer_address, outputs[trigger_output]);
    timer_master_slave_mode_config(timer_address,
                                   (master_slave_enable != 0) ?
                                   TIMER_MASTER_SLAVE_MODE_ENABLE :
                                   TIMER_MASTER_SLAVE_MODE_DISABLE);
    return 0;
}

int GD32_HAL_TIMER_ConfigureSlave(uint32_t timer_address,
                                  const GD32_HAL_TIMERSlaveConfig *config)
{
    static const uint32_t slave_modes[] =
    {
        TIMER_SLAVE_MODE_DISABLE,
        TIMER_SLAVE_MODE_RESTART,
        TIMER_SLAVE_MODE_PAUSE,
        TIMER_SLAVE_MODE_EVENT,
        TIMER_SLAVE_MODE_EXTERNAL0
    };
    static const uint32_t trigger_sources[] =
    {
        TIMER_SMCFG_TRGSEL_ITI0,
        TIMER_SMCFG_TRGSEL_ITI1,
        TIMER_SMCFG_TRGSEL_ITI2,
        TIMER_SMCFG_TRGSEL_ITI3,
        TIMER_SMCFG_TRGSEL_CI0F_ED,
        TIMER_SMCFG_TRGSEL_CI0FE0,
        TIMER_SMCFG_TRGSEL_CI1FE1,
        TIMER_SMCFG_TRGSEL_ETIFP
    };
    static const uint32_t external_prescalers[] =
    {
        TIMER_EXT_TRI_PSC_OFF,
        TIMER_EXT_TRI_PSC_DIV2,
        TIMER_EXT_TRI_PSC_DIV4,
        TIMER_EXT_TRI_PSC_DIV8
    };
    uint8_t trigger = config != NULL ? config->input_trigger : 0U;

    if ((config == NULL) || (config->slave_mode > 4U) ||
        (trigger > 7U) || (config->trigger_prescaler > 3U) ||
        (config->trigger_filter > 15U) ||
        (GD32_HAL_TIMER_IsMasterSlaveCapable(timer_address) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_MASTER_SLAVE_UNSUPPORTED,
                           timer_address);
        return -1;
    }

    if (trigger < 4U)
    {
        uint8_t gd_index;
        if ((config->trigger_polarity != GD32_HAL_TIMER_POLARITY_RISING_HIGH) ||
            (config->trigger_prescaler != 0U) ||
            (config->trigger_filter != 0U))
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                               timer_address | trigger);
            return -1;
        }
        if (STM32_TIMER_TriggerToGD32ITI(config->stm32_timer_instance,
                                         trigger,
                                         &gd_index) == 0)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_ITR_UNMAPPABLE,
                               timer_address | trigger);
            return -1;
        }
        trigger = gd_index;
    }
    else if (trigger < 7U)
    {
        GD32_HAL_TIMERICConfig input;
        const uint8_t channel = (trigger == 6U) ? 1U : 0U;
        if (config->trigger_prescaler != 0U)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                               timer_address | config->trigger_prescaler);
            return -1;
        }
        input.polarity = config->trigger_polarity;
        input.selection = GD32_HAL_TIMER_IC_DIRECT;
        input.prescaler = 0U;
        input.filter = config->trigger_filter;
        if (GD32_HAL_TIMER_ConfigureInput(timer_address, channel, &input) != 0)
        {
            return -1;
        }
    }
    else if (trigger == 7U)
    {
        if ((timer_address != GD32_HAL_TIMER0_ADDRESS) &&
            (timer_address != GD32_HAL_TIMER1_ADDRESS) &&
            (timer_address != GD32_HAL_TIMER2_ADDRESS) &&
            (timer_address != GD32_HAL_TIMER3_ADDRESS) &&
            (timer_address != GD32_HAL_TIMER4_ADDRESS) &&
            (timer_address != GD32_HAL_TIMER7_ADDRESS))
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_MASTER_SLAVE_UNSUPPORTED,
                               timer_address);
            return -1;
        }
        if (config->trigger_polarity == GD32_HAL_TIMER_POLARITY_BOTH)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                               timer_address);
            return -1;
        }
        timer_external_trigger_config(
            timer_address,
            external_prescalers[config->trigger_prescaler],
            (config->trigger_polarity == GD32_HAL_TIMER_POLARITY_FALLING_LOW) ?
            TIMER_ETP_FALLING : TIMER_ETP_RISING,
            config->trigger_filter);
    }

    timer_input_trigger_source_select(timer_address, trigger_sources[trigger]);
    timer_slave_mode_select(timer_address, slave_modes[config->slave_mode]);
    return 0;
}

void GD32_HAL_TIMER_SetInterrupt(uint32_t timer_address,
                                 uint32_t interrupts,
                                 int enable)
{
    if (enable != 0)
    {
        timer_interrupt_enable(timer_address, interrupts & 0xFFU);
    }
    else
    {
        timer_interrupt_disable(timer_address, interrupts & 0xFFU);
    }
}

uint32_t GD32_HAL_TIMER_GetInterrupts(uint32_t timer_address)
{
    return TIMER_DMAINTEN(timer_address) & 0xFFU;
}

uint32_t GD32_HAL_TIMER_GetFlag(uint32_t timer_address, uint32_t flags)
{
    return TIMER_INTF(timer_address) & flags;
}

void GD32_HAL_TIMER_ClearFlag(uint32_t timer_address, uint32_t flags)
{
    timer_flag_clear(timer_address, flags);
}

int GD32_HAL_TIMER_IsDMAChannelValid(uint32_t timer_address,
                                     GD32_HAL_TIMERDMARequest request,
                                     uint32_t channel_address,
                                     uint32_t request_token)
{
    uint32_t i;
    for (i = 0U; i < (uint32_t)(sizeof(timerDMAMap) / sizeof(timerDMAMap[0])); ++i)
    {
        if ((timerDMAMap[i].timer == timer_address) &&
            (timerDMAMap[i].request == request) &&
            (timerDMAMap[i].channel == channel_address) &&
            (timerDMAMap[i].token == request_token))
        {
            return 1;
        }
    }
    return 0;
}

int GD32_HAL_TIMER_GetDMAMapping(uint32_t timer_address,
                                 GD32_HAL_TIMERDMARequest request,
                                 uint32_t *channel_address,
                                 uint32_t *request_token)
{
    size_t index;

    if ((channel_address == NULL) || (request_token == NULL))
    {
        return 0;
    }
    for (index = 0U;
         index < (sizeof(timerDMAMap) / sizeof(timerDMAMap[0]));
         ++index)
    {
        if ((timerDMAMap[index].timer == timer_address) &&
            (timerDMAMap[index].request == request))
        {
            *channel_address = timerDMAMap[index].channel;
            *request_token = timerDMAMap[index].token;
            return 1;
        }
    }
    return 0;
}

uint32_t GD32_HAL_TIMER_GetDMADataAddress(uint32_t timer_address,
                                          GD32_HAL_TIMERDMARequest request)
{
    if (request == GD32_HAL_TIMER_DMA_UPDATE)
    {
        return timer_address + 0x2CU;
    }
    if ((request >= GD32_HAL_TIMER_DMA_CC1) &&
        (request <= GD32_HAL_TIMER_DMA_CC4))
    {
        return timer_address + 0x34U +
               (4U * ((uint32_t)request - (uint32_t)GD32_HAL_TIMER_DMA_CC1));
    }
    GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_DMA_UNSUPPORTED,
                       timer_address | (uint32_t)request);
    return 0U;
}

void GD32_HAL_TIMER_SetCounter(uint32_t timer_address, uint32_t value)
{
    if (value > 0xFFFFU)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE, value);
        return;
    }
    timer_counter_value_config(timer_address, (uint16_t)value);
}

uint32_t GD32_HAL_TIMER_GetCounter(uint32_t timer_address)
{
    return timer_counter_read(timer_address) & 0xFFFFU;
}

void GD32_HAL_TIMER_SetAutoReload(uint32_t timer_address, uint32_t value)
{
    if (value > 0xFFFFU)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE, value);
        return;
    }
    timer_autoreload_value_config(timer_address, (uint16_t)value);
}

uint32_t GD32_HAL_TIMER_GetAutoReload(uint32_t timer_address)
{
    return TIMER_CAR(timer_address) & 0xFFFFU;
}

void GD32_HAL_TIMER_SetPrescaler(uint32_t timer_address, uint32_t value)
{
    if (value > 0xFFFFU)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE, value);
        return;
    }
    timer_prescaler_config(timer_address, (uint16_t)value, TIMER_PSC_RELOAD_UPDATE);
}

uint32_t GD32_HAL_TIMER_GetPrescaler(uint32_t timer_address)
{
    return TIMER_PSC(timer_address) & 0xFFFFU;
}

void GD32_HAL_TIMER_SetCompare(uint32_t timer_address,
                               uint8_t channel,
                               uint32_t value)
{
    if ((value > 0xFFFFU) ||
        (GD32_HAL_TIMER_IsChannelValid(timer_address, channel) == 0))
    {
        GD32_HAL_ErrorHook((value > 0xFFFFU) ?
                           GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE :
                           GD32_HAL_PORT_ERROR_TIMER_CHANNEL_UNSUPPORTED,
                           (value > 0xFFFFU) ? value : (timer_address | channel));
        return;
    }
    timer_channel_output_pulse_value_config(timer_address,
                                            TIMER_Channel(channel),
                                            value);
}

uint32_t GD32_HAL_TIMER_GetCompare(uint32_t timer_address, uint8_t channel)
{
    if (GD32_HAL_TIMER_IsChannelValid(timer_address, channel) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_CHANNEL_UNSUPPORTED,
                           timer_address | channel);
        return 0U;
    }
    return timer_channel_capture_value_register_read(timer_address,
                                                      TIMER_Channel(channel)) &
           0xFFFFU;
}
