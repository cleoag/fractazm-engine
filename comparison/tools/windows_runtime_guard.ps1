 $workspaceScript = Join-Path $PSScriptRoot 'windows_workspace.ps1'
 . $workspaceScript

 function Get-MB3DGuardWorkspace {
     try {
         return Get-WindowsWorkspace
     } catch {
         $repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).ProviderPath
         return [pscustomobject]@{
             WorkflowMode = if ($env:MB3D_WORKFLOW_MODE) { $env:MB3D_WORKFLOW_MODE } else { 'local-windows' }
             RepoRoot = $repoRoot
             PascalWorkRoot = $env:MB3D_PASCAL_WORK_ROOT
             LegacyWindowsRepoRoot = if ($env:MB3D_LEGACY_WINDOWS_REPO_ROOT) { $env:MB3D_LEGACY_WINDOWS_REPO_ROOT } else { '' }
             ComparisonRoot = Join-Path $repoRoot 'comparison'
         }
     }
 }

 function Get-MB3DGuardBuildDir {
     $ws = Get-MB3DGuardWorkspace
     return (Join-Path $ws.ComparisonRoot 'windows_build')
 }

function Get-MB3DGuardExcludedProcessIds {
    $excluded = [System.Collections.Generic.HashSet[int]]::new()
    $currentId = [int]$PID

    while ($currentId -gt 0) {
        if ($excluded.Contains($currentId)) {
            break
        }
        [void]$excluded.Add($currentId)

        try {
            $proc = Get-CimInstance Win32_Process -Filter ("ProcessId=" + $currentId)
        } catch {
            break
        }

        if (-not $proc -or $proc.ParentProcessId -le 0) {
            break
        }

        $currentId = [int]$proc.ParentProcessId
    }

    return $excluded
}

function Get-MB3DRelevantProcesses {
    param(
        [string]$BuildDir = $(Get-MB3DGuardBuildDir),
        [System.Collections.Generic.HashSet[int]]$ExcludeProcessIds = $(Get-MB3DGuardExcludedProcessIds)
    )

    Get-CimInstance Win32_Process | Where-Object {
        (-not $ExcludeProcessIds.Contains([int]$_.ProcessId)) -and (
            ($_.Name -like "Mandelbulb3D*.exe" -and $_.ExecutablePath -like ($BuildDir + "*")) -or
            (
                ($_.Name -eq "cmd.exe" -or $_.Name -eq "powershell.exe" -or $_.Name -eq "pwsh.exe") -and
                $_.CommandLine -and (
                    $_.CommandLine -like "*Mandelbulb3D.exe*" -or
                    $_.CommandLine -like "*build_pascal_windows.ps1*" -or
                    $_.CommandLine -like "*clean_windows_runtime.ps1*" -or
                    $_.CommandLine -like "*render_one_windows.ps1*"
                )
            )
        )
    }
}

function Stop-MB3DRelevantProcesses {
    param(
        [string]$BuildDir = $(Get-MB3DGuardBuildDir),
        [int]$Passes = 5,
        [System.Collections.Generic.HashSet[int]]$ExcludeProcessIds = $(Get-MB3DGuardExcludedProcessIds)
    )

    $killed = @()
    for ($pass = 0; $pass -lt $Passes; $pass++) {
        $toStop = @(Get-MB3DRelevantProcesses -BuildDir $BuildDir -ExcludeProcessIds $ExcludeProcessIds)
        if (-not $toStop -or $toStop.Count -eq 0) {
            break
        }
        foreach ($proc in $toStop) {
            try {
                Stop-Process -Id $proc.ProcessId -Force -ErrorAction Stop
                $killed += "$($proc.Name):$($proc.ProcessId)"
            } catch {
                Write-Warning ("Failed to stop " + $proc.Name + " PID " + $proc.ProcessId + ": " + $_.Exception.Message)
            }
        }
        Start-Sleep -Milliseconds 500
    }
    return $killed
}

function Ensure-SingleMB3DLaunchEnvironment {
    param(
        [string]$BuildDir = $(Get-MB3DGuardBuildDir),
        [ValidateSet('wait','kill','fail')]
        [string]$ConflictPolicy = "kill",
        [int]$WaitMs = 3000,
        [int]$PollMs = 250,
        [System.Collections.Generic.HashSet[int]]$ExcludeProcessIds = $(Get-MB3DGuardExcludedProcessIds)
    )

    $deadline = [Environment]::TickCount64 + $WaitMs
    while ([Environment]::TickCount64 -lt $deadline) {
        $running = @(Get-MB3DRelevantProcesses -BuildDir $BuildDir -ExcludeProcessIds $ExcludeProcessIds)
        if ($running.Count -eq 0) {
            return @()
        }
        Start-Sleep -Milliseconds $PollMs
    }

    $running = @(Get-MB3DRelevantProcesses -BuildDir $BuildDir -ExcludeProcessIds $ExcludeProcessIds)
    if ($running.Count -eq 0) {
        return @()
    }

    switch ($ConflictPolicy) {
        'wait' {
            throw "Existing MB3D-related processes are still running after wait window"
        }
        'fail' {
            $names = $running | ForEach-Object { "$($_.Name):$($_.ProcessId)" }
            throw ("Refusing to launch while MB3D-related processes are alive: " + ($names -join ", "))
        }
        'kill' {
            $killed = Stop-MB3DRelevantProcesses -BuildDir $BuildDir -ExcludeProcessIds $ExcludeProcessIds
            $remaining = @(Get-MB3DRelevantProcesses -BuildDir $BuildDir -ExcludeProcessIds $ExcludeProcessIds)
            if ($remaining.Count -ne 0) {
                $names = $remaining | ForEach-Object { "$($_.Name):$($_.ProcessId)" }
                throw ("Failed to obtain single-process runtime; remaining: " + ($names -join ", "))
            }
            foreach ($entry in $killed) {
                Write-Output ("stopped=" + $entry)
            }
            return $killed
        }
    }
}
