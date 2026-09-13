param(
    [string]$IarPath
)

$ErrorActionPreference = 'Stop'

$project = Split-Path -Parent $PSScriptRoot
$workspace = Split-Path -Parent $project
$build = Join-Path $PSScriptRoot 'build\iar'

function Find-IarTool {
    param(
        [string]$Name,
        [string[]]$Roots
    )

    foreach ($root in $Roots) {
        if ([string]::IsNullOrWhiteSpace($root) -or
            -not (Test-Path -LiteralPath $root)) {
            continue
        }
        $item = Get-Item -LiteralPath $root
        if (-not $item.PSIsContainer -and $item.Name -ieq $Name) {
            return $item.FullName
        }
        if ($item.PSIsContainer) {
            foreach ($candidate in @(
                (Join-Path $item.FullName $Name),
                (Join-Path $item.FullName "bin\$Name"),
                (Join-Path $item.FullName "arm\bin\$Name")
            )) {
                if (Test-Path -LiteralPath $candidate) {
                    return (Get-Item -LiteralPath $candidate).FullName
                }
            }
            $match = Get-ChildItem -LiteralPath $item.FullName -Filter $Name `
                -File -Recurse -ErrorAction SilentlyContinue |
                Select-Object -First 1
            if ($null -ne $match) {
                return $match.FullName
            }
        }
    }
    return $null
}

$searchRoots = @()
if (-not [string]::IsNullOrWhiteSpace($IarPath)) {
    $searchRoots += $IarPath
}
$searchRoots += @(
    'C:\Program Files\IAR Systems',
    'C:\Program Files (x86)\IAR Systems',
    'D:\IAR Systems'
)

$iccarm = Find-IarTool -Name 'iccarm.exe' -Roots $searchRoots
if ($null -eq $iccarm) {
    Write-Output 'IAR checks: SKIPPED - IAR compiler not found'
    exit 0
}

$iarBin = Split-Path -Parent $iccarm
$toolRoots = @($iarBin) + $searchRoots
$iasmarm = Find-IarTool -Name 'iasmarm.exe' -Roots $toolRoots
$ilinkarm = Find-IarTool -Name 'ilinkarm.exe' -Roots $toolRoots
if (($null -eq $iasmarm) -or ($null -eq $ilinkarm)) {
    throw 'IAR installation is incomplete: iasmarm.exe or ilinkarm.exe not found'
}

Write-Output "IAR compiler: $iccarm"
Write-Output "IAR assembler: $iasmarm"
Write-Output "IAR linker: $ilinkarm"

New-Item -ItemType Directory -Force -Path $build | Out-Null

$includes = @(
    (Join-Path $project 'Include'),
    (Join-Path $project 'Port'),
    (Join-Path $workspace 'GD32F403RET6'),
    (Join-Path $workspace 'GD32F403RET6\CMSIS'),
    (Join-Path $workspace 'GD32F403RET6\CMSIS\GD\GD32F403\Include'),
    (Join-Path $workspace 'GD32F403RET6\GD32F403_standard_peripheral\Include')
)

foreach ($include in $includes) {
    if (-not (Test-Path -LiteralPath $include)) {
        throw "Required include directory not found: $include"
    }
}

$sources = @(
    Get-ChildItem -LiteralPath (Join-Path $project 'Source') -Filter '*.c' -File
    Get-ChildItem -LiteralPath (Join-Path $project 'Port') -Filter '*.c' -File
)
$sources += Get-Item -LiteralPath (Join-Path $PSScriptRoot 'target_tim_link_smoke.c')

$commonArgs = @(
    '--cpu=Cortex-M4',
    '--thumb',
    '--endian=little',
    '--c11',
    '--warnings_are_errors',
    '--silent'
)
foreach ($include in $includes) {
    $commonArgs += "-I$include"
}

foreach ($source in $sources) {
    $scope = if ($source.DirectoryName -eq (Join-Path $project 'Source')) {
        'source'
    } elseif ($source.DirectoryName -eq (Join-Path $project 'Port')) {
        'port'
    } else {
        'smoke'
    }
    $object = Join-Path $build ($scope + '_' + $source.BaseName + '.o')
    & $iccarm @commonArgs $source.FullName -o $object
    if ($LASTEXITCODE -ne 0) {
        throw "IAR compile failed: $($source.FullName)"
    }
}

Write-Output 'IAR Source/Port compile: PASS'
Write-Output 'IAR TIM target smoke object compile: PASS'
Write-Output 'IAR target link: SKIPPED - board startup and ICF are project-owned'
Write-Output 'IAR checks: PASS'
