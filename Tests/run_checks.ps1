param(
    [string]$VendorRoot
)

$ErrorActionPreference = 'Stop'

$project = Split-Path -Parent $PSScriptRoot
$workspace = Split-Path -Parent $project
if ([string]::IsNullOrWhiteSpace($VendorRoot)) {
    $VendorRoot = Join-Path $workspace 'GD32F403RET6'
}
if (-not (Test-Path -LiteralPath $VendorRoot -PathType Container)) {
    throw "GD32F403 vendor root not found: $VendorRoot"
}
$VendorRoot = (Resolve-Path -LiteralPath $VendorRoot).Path
$vendorCmsis = Join-Path $VendorRoot 'CMSIS'
$vendorDeviceInclude = Join-Path $vendorCmsis 'GD\GD32F403\Include'
$vendorSpl = Join-Path $VendorRoot 'GD32F403_standard_peripheral'
$vendorSplInclude = Join-Path $vendorSpl 'Include'
$vendorSplSource = Join-Path $vendorSpl 'Source'

foreach ($requiredVendorPath in @($vendorCmsis, $vendorDeviceInclude,
                                  $vendorSplInclude, $vendorSplSource)) {
    if (-not (Test-Path -LiteralPath $requiredVendorPath)) {
        throw "Required GD32F403 vendor path not found: $requiredVendorPath"
    }
}
Write-Output "GD32F403 vendor root: $VendorRoot"

$clang = 'D:\mingw64\bin\clang.exe'
$clangTidy = 'D:\mingw64\bin\clang-tidy.exe'
$llvmNm = 'D:\mingw64\bin\llvm-nm.exe'
$ldLld = 'D:\mingw64\bin\ld.lld.exe'
$gcc = 'D:\mingw64\bin\gcc.exe'
$build = Join-Path $PSScriptRoot 'build'
$armBuild = Join-Path $build 'arm'

New-Item -ItemType Directory -Force -Path $build | Out-Null
New-Item -ItemType Directory -Force -Path $armBuild | Out-Null

$armIncludes = @(
    '-I', (Join-Path $PSScriptRoot 'ArmShims'),
    '-I', (Join-Path $project 'Include'),
    '-I', (Join-Path $project 'Port'),
    '-I', $VendorRoot,
    '-I', $vendorCmsis,
    '-I', $vendorDeviceInclude,
    '-I', $vendorSplInclude
)

$compatSources = @(
    (Join-Path $project 'Source\stm32f4xx_hal.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_gpio.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_cortex.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_dma.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_uart.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_uart_callbacks.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_tim.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_tim_ex.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_tim_callbacks.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_adc.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_adc_callbacks.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_i2c.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_i2c_callbacks.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_spi.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_spi_callbacks.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_rcc.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_rcc_callbacks.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_exti.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_flash.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_flash_ex.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_flash_callbacks.c'),
    (Join-Path $project 'Port\gd32_core_port.c'),
    (Join-Path $project 'Port\gd32_gpio_port.c'),
    (Join-Path $project 'Port\gd32_instance_map.c'),
    (Join-Path $project 'Port\gd32_dma_port.c'),
    (Join-Path $project 'Port\gd32_uart_port.c'),
    (Join-Path $project 'Port\gd32_timer_port.c'),
    (Join-Path $project 'Port\gd32_timer_dma_port.c'),
    (Join-Path $project 'Port\stm32_timer_trigger_map.c'),
    (Join-Path $project 'Port\gd32_adc_port.c'),
    (Join-Path $project 'Port\gd32_i2c_port.c'),
    (Join-Path $project 'Port\gd32_i2c_flag_port.c'),
    (Join-Path $project 'Port\gd32_spi_port.c'),
    (Join-Path $project 'Port\gd32_rcc_port.c'),
    (Join-Path $project 'Port\gd32_exti_port.c'),
    (Join-Path $project 'Port\gd32_flash_port.c'),
    (Join-Path $project 'Port\gd32_tick_port.c'),
    (Join-Path $project 'Port\gd32_irq_port.c')
)

