param(
    [string]$BuildDir = "",
    [switch]$DeleteLegacyExe
)

$ErrorActionPreference = "Stop"
$workspaceScript = Join-Path $PSScriptRoot 'windows_workspace.ps1'
. $workspaceScript
$guardScript = Join-Path $PSScriptRoot 'windows_runtime_guard.ps1'
. $guardScript

if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    try {
        $ws = Get-WindowsWorkspace
        $BuildDir = Join-Path $ws.ComparisonRoot 'windows_build'
    } catch {
        $repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).ProviderPath
        $BuildDir = Join-Path $repoRoot 'comparison\windows_build'
    }
}

$killed = Stop-MB3DRelevantProcesses -BuildDir $BuildDir

foreach ($entry in $killed) {
    Write-Output ("stopped=" + $entry)
}

if ($DeleteLegacyExe) {
    $legacyExe = Join-Path $BuildDir "Mandelbulb3D.exe"
    if (Test-Path -LiteralPath $legacyExe) {
        Remove-Item -LiteralPath $legacyExe -Force -ErrorAction Stop
        Write-Output ("removed=" + $legacyExe)
    }
}
