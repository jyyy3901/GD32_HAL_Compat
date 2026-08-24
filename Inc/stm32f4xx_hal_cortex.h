#ifndef STM32F4XX_HAL_CORTEX_H
#define STM32F4XX_HAL_CORTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"

#define NVIC_PRIORITYGROUP_0 0x00000007U
#define NVIC_PRIORITYGROUP_1 0x00000006U
#define NVIC_PRIORITYGROUP_2 0x00000005U
#define NVIC_PRIORITYGROUP_3 0x00000004U
#define NVIC_PRIORITYGROUP_4 0x00000003U

#ifdef SYSTICK_CLKSOURCE_HCLK_DIV8
#undef SYSTICK_CLKSOURCE_HCLK_DIV8
#endif
#ifdef SYSTICK_CLKSOURCE_HCLK
#undef SYSTICK_CLKSOURCE_HCLK
#endif
#define SYSTICK_CLKSOURCE_HCLK_DIV8 0x00000000U
#define SYSTICK_CLKSOURCE_HCLK 0x00000004U

#if (__MPU_PRESENT == 1U)
typedef struct
{
    uint8_t Enable;
    uint8_t Number;
    uint32_t BaseAddress;
    uint8_t Size;
    uint8_t SubRegionDisable;
    uint8_t TypeExtField;
    uint8_t AccessPermission;
    uint8_t DisableExec;
    uint8_t IsShareable;
    uint8_t IsCacheable;
    uint8_t IsBufferable;
} MPU_Region_InitTypeDef;

#define MPU_HFNMI_PRIVDEF_NONE 0x00000000U
#define MPU_HARDFAULT_NMI MPU_CTRL_HFNMIENA_Msk
#define MPU_PRIVILEGED_DEFAULT MPU_CTRL_PRIVDEFENA_Msk
#define MPU_HFNMI_PRIVDEF (MPU_CTRL_HFNMIENA_Msk | MPU_CTRL_PRIVDEFENA_Msk)
#define MPU_REGION_ENABLE ((uint8_t)0x01U)
#define MPU_REGION_DISABLE ((uint8_t)0x00U)
#define MPU_INSTRUCTION_ACCESS_ENABLE ((uint8_t)0x00U)
#define MPU_INSTRUCTION_ACCESS_DISABLE ((uint8_t)0x01U)
#define MPU_ACCESS_SHAREABLE ((uint8_t)0x01U)
#define MPU_ACCESS_NOT_SHAREABLE ((uint8_t)0x00U)
#define MPU_ACCESS_CACHEABLE ((uint8_t)0x01U)
#define MPU_ACCESS_NOT_CACHEABLE ((uint8_t)0x00U)
#define MPU_ACCESS_BUFFERABLE ((uint8_t)0x01U)
#define MPU_ACCESS_NOT_BUFFERABLE ((uint8_t)0x00U)
#define MPU_TEX_LEVEL0 ((uint8_t)0x00U)
#define MPU_TEX_LEVEL1 ((uint8_t)0x01U)
#define MPU_TEX_LEVEL2 ((uint8_t)0x02U)