foreach ($source in $compatSources) {
    $objectName = ([IO.Path]::GetFileNameWithoutExtension($source)) + '.o'
    $objectPath = Join-Path $armBuild $objectName
    & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 -ffreestanding `
        -Wall -Wextra -Werror @armIncludes -c $source -o $objectPath
    if ($LASTEXITCODE -ne 0) {
        throw "ARM Cortex-M4 object compile failed: $source"
    }
}
Write-Output 'ARM Cortex-M4 object compile: PASS'

$uartCallbackObject = Join-Path $armBuild 'stm32f4xx_hal_uart_callbacks.o'
$uartCallbackSymbols = (& $llvmNm $uartCallbackObject | Out-String)
$expectedWeakCallbacks = @(
    'HAL_UART_MspInit',
    'HAL_UART_MspDeInit',
    'HAL_UART_TxCpltCallback',
    'HAL_UART_TxHalfCpltCallback',
    'HAL_UART_RxCpltCallback',
    'HAL_UART_RxHalfCpltCallback',
    'HAL_UART_ErrorCallback',
    'HAL_UART_AbortCpltCallback',
    'HAL_UART_AbortTransmitCpltCallback',
    'HAL_UART_AbortReceiveCpltCallback'
)
foreach ($callback in $expectedWeakCallbacks) {
    if ($uartCallbackSymbols -notmatch "(?m)\sW\s+$callback\r?`$") {
        throw "UART callback is not weak in ARM object: $callback"
    }
}
Write-Output 'UART ARM weak callback symbols: PASS'

$timCallbackObject = Join-Path $armBuild 'stm32f4xx_hal_tim_callbacks.o'
$timCallbackSymbols = (& $llvmNm $timCallbackObject | Out-String)
$expectedTimWeakCallbacks = @(
    'HAL_TIM_Base_MspInit',
    'HAL_TIM_Base_MspDeInit',
    'HAL_TIM_OC_MspInit',
    'HAL_TIM_OC_MspDeInit',
    'HAL_TIM_PWM_MspInit',
    'HAL_TIM_PWM_MspDeInit',
    'HAL_TIM_IC_MspInit',
    'HAL_TIM_IC_MspDeInit',
    'HAL_TIM_OnePulse_MspInit',
    'HAL_TIM_OnePulse_MspDeInit',
    'HAL_TIM_PeriodElapsedCallback',
    'HAL_TIM_OC_DelayElapsedCallback',
    'HAL_TIM_IC_CaptureCallback',
    'HAL_TIM_PWM_PulseFinishedCallback',
    'HAL_TIM_TriggerCallback',
    'HAL_TIM_ErrorCallback'
)
foreach ($callback in $expectedTimWeakCallbacks) {
    if ($timCallbackSymbols -notmatch "(?m)\sW\s+$callback\r?`$") {
        throw "TIM callback is not weak in ARM object: $callback"
    }
}
Write-Output 'TIM ARM weak callback symbols: PASS'

$adcCallbackObject = Join-Path $armBuild 'stm32f4xx_hal_adc_callbacks.o'
$adcCallbackSymbols = (& $llvmNm $adcCallbackObject | Out-String)
$expectedAdcWeakCallbacks = @(
    'HAL_ADC_MspInit',
    'HAL_ADC_MspDeInit',
    'HAL_ADC_ConvCpltCallback',
    'HAL_ADC_ConvHalfCpltCallback',
    'HAL_ADC_ErrorCallback'
)
foreach ($callback in $expectedAdcWeakCallbacks) {
    if ($adcCallbackSymbols -notmatch "(?m)\sW\s+$callback\r?`$") {
        throw "ADC callback is not weak in ARM object: $callback"
    }
}
Write-Output 'ADC ARM weak callback symbols: PASS'

$i2cCallbackObject = Join-Path $armBuild 'stm32f4xx_hal_i2c_callbacks.o'
$i2cCallbackSymbols = (& $llvmNm $i2cCallbackObject | Out-String)
foreach ($callback in @(
    'HAL_I2C_MspInit',
    'HAL_I2C_MspDeInit',
    'HAL_I2C_MasterTxCpltCallback',
    'HAL_I2C_MasterRxCpltCallback',
    'HAL_I2C_MemTxCpltCallback',
    'HAL_I2C_MemRxCpltCallback',
    'HAL_I2C_ErrorCallback'
)) {
    if ($i2cCallbackSymbols -notmatch "(?m)\sW\s+$callback\r?`$") {
        throw "I2C callback is not weak in ARM object: $callback"
    }
}
Write-Output 'I2C ARM weak callback symbols: PASS'

$spiCallbackObject = Join-Path $armBuild 'stm32f4xx_hal_spi_callbacks.o'
$spiCallbackSymbols = (& $llvmNm $spiCallbackObject | Out-String)
foreach ($callback in @(
    'HAL_SPI_MspInit',
    'HAL_SPI_MspDeInit',
    'HAL_SPI_TxCpltCallback',
    'HAL_SPI_RxCpltCallback',
    'HAL_SPI_TxRxCpltCallback',
    'HAL_SPI_TxHalfCpltCallback',
    'HAL_SPI_RxHalfCpltCallback',
    'HAL_SPI_TxRxHalfCpltCallback',
    'HAL_SPI_ErrorCallback'
)) {
    if ($spiCallbackSymbols -notmatch "(?m)\sW\s+$callback\r?`$") {
        throw "SPI callback is not weak in ARM object: $callback"
    }
}
Write-Output 'SPI ARM weak callback symbols: PASS'

