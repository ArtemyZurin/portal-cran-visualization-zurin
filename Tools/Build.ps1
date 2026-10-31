param([string]$EngineRoot = 'E:\UnrealEngine\UE_5.7')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'PortalCrane.uproject'
$buildTool = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path -LiteralPath $buildTool)) { throw "Unreal Engine not found: $EngineRoot. Supply -EngineRoot." }
& $buildTool PortalCraneEditor Win64 Development "-Project=$projectFile" -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) { throw 'UE build failed. Check MSVC v143, Windows SDK, .NET Framework 4.8 SDK/Targeting Pack and the UnrealBuildTool log.' }
