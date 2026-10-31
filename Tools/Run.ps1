param([string]$EngineRoot = 'E:\UnrealEngine\UE_5.7', [switch]$Screenshot, [ValidateSet('normal','overload','wind','cycle')][string]$Scenario = 'normal')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
$projectFile = Join-Path $projectRoot 'PortalCrane.uproject'
if (-not (Test-Path -LiteralPath (Join-Path $projectRoot 'Binaries\Win64\UnrealEditor-PortalCrane.dll'))) {
    & (Join-Path $PSScriptRoot 'Build.ps1') -EngineRoot $EngineRoot
}
$launchArgs = @(('"' + $projectFile + '"'), '/Engine/Maps/Entry', '-game', '-windowed', '-ResX=1440', '-ResY=900', "-CraneScenario=$Scenario")
if ($Screenshot) { $launchArgs += '-CraneScreenshot' }
# This is the interactive demo window explicitly requested by the user.
Start-Process -FilePath $editor -ArgumentList $launchArgs