$rccCallbackObject = Join-Path $armBuild 'stm32f4xx_hal_rcc_callbacks.o'
$rccCallbackSymbols = (& $llvmNm $rccCallbackObject | Out-String)
if ($rccCallbackSymbols -notmatch "(?m)\sW\s+HAL_RCC_CSSCallback\r?`$") {
    throw 'RCC CSS callback is not weak'
}
Write-Output 'RCC ARM weak callback symbol: PASS'

$flashCallbackObject = Join-Path $armBuild 'stm32f4xx_hal_flash_callbacks.o'
$flashCallbackSymbols = (& $llvmNm $flashCallbackObject | Out-String)
foreach ($callback in @('HAL_FLASH_EndOfOperationCallback',
                         'HAL_FLASH_OperationErrorCallback')) {
    if ($flashCallbackSymbols -notmatch "(?m)\sW\s+$callback\r?`$") {
        throw "FLASH callback is not weak: $callback"
    }
}
Write-Output 'FLASH ARM weak callback symbols: PASS'

$timLinkBuild = Join-Path $armBuild 'tim_link'
New-Item -ItemType Directory -Force -Path $timLinkBuild | Out-Null
$timLinkSources = @(
    (Join-Path $project 'Source\stm32f4xx_hal.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_dma.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_tim.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_tim_ex.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_tim_callbacks.c'),
    (Join-Path $project 'Port\gd32_core_port.c'),
    (Join-Path $project 'Port\gd32_dma_port.c'),
    (Join-Path $project 'Port\gd32_timer_port.c'),
    (Join-Path $project 'Port\gd32_timer_dma_port.c'),
    (Join-Path $project 'Port\stm32_timer_trigger_map.c'),
    (Join-Path $project 'Port\gd32_instance_map.c'),
    (Join-Path $PSScriptRoot 'target_tim_link_smoke.c'),
    (Join-Path $vendorSplSource 'gd32f403_timer.c'),
    (Join-Path $vendorSplSource 'gd32f403_dma.c'),
    (Join-Path $vendorSplSource 'gd32f403_rcu.c')
)
$timLinkObjects = @()
for ($index = 0; $index -lt $timLinkSources.Count; ++$index) {
    $objectPath = Join-Path $timLinkBuild ("timer_link_$index.o")
    & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
        -ffreestanding -ffunction-sections -fdata-sections `
        -Wall -Wextra -Werror @armIncludes `
        -c $timLinkSources[$index] -o $objectPath
    if ($LASTEXITCODE -ne 0) {
        throw "TIM target-link object compile failed: $($timLinkSources[$index])"
    }
    $timLinkObjects += $objectPath
}
$timLinkElf = Join-Path $timLinkBuild 'target_tim_link_smoke.elf'
$timLinkExit = 1
for ($attempt = 0; $attempt -lt 3; ++$attempt) {
    if (Test-Path -LiteralPath $timLinkElf) {
        Remove-Item -LiteralPath $timLinkElf -Force
    }
    $timLinkOutput = & $ldLld -m armelf --threads=1 -e TargetSmoke --gc-sections @timLinkObjects -o $timLinkElf 2>&1
    $timLinkExit = $LASTEXITCODE
    if ($timLinkExit -eq 0) { break }
}
if ($timLinkExit -ne 0) {
    throw "TIM compatibility layer did not link with official GD32 TIMER/RCU sources: $($timLinkOutput | Out-String)"
}
$timLinkSymbols = (& $llvmNm $timLinkElf | Out-String)
foreach ($symbol in @('TargetSmoke', 'HAL_TIM_Base_Init', 'timer_init')) {
    if ($timLinkSymbols -notmatch "(?m)\sT\s+$symbol\r?`$") {
        throw "TIM target-link symbol missing: $symbol"
    }
}
Write-Output 'TIM official GD32 SPL target link: PASS'

