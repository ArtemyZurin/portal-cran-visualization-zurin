param([string]$EngineRoot = 'E:\UnrealEngine\UE_5.7')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$projectFile = Join-Path $projectRoot 'PortalCrane.uproject'
$reportPath = Join-Path $projectRoot 'Saved\TestReport'
& $editor $projectFile -unattended -nop4 -NullRHI '-ExecCmds=Automation RunTests PortalCrane' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$reportPath" -stdout -FullStdOutLogOutput
if ($LASTEXITCODE -ne 0) { throw 'UE automation failed. See Saved/Logs and Saved/TestReport.' }
$report = Join-Path $reportPath 'index.json'
if (-not (Test-Path -LiteralPath $report)) { throw 'No automation report was produced.' }
$results = Get-Content -LiteralPath $report -Raw | ConvertFrom-Json
if ($results.failed -gt 0 -or $results.succeeded -lt 2) { throw 'Expected both PortalCrane automation tests to pass.' }