#define MPU_REGION_SIZE_32B ((uint8_t)0x04U)
#define MPU_REGION_SIZE_64B ((uint8_t)0x05U)
#define MPU_REGION_SIZE_128B ((uint8_t)0x06U)
#define MPU_REGION_SIZE_256B ((uint8_t)0x07U)
#define MPU_REGION_SIZE_512B ((uint8_t)0x08U)
#define MPU_REGION_SIZE_1KB ((uint8_t)0x09U)
#define MPU_REGION_SIZE_2KB ((uint8_t)0x0AU)
#define MPU_REGION_SIZE_4KB ((uint8_t)0x0BU)
#define MPU_REGION_SIZE_8KB ((uint8_t)0x0CU)
#define MPU_REGION_SIZE_16KB ((uint8_t)0x0DU)
#define MPU_REGION_SIZE_32KB ((uint8_t)0x0EU)
#define MPU_REGION_SIZE_64KB ((uint8_t)0x0FU)
#define MPU_REGION_SIZE_128KB ((uint8_t)0x10U)
#define MPU_REGION_SIZE_256KB ((uint8_t)0x11U)
#define MPU_REGION_SIZE_512KB ((uint8_t)0x12U)
#define MPU_REGION_SIZE_1MB ((uint8_t)0x13U)
#define MPU_REGION_SIZE_2MB ((uint8_t)0x14U)
#define MPU_REGION_SIZE_4MB ((uint8_t)0x15U)
#define MPU_REGION_SIZE_8MB ((uint8_t)0x16U)
#define MPU_REGION_SIZE_16MB ((uint8_t)0x17U)
#define MPU_REGION_SIZE_32MB ((uint8_t)0x18U)
#define MPU_REGION_SIZE_64MB ((uint8_t)0x19U)
#define MPU_REGION_SIZE_128MB ((uint8_t)0x1AU)
#define MPU_REGION_SIZE_256MB ((uint8_t)0x1BU)
#define MPU_REGION_SIZE_512MB ((uint8_t)0x1CU)
#define MPU_REGION_SIZE_1GB ((uint8_t)0x1DU)
#define MPU_REGION_SIZE_2GB ((uint8_t)0x1EU)
#define MPU_REGION_SIZE_4GB ((uint8_t)0x1FU)

#define MPU_REGION_NO_ACCESS ((uint8_t)0x00U)
#define MPU_REGION_PRIV_RW ((uint8_t)0x01U)
#define MPU_REGION_PRIV_RW_URO ((uint8_t)0x02U)
#define MPU_REGION_FULL_ACCESS ((uint8_t)0x03U)
#define MPU_REGION_PRIV_RO ((uint8_t)0x05U)
#define MPU_REGION_PRIV_RO_URO ((uint8_t)0x06U)

#define MPU_REGION_NUMBER0 ((uint8_t)0x00U)
#define MPU_REGION_NUMBER1 ((uint8_t)0x01U)
#define MPU_REGION_NUMBER2 ((uint8_t)0x02U)
#define MPU_REGION_NUMBER3 ((uint8_t)0x03U)
#define MPU_REGION_NUMBER4 ((uint8_t)0x04U)
#define MPU_REGION_NUMBER5 ((uint8_t)0x05U)
#define MPU_REGION_NUMBER6 ((uint8_t)0x06U)
#define MPU_REGION_NUMBER7 ((uint8_t)0x07U)
#endif

void HAL_NVIC_SetPriorityGrouping(uint32_t PriorityGroup);
void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);
void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);
void HAL_NVIC_DisableIRQ(IRQn_Type IRQn);
void HAL_NVIC_SystemReset(void);
uint32_t HAL_SYSTICK_Config(uint32_t TicksNumb);
uint32_t HAL_NVIC_GetPriorityGrouping(void);
void HAL_NVIC_GetPriority(IRQn_Type IRQn, uint32_t PriorityGroup, uint32_t *pPreemptPriority, uint32_t *pSubPriority);
uint32_t HAL_NVIC_GetPendingIRQ(IRQn_Type IRQn);
void HAL_NVIC_SetPendingIRQ(IRQn_Type IRQn);
void HAL_NVIC_ClearPendingIRQ(IRQn_Type IRQn);
uint32_t HAL_NVIC_GetActive(IRQn_Type IRQn);
void HAL_SYSTICK_CLKSourceConfig(uint32_t CLKSource);
void HAL_SYSTICK_IRQHandler(void);
void HAL_SYSTICK_Callback(void);

#if (__MPU_PRESENT == 1U)
void HAL_MPU_Enable(uint32_t MPU_Control);
void HAL_MPU_Disable(void);
void HAL_MPU_EnableRegion(uint32_t RegionNumber);
void HAL_MPU_DisableRegion(uint32_t RegionNumber);
void HAL_MPU_ConfigRegion(MPU_Region_InitTypeDef *MPU_Init);
#endif

void HAL_CORTEX_ClearEvent(void);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_CORTEX_H */