$timDMARequestPortHostExe = Join-Path $build 'test_tim_dma_request_port_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -Wno-unused-parameter -Wno-int-to-pointer-cast `
    -I (Join-Path $PSScriptRoot 'ArmShims') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    -I $VendorRoot `
    -I $vendorCmsis `
    -I $vendorDeviceInclude `
    -I $vendorSplInclude `
    (Join-Path $project 'Port\gd32_timer_dma_port.c') `
    (Join-Path $PSScriptRoot 'test_tim_dma_request_port_host.c') `
    -o $timDMARequestPortHostExe
if ($LASTEXITCODE -ne 0) {
    throw "TIM DMA request vendor Port test build failed: $LASTEXITCODE"
}
& $timDMARequestPortHostExe
if ($LASTEXITCODE -ne 0) {
    throw "TIM DMA request vendor Port test failed: $LASTEXITCODE"
}

$adcLinkBuild = Join-Path $armBuild 'adc_link'
New-Item -ItemType Directory -Force -Path $adcLinkBuild | Out-Null
$adcLinkSources = @(
    (Join-Path $project 'Source\stm32f4xx_hal.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_dma.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_adc.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_adc_callbacks.c'),
    (Join-Path $project 'Port\gd32_core_port.c'),
    (Join-Path $project 'Port\gd32_dma_port.c'),
    (Join-Path $project 'Port\gd32_adc_port.c'),
    (Join-Path $project 'Port\gd32_instance_map.c'),
    (Join-Path $project 'Port\gd32_rcc_port.c'),
    (Join-Path $PSScriptRoot 'target_adc_link_smoke.c'),
    (Join-Path $vendorSplSource 'gd32f403_adc.c'),
    (Join-Path $vendorSplSource 'gd32f403_dma.c'),
    (Join-Path $vendorSplSource 'gd32f403_gpio.c'),
    (Join-Path $vendorSplSource 'gd32f403_rcu.c')
)
$adcLinkObjects = @()
for ($index = 0; $index -lt $adcLinkSources.Count; ++$index) {
    $objectPath = Join-Path $adcLinkBuild ("adc_link_$index.o")
    & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
        -ffreestanding -ffunction-sections -fdata-sections `
        -Wall -Wextra -Werror @armIncludes `
        -c $adcLinkSources[$index] -o $objectPath
    if ($LASTEXITCODE -ne 0) {
        throw "ADC target-link object compile failed: $($adcLinkSources[$index])"
    }
    $adcLinkObjects += $objectPath
}
$adcLinkElf = Join-Path $adcLinkBuild 'target_adc_link_smoke.elf'
$adcLinkExit = 1
for ($attempt = 0; $attempt -lt 3; ++$attempt) {
    if (Test-Path -LiteralPath $adcLinkElf) {
        Remove-Item -LiteralPath $adcLinkElf -Force
    }
    $adcLinkOutput = & $ldLld -m armelf --threads=1 -e TargetSmoke --gc-sections @adcLinkObjects -o $adcLinkElf 2>&1
    $adcLinkExit = $LASTEXITCODE
    if ($adcLinkExit -eq 0) { break }
}
if ($adcLinkExit -ne 0) {
    throw "ADC compatibility layer did not link with official GD32 ADC/DMA/GPIO/RCU sources: $($adcLinkOutput | Out-String)"
}
$adcLinkSymbols = (& $llvmNm $adcLinkElf | Out-String)
foreach ($symbol in @('TargetSmoke', 'HAL_ADC_Init',
                      'GD32_HAL_ADC_EnableAndCalibrate')) {
    if ($adcLinkSymbols -notmatch "(?m)\sT\s+$symbol\r?`$") {
        throw "ADC target-link symbol missing: $symbol"
    }
}
Write-Output 'ADC official GD32 SPL target link: PASS'

