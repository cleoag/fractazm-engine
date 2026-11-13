function Get-WindowsWorkspace {
    param(
        [string]$RepoRoot = ((Resolve-Path (Join-Path $PSScriptRoot "..\..")).ProviderPath)
    )

    $configPath = Join-Path $PSScriptRoot 'windows_workspace.local.json'
    $workflowMode = if ($env:MB3D_WORKFLOW_MODE) { $env:MB3D_WORKFLOW_MODE } else { 'local-windows' }
    $pascalWorkRoot = $env:MB3D_PASCAL_WORK_ROOT

    if (Test-Path -LiteralPath $configPath) {
        $loaded = Get-Content -LiteralPath $configPath -Raw | ConvertFrom-Json
        if ($loaded.workflow_mode) { $workflowMode = $loaded.workflow_mode }
        if ($loaded.pascal_work_root) { $pascalWorkRoot = $loaded.pascal_work_root }
    }

    if ([string]::IsNullOrWhiteSpace($pascalWorkRoot)) {
        throw 'pascal_work_root must be set in windows_workspace.local.json or MB3D_PASCAL_WORK_ROOT'
    }

    [pscustomobject]@{
        WorkflowMode = $workflowMode
        RepoRoot = $RepoRoot
        PascalWorkRoot = $pascalWorkRoot
        ComparisonRoot = Join-Path $RepoRoot 'comparison'
    }
}
