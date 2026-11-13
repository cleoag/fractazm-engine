param(
    [ValidateSet('diag','diag-fast','release-fast')]
    [string]$Mode = 'diag-fast'
)

$ErrorActionPreference = 'Stop'
$workspaceScript = Join-Path $PSScriptRoot 'windows_workspace.ps1'
. $workspaceScript
$ws = Get-WindowsWorkspace
$guardScript = Join-Path $PSScriptRoot 'windows_runtime_guard.ps1'
. $guardScript

$projectDir = $ws.PascalWorkRoot
$projectFile = Join-Path $projectDir 'Mandelbulb3D.lpi'
$formulaSrcDir = Join-Path $projectDir 'M3Formulas'
$mapSrcDir = Join-Path $projectDir 'M3Maps'
$buildOutDir = Join-Path $ws.ComparisonRoot 'windows_build'
$formulaOutDir = Join-Path $buildOutDir 'M3Formulas'
$mapOutDir = Join-Path $buildOutDir 'M3Maps'
$projectExe = Join-Path $projectDir 'Mandelbulb3D.exe'
$sourceDll = Join-Path $projectDir 'portable_math.dll'
$stageExe = Join-Path $buildOutDir 'Mandelbulb3D_build.exe'
$modeExe = switch ($Mode) {
    'diag' { Join-Path $buildOutDir 'Mandelbulb3D_diag.exe' }
    'diag-fast' { Join-Path $buildOutDir 'Mandelbulb3D_diag_fast.exe' }
    'release-fast' { Join-Path $buildOutDir 'Mandelbulb3D_release_fast.exe' }
}
$historyDir = Join-Path $buildOutDir 'History'

function Resolve-LazbuildPath {
    $cmd = Get-Command lazbuild -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }

    $candidates = @(
        'C:\lazarus\lazbuild.exe',
        'C:\Program Files\Lazarus\lazbuild.exe',
        'C:\Program Files (x86)\Lazarus\lazbuild.exe'
    )
    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    throw 'lazbuild not found in PATH or standard install locations'
}

$args = @('--build-all', '--cpu=i386', '--os=win32', $projectFile)
$lazbuildExe = Resolve-LazbuildPath

if (-not (Test-Path -LiteralPath $projectDir)) {
    throw "Local Pascal working copy root does not exist: $projectDir"
}
if (-not (Test-Path -LiteralPath $projectFile)) {
    throw "Pascal project file missing: $projectFile"
}
if (-not (Test-Path -LiteralPath $sourceDll)) {
    throw "Required Pascal math runtime missing: $sourceDll"
}
switch ($Mode) {
    'diag' {
        # Uses project defaults from Mandelbulb3D.lpi, including -dFPC_DIAG.
    }
    'diag-fast' {
        # Keep FPC_DIAG for siLight5 / parity support, but remove debug info and
        # raise optimization to make full-scene reference renders less painful.
        $args += '--opt=-O2'
        $args += '--opt=-g-'
        $args += '--opt=-gl-'
    }
    'release-fast' {
        # Build a production-style Windows reference binary without trace/diag code.
        $args += '--opt=-uFPC_DIAG'
        $args += '--opt=-O2'
        $args += '--opt=-g-'
        $args += '--opt=-gl-'
    }
}

Push-Location $projectDir
try {
    New-Item -ItemType Directory -Path $buildOutDir -Force | Out-Null
    Ensure-SingleMB3DLaunchEnvironment -BuildDir $buildOutDir -ConflictPolicy fail | Out-Null

    & $lazbuildExe @args
    if ($LASTEXITCODE -ne 0) {
        throw "lazbuild failed with exit code $LASTEXITCODE"
    }

    if (Test-Path -LiteralPath $projectExe) {
        Copy-Item -LiteralPath $projectExe -Destination $stageExe -Force
    } else {
        throw "Expected build output missing after build: $projectExe"
    }

    New-Item -ItemType Directory -Path $historyDir -Force | Out-Null

    if (Test-Path -LiteralPath $modeExe) {
        $timestamp = Get-Date -Format 'yyyyMMdd_HHmmss'
        $archiveExe = Join-Path $historyDir ("{0}_{1}.exe" -f [System.IO.Path]::GetFileNameWithoutExtension($modeExe), $timestamp)
        Move-Item -LiteralPath $modeExe -Destination $archiveExe -Force
    }

    Copy-Item -LiteralPath $stageExe -Destination $modeExe -Force

    $stagedDll = Join-Path $buildOutDir 'portable_math.dll'
    Copy-Item -LiteralPath $sourceDll -Destination $stagedDll -Force

    # External Pascal formulas are resolved relative to AppFolder via
    # IniDirs[3] := AppFolder + 'M3Formulas'. The headless reference exe lives in
    # comparison/windows_build, so keep a synced formula tree next to the exe.
    if (Test-Path -LiteralPath $formulaSrcDir) {
        New-Item -ItemType Directory -Path $buildOutDir -Force | Out-Null
        & robocopy $formulaSrcDir $formulaOutDir /MIR /NFL /NDL /NJH /NJS /NP *> $null
        if (($LASTEXITCODE -lt 0) -or ($LASTEXITCODE -gt 7)) {
            throw "robocopy M3Formulas failed with exit code $LASTEXITCODE"
        }
    }

    # HeightMap-based external formulas resolve through IniDirs[9] = AppFolder + 'M3Maps'
    # in headless/reference runs. Keep the runtime map tree next to the mode-specific exe,
    # otherwise Pascal trace/reference builds silently fall back to missing-map behavior.
    if (Test-Path -LiteralPath $mapSrcDir) {
        New-Item -ItemType Directory -Path $buildOutDir -Force | Out-Null
        & robocopy $mapSrcDir $mapOutDir /MIR /NFL /NDL /NJH /NJS /NP *> $null
        if (($LASTEXITCODE -lt 0) -or ($LASTEXITCODE -gt 7)) {
            throw "robocopy M3Maps failed with exit code $LASTEXITCODE"
        }
    }
}
finally {
    Pop-Location
}

Write-Output ("build_mode=" + $Mode)
Write-Output ("lazbuild=" + $lazbuildExe)
Write-Output ("project_exe=" + $projectExe)
Write-Output ("stage_exe=" + $stageExe)
Write-Output ("runtime_exe=" + $modeExe)