$phase7LinkBuild = Join-Path $armBuild 'i2c_spi_link'
New-Item -ItemType Directory -Force -Path $phase7LinkBuild | Out-Null
$phase7LinkSources = @(
    (Join-Path $project 'Source\stm32f4xx_hal.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_dma.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_i2c.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_i2c_callbacks.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_spi.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_spi_callbacks.c'),
    (Join-Path $project 'Port\gd32_core_port.c'),
    (Join-Path $project 'Port\gd32_dma_port.c'),
    (Join-Path $project 'Port\gd32_instance_map.c'),
    (Join-Path $project 'Port\gd32_rcc_port.c'),
    (Join-Path $project 'Port\gd32_i2c_port.c'),
    (Join-Path $project 'Port\gd32_i2c_flag_port.c'),
    (Join-Path $project 'Port\gd32_spi_port.c'),
    (Join-Path $PSScriptRoot 'target_i2c_spi_link_smoke.c'),
    (Join-Path $vendorSplSource 'gd32f403_i2c.c'),
    (Join-Path $vendorSplSource 'gd32f403_spi.c'),
    (Join-Path $vendorSplSource 'gd32f403_dma.c'),
    (Join-Path $vendorSplSource 'gd32f403_rcu.c')
)
$phase7LinkObjects = @()
for ($index = 0; $index -lt $phase7LinkSources.Count; ++$index) {
    $objectPath = Join-Path $phase7LinkBuild ("i2c_spi_link_$index.o")
    & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
        -ffreestanding -ffunction-sections -fdata-sections `
        -Wall -Wextra -Werror @armIncludes `
        -c $phase7LinkSources[$index] -o $objectPath
    if ($LASTEXITCODE -ne 0) {
        throw "I2C/SPI target-link object compile failed: $($phase7LinkSources[$index])"
    }
    $phase7LinkObjects += $objectPath
}
$phase7LinkElf = Join-Path $phase7LinkBuild 'target_i2c_spi_link_smoke.elf'
$phase7LinkExit = 1
for ($attempt = 0; $attempt -lt 3; ++$attempt) {
    if (Test-Path -LiteralPath $phase7LinkElf) {
        Remove-Item -LiteralPath $phase7LinkElf -Force
    }
    $phase7LinkOutput = & $ldLld -m armelf --threads=1 -e TargetSmoke --gc-sections @phase7LinkObjects -o $phase7LinkElf 2>&1
    $phase7LinkExit = $LASTEXITCODE
    if ($phase7LinkExit -eq 0) { break }
}
if ($phase7LinkExit -ne 0) {
    throw "I2C/SPI compatibility layer did not link with official GD32 I2C/SPI/DMA/RCU sources: $($phase7LinkOutput | Out-String)"
}
$phase7LinkSymbols = (& $llvmNm $phase7LinkElf | Out-String)
foreach ($symbol in @('TargetSmoke', 'HAL_I2C_Init', 'HAL_SPI_Init',
                      'i2c_start_on_bus', 'spi_init')) {
    if ($phase7LinkSymbols -notmatch "(?m)\sT\s+$symbol\r?`$") {
        throw "I2C/SPI target-link symbol missing: $symbol"
    }
}
Write-Output 'I2C/SPI official GD32 SPL target link: PASS'

$phase8LinkBuild = Join-Path $armBuild 'rcc_exti_flash_link'
New-Item -ItemType Directory -Force -Path $phase8LinkBuild | Out-Null
$phase8LinkSources = @(
    (Join-Path $project 'Source\stm32f4xx_hal.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_cortex.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_rcc.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_rcc_callbacks.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_exti.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_flash.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_flash_ex.c'),
    (Join-Path $project 'Source\stm32f4xx_hal_flash_callbacks.c'),
    (Join-Path $project 'Port\gd32_core_port.c'),
    (Join-Path $project 'Port\gd32_instance_map.c'),
    (Join-Path $project 'Port\gd32_rcc_port.c'),
    (Join-Path $project 'Port\gd32_exti_port.c'),
    (Join-Path $project 'Port\gd32_flash_port.c'),
    (Join-Path $project 'Port\gd32_tick_port.c'),
    (Join-Path $PSScriptRoot 'target_rcc_exti_flash_link_smoke.c'),
    (Join-Path $vendorCmsis 'GD\GD32F403\Source\system_gd32f403.c'),
    (Join-Path $vendorSplSource 'gd32f403_rcu.c'),
    (Join-Path $vendorSplSource 'gd32f403_fmc.c'),
    (Join-Path $vendorSplSource 'gd32f403_exti.c'),
    (Join-Path $vendorSplSource 'gd32f403_gpio.c'),
    (Join-Path $vendorSplSource 'gd32f403_dma.c')
)
$phase8LinkObjects = @()
for ($index = 0; $index -lt $phase8LinkSources.Count; ++$index) {
    $objectPath = Join-Path $phase8LinkBuild ("rcc_exti_flash_link_$index.o")
    & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
        -ffreestanding -ffunction-sections -fdata-sections `
        -Wall -Wextra -Werror @armIncludes `
        -c $phase8LinkSources[$index] -o $objectPath
    if ($LASTEXITCODE -ne 0) {
        throw "RCC/EXTI/FLASH target-link object compile failed: $($phase8LinkSources[$index])"
    }
    $phase8LinkObjects += $objectPath
}
$phase8LinkElf = Join-Path $phase8LinkBuild 'target_rcc_exti_flash_link_smoke.elf'
$phase8LinkExit = 1
for ($attempt = 0; $attempt -lt 3; ++$attempt) {
    if (Test-Path -LiteralPath $phase8LinkElf) {
        Remove-Item -LiteralPath $phase8LinkElf -Force
    }
    $phase8LinkOutput = & $ldLld -m armelf --threads=1 -e TargetSmoke --gc-sections `
        @phase8LinkObjects -o $phase8LinkElf 2>&1
    $phase8LinkExit = $LASTEXITCODE
    if ($phase8LinkExit -eq 0) { break }
}
if ($phase8LinkExit -ne 0) {
    throw "RCC/EXTI/FLASH compatibility layer did not link with official GD32 sources: $($phase8LinkOutput | Out-String)"
}
$phase8LinkSymbols = (& $llvmNm $phase8LinkElf | Out-String)
foreach ($symbol in @('TargetSmoke', 'HAL_RCC_ClockConfig',
                      'HAL_EXTI_SetConfigLine', 'HAL_FLASH_Program',
                      'rcu_clock_freq_get', 'exti_init',
                      'fmc_word_program', 'fmc_page_erase')) {
    if ($phase8LinkSymbols -notmatch "(?m)\sT\s+$symbol\r?`$") {
        throw "RCC/EXTI/FLASH target-link symbol missing: $symbol"
    }
}
Write-Output 'RCC/EXTI/FLASH official GD32 SPL target link: PASS'

$registerCompileSources = Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'Compile') `
    -Filter '*.c' | Where-Object { $_.Name -notmatch 'negative' }
foreach ($source in $registerCompileSources) {
    & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
        -ffreestanding -Wall -Wextra -Werror @armIncludes -fsyntax-only $source.FullName
    if ($LASTEXITCODE -ne 0) {
        throw "Register compatibility compile failed: $($source.Name)"
    }
}
Write-Output 'Register compatibility positive compile: PASS'

$registerNegativeTests = @(
    @{ File = 'register_dma_negative.c'; Pattern = 'incomplete definition' },
    @{ File = 'register_gpio_model_negative.c'; Pattern = 'MODER' },
    @{ File = 'register_tim_dma_burst_negative.c'; Pattern = 'DCR' },
    @{ File = 'register_adc_start_negative.c'; Pattern = 'ADC_CR2_SWSTART' },
    @{ File = 'register_unsupported_bit_negative.c'; Pattern = 'ADC_SR_OVR' }
)
foreach ($test in $registerNegativeTests) {
    $source = Join-Path (Join-Path $PSScriptRoot 'Compile') $test.File
    $output = & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
        -ffreestanding @armIncludes -fsyntax-only $source 2>&1
    if (($LASTEXITCODE -eq 0) -or
        (($output | Out-String) -notmatch $test.Pattern)) {
        throw "Register compatibility negative test unexpectedly passed: $($test.File)"
    }
}
Write-Output 'Register compatibility negative compile: PASS'

$unsupportedSource = Join-Path $PSScriptRoot 'test_unsupported_compile.c'
$unsupportedOutput = & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
    -ffreestanding @armIncludes -fsyntax-only $unsupportedSource 2>&1
$unsupportedExit = $LASTEXITCODE
if (($unsupportedExit -eq 0) -or
    (($unsupportedOutput | Out-String) -notmatch 'GD32_HAL_UNSUPPORTED_I2C3_ON_GD32F403RET6')) {
    throw 'Unsupported-feature compile guard did not fail as designed'
}
Write-Output 'Unsupported-feature compile guard: PASS'

$extiUnsupportedSource = Join-Path $PSScriptRoot 'test_exti_unsupported_compile.c'
$extiUnsupportedOutput = & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
    -ffreestanding @armIncludes -fsyntax-only $extiUnsupportedSource 2>&1
$extiUnsupportedExit = $LASTEXITCODE
if (($extiUnsupportedExit -eq 0) -or
    (($extiUnsupportedOutput | Out-String) -notmatch 'EXTI_LINE_19')) {
    throw 'Unavailable GD32 EXTI line compile guard did not fail as designed'
}
Write-Output 'Unavailable EXTI line compile guard: PASS'

$uartDMASource = Join-Path $PSScriptRoot 'test_uart_dma_compile.c'
& $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
    -ffreestanding -Wall -Wextra -Werror @armIncludes -fsyntax-only $uartDMASource
if ($LASTEXITCODE -ne 0) {
    throw 'UART DMA public API compile failed'
}
Write-Output 'UART DMA public API compile: PASS'

$dmaStreamUnsupportedSource = Join-Path $PSScriptRoot 'test_dma_stream_unsupported_compile.c'
$dmaStreamUnsupportedOutput = & $clang --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 `
    -ffreestanding @armIncludes -fsyntax-only $dmaStreamUnsupportedSource 2>&1
$dmaStreamUnsupportedExit = $LASTEXITCODE
if (($dmaStreamUnsupportedExit -eq 0) -or
    (($dmaStreamUnsupportedOutput | Out-String) -notmatch 'DMA1_Stream0')) {
    throw 'STM32 DMA Stream address guard did not fail as designed'
}
Write-Output 'STM32 DMA Stream address guard: PASS'

& $clangTidy @compatSources `
    --checks='-*,clang-analyzer-*' `
    --warnings-as-errors='clang-analyzer-*' `
    -- `
    --target=arm-none-eabi -mcpu=cortex-m4 -mthumb -std=c11 -ffreestanding `
    @armIncludes
if ($LASTEXITCODE -ne 0) {
    throw "Clang static analysis failed: $LASTEXITCODE"
}
Write-Output 'Clang Analyzer: PASS'

$instanceMapHostExe = Join-Path $build 'test_instance_map_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_instance_map_host.c') `
    -o $instanceMapHostExe
if ($LASTEXITCODE -ne 0) {
    throw "Instance map host test build failed: $LASTEXITCODE"
}
& $instanceMapHostExe
if ($LASTEXITCODE -ne 0) {
    throw "Instance map host test failed: $LASTEXITCODE"
}

$hostExe = Join-Path $build 'test_gpio_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_gpio.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_gpio_host.c') `
    -o $hostExe
if ($LASTEXITCODE -ne 0) {
    throw "Host test build failed: $LASTEXITCODE"
}

& $hostExe
if ($LASTEXITCODE -ne 0) {
    throw "Host test failed: $LASTEXITCODE"
}

$coreHostExe = Join-Path $build 'test_core_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal.c') `
    (Join-Path $project 'Source\stm32f4xx_hal_cortex.c') `
    (Join-Path $project 'Port\gd32_tick_port.c') `
    (Join-Path $project 'Port\gd32_irq_port.c') `
    (Join-Path $PSScriptRoot 'test_core_host.c') `
    -o $coreHostExe
if ($LASTEXITCODE -ne 0) {
    throw "Core host test build failed: $LASTEXITCODE"
}

& $coreHostExe
if ($LASTEXITCODE -ne 0) {
    throw "Core host test failed: $LASTEXITCODE"
}

$uartHostExe = Join-Path $build 'test_uart_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_uart.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_uart_host.c') `
    -o $uartHostExe
if ($LASTEXITCODE -ne 0) {
    throw "UART host test build failed: $LASTEXITCODE"
}

& $uartHostExe
if ($LASTEXITCODE -ne 0) {
    throw "UART host test failed: $LASTEXITCODE"
}

$dmaHostExe = Join-Path $build 'test_dma_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_dma.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_dma_host.c') `
    -o $dmaHostExe
if ($LASTEXITCODE -ne 0) {
    throw "DMA host test build failed: $LASTEXITCODE"
}

& $dmaHostExe
if ($LASTEXITCODE -ne 0) {
    throw "DMA host test failed: $LASTEXITCODE"
}

$dmaPortHostExe = Join-Path $build 'test_dma_port_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Port\gd32_dma_port.c') `
    (Join-Path $PSScriptRoot 'test_dma_port_host.c') `
    -o $dmaPortHostExe
if ($LASTEXITCODE -ne 0) {
    throw "DMA Port host test build failed: $LASTEXITCODE"
}

& $dmaPortHostExe
if ($LASTEXITCODE -ne 0) {
    throw "DMA Port host test failed: $LASTEXITCODE"
}

$timHostExe = Join-Path $build 'test_tim_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_tim.c') `
    (Join-Path $project 'Source\stm32f4xx_hal_tim_ex.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_tim_host.c') `
    -o $timHostExe
if ($LASTEXITCODE -ne 0) {
    throw "TIM host test build failed: $LASTEXITCODE"
}

& $timHostExe
if ($LASTEXITCODE -ne 0) {
    throw "TIM host test failed: $LASTEXITCODE"
}

$timItrHostExe = Join-Path $build 'test_tim_itr_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Port\stm32_timer_trigger_map.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_tim_itr_host.c') `
    -o $timItrHostExe
if ($LASTEXITCODE -ne 0) {
    throw "TIM ITR host test build failed: $LASTEXITCODE"
}

& $timItrHostExe
if ($LASTEXITCODE -ne 0) {
    throw "TIM ITR host test failed: $LASTEXITCODE"
}

$adcHostExe = Join-Path $build 'test_adc_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_adc.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_adc_host.c') `
    -o $adcHostExe
if ($LASTEXITCODE -ne 0) {
    throw "ADC host test build failed: $LASTEXITCODE"
}

& $adcHostExe
if ($LASTEXITCODE -ne 0) {
    throw "ADC host test failed: $LASTEXITCODE"
}

$adcCalibrationPortHostExe = Join-Path $build 'test_adc_calibration_port_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'ADCMocks') `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Port\gd32_adc_port.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_adc_calibration_port_host.c') `
    -o $adcCalibrationPortHostExe
if ($LASTEXITCODE -ne 0) {
    throw "ADC calibration Port host test build failed: $LASTEXITCODE"
}

& $adcCalibrationPortHostExe
if ($LASTEXITCODE -ne 0) {
    throw "ADC calibration Port host test failed: $LASTEXITCODE"
}

$i2cHostExe = Join-Path $build 'test_i2c_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_i2c.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_i2c_host.c') `
    -o $i2cHostExe
if ($LASTEXITCODE -ne 0) {
    throw "I2C host test build failed: $LASTEXITCODE"
}
& $i2cHostExe
if ($LASTEXITCODE -ne 0) {
    throw "I2C host test failed: $LASTEXITCODE"
}

$i2cFlagPortHostExe = Join-Path $build 'test_i2c_flag_port_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -Wno-unused-parameter -Wno-int-to-pointer-cast `
    -I (Join-Path $PSScriptRoot 'I2CPortMocks') `
    -I (Join-Path $PSScriptRoot 'ArmShims') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    -I $VendorRoot `
    -I $vendorCmsis `
    -I $vendorDeviceInclude `
    -I $vendorSplInclude `
    (Join-Path $project 'Port\gd32_i2c_flag_port.c') `
    (Join-Path $PSScriptRoot 'test_i2c_flag_port_host.c') `
    -o $i2cFlagPortHostExe
if ($LASTEXITCODE -ne 0) {
    throw "I2C side-effect-safe Port flag test build failed: $LASTEXITCODE"
}
& $i2cFlagPortHostExe
if ($LASTEXITCODE -ne 0) {
    throw "I2C side-effect-safe Port flag test failed: $LASTEXITCODE"
}

$spiHostExe = Join-Path $build 'test_spi_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_spi.c') `
    (Join-Path $project 'Port\gd32_instance_map.c') `
    (Join-Path $PSScriptRoot 'test_spi_host.c') `
    -o $spiHostExe
if ($LASTEXITCODE -ne 0) {
    throw "SPI host test build failed: $LASTEXITCODE"
}
& $spiHostExe
if ($LASTEXITCODE -ne 0) {
    throw "SPI host test failed: $LASTEXITCODE"
}

$rccHostExe = Join-Path $build 'test_rcc_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_rcc.c') `
    (Join-Path $PSScriptRoot 'test_rcc_host.c') `
    -o $rccHostExe
if ($LASTEXITCODE -ne 0) {
    throw "RCC host test build failed: $LASTEXITCODE"
}
& $rccHostExe
if ($LASTEXITCODE -ne 0) {
    throw "RCC host test failed: $LASTEXITCODE"
}

$extiHostExe = Join-Path $build 'test_exti_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_exti.c') `
    (Join-Path $PSScriptRoot 'test_exti_host.c') `
    -o $extiHostExe
if ($LASTEXITCODE -ne 0) {
    throw "EXTI host test build failed: $LASTEXITCODE"
}
& $extiHostExe
if ($LASTEXITCODE -ne 0) {
    throw "EXTI host test failed: $LASTEXITCODE"
}

$flashHostExe = Join-Path $build 'test_flash_host.exe'
& $gcc -std=c11 -Wall -Wextra -Werror `
    -I (Join-Path $PSScriptRoot 'HostMocks') `
    -I (Join-Path $project 'Include') `
    -I (Join-Path $project 'Port') `
    (Join-Path $project 'Source\stm32f4xx_hal_flash.c') `
    (Join-Path $project 'Source\stm32f4xx_hal_flash_ex.c') `
    (Join-Path $PSScriptRoot 'test_flash_host.c') `
    -o $flashHostExe
if ($LASTEXITCODE -ne 0) {
    throw "FLASH host test build failed: $LASTEXITCODE"
}
& $flashHostExe
if ($LASTEXITCODE -ne 0) {
    throw "FLASH host test failed: $LASTEXITCODE"
}

Write-Output 'All checks: PASS'
