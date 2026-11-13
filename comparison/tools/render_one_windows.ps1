param(
    [string]$ExePath = "",
    [Alias('Scene')]
    [string]$ScenePath,
    [string]$ImageOut,
    [string]$OutputPath = "",
    [string]$SiLightOut = "",
    [string]$ZBufferOut = "",
    [string]$NormalMapOut = "",
    [string]$WorkingDir = "",
    [int]$Threads = 4,
    [switch]$Deterministic,
    [switch]$CapturedSiLight,
    [switch]$NoShadows,
    [switch]$NoAO,
    [switch]$NoReflections,
    [switch]$NoPostprocess,
    [int]$TimeoutMs = 1800000
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing
$workspaceScript = Join-Path $PSScriptRoot 'windows_workspace.ps1'
. $workspaceScript
$ws = Get-WindowsWorkspace
$guardScript = Join-Path $PSScriptRoot 'windows_runtime_guard.ps1'
. $guardScript

function Resolve-ScenePath {
    param(
        [string]$ScenePath
    )

    if ([string]::IsNullOrWhiteSpace($ScenePath)) {
        throw "ScenePath is required"
    }
    if (Test-Path -LiteralPath $ScenePath) {
        return (Resolve-Path -LiteralPath $ScenePath).ProviderPath
    }

    $repoRoot = $ws.RepoRoot
    if (-not ($ScenePath.StartsWith("\\") -or ($ScenePath.Length -ge 2 -and $ScenePath[1] -eq ':'))) {
        $repoRelative = Join-Path $repoRoot $ScenePath.Replace('/', '\')
        if (Test-Path -LiteralPath $repoRelative) {
            return (Resolve-Path -LiteralPath $repoRelative).ProviderPath
        }
    }

    $sceneName = [System.IO.Path]::GetFileNameWithoutExtension($ScenePath)
    $sceneDir = Join-Path $ws.ComparisonRoot 'scenes'
    $candidate = Join-Path $sceneDir ($sceneName + ".m3p")
    if (Test-Path -LiteralPath $candidate) {
        return (Resolve-Path -LiteralPath $candidate).ProviderPath
    }

    throw "Scene not found: $ScenePath"
}

function Resolve-SharedPath {
    param(
        [string]$Path
    )

    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $Path
    }
    if ($Path.StartsWith("\\") -or ($Path.Length -ge 2 -and $Path[1] -eq ':')) {
        return $Path
    }

    return (Join-Path $ws.RepoRoot $Path.Replace('/', '\'))
}

function New-TempSiblingPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FinalPath
    )

    $parent = Split-Path -Parent $FinalPath
    $leaf = Split-Path -Leaf $FinalPath
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($leaf)
    $ext = [System.IO.Path]::GetExtension($leaf)
    $stamp = [Guid]::NewGuid().ToString('N')
    return (Join-Path $parent ($baseName + '.tmp.' + $stamp + $ext))
}

function Publish-TempArtifact {
    param(
        [Parameter(Mandatory = $true)]
        [string]$TempPath,
        [Parameter(Mandatory = $true)]
        [string]$FinalPath
    )

    if (-not (Test-Path -LiteralPath $TempPath)) {
        throw "Temporary artifact missing: $TempPath"
    }

    if (Test-Path -LiteralPath $FinalPath) {
        Remove-Item -LiteralPath $FinalPath -Force -ErrorAction Stop
    }

    Move-Item -LiteralPath $TempPath -Destination $FinalPath -Force
}

if ([string]::IsNullOrWhiteSpace($ExePath)) {
    $ExePath = Join-Path $ws.ComparisonRoot 'windows_build\Mandelbulb3D_release_fast.exe'
}

function Repair-FullyTransparentPng {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    $bitmap = [System.Drawing.Bitmap]::new($Path)
    try {
        $hasAlpha = $false
        $hasNonZeroAlpha = $false

        for ($y = 0; $y -lt $bitmap.Height -and -not $hasNonZeroAlpha; $y++) {
            for ($x = 0; $x -lt $bitmap.Width; $x++) {
                $pixel = $bitmap.GetPixel($x, $y)
                if ($pixel.A -lt 255) {
                    $hasAlpha = $true
                }
                if ($pixel.A -ne 0) {
                    $hasNonZeroAlpha = $true
                    break
                }
            }
        }

        if ((-not $hasAlpha) -or $hasNonZeroAlpha) {
            return $false
        }

        $fixed = [System.Drawing.Bitmap]::new($bitmap.Width, $bitmap.Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            for ($y = 0; $y -lt $bitmap.Height; $y++) {
                for ($x = 0; $x -lt $bitmap.Width; $x++) {
                    $pixel = $bitmap.GetPixel($x, $y)
                    $fixed.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $pixel.R, $pixel.G, $pixel.B))
                }
            }

            $tmpPath = $Path + ".tmp.png"
            $fixed.Save($tmpPath, [System.Drawing.Imaging.ImageFormat]::Png)
            Remove-Item -LiteralPath $Path -Force
            Move-Item -LiteralPath $tmpPath -Destination $Path
            return $true
        } finally {
            $fixed.Dispose()
        }
    } finally {
        $bitmap.Dispose()
    }
}

function Quote-ProcessArgument {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Value
    )

    if ($Value.Length -eq 0) {
        return '""'
    }

    if ($Value -notmatch '[\s"]') {
        return $Value
    }

    $escaped = $Value -replace '(\\*)"', '$1$1\"'
    $escaped = $escaped -replace '(\\+)$', '$1$1'
    return '"' + $escaped + '"'
}

if ([string]::IsNullOrWhiteSpace($ScenePath)) {
    throw "ScenePath is required"
}
$ScenePath = Resolve-ScenePath -ScenePath $ScenePath
if ([string]::IsNullOrWhiteSpace($ImageOut) -and (-not [string]::IsNullOrWhiteSpace($OutputPath))) {
    $ImageOut = $OutputPath
}
if ([string]::IsNullOrWhiteSpace($ImageOut)) {
    throw "ImageOut is required (or pass OutputPath)"
}
$ImageOut = Resolve-SharedPath -Path $ImageOut
if (-not [string]::IsNullOrWhiteSpace($SiLightOut)) {
    $SiLightOut = Resolve-SharedPath -Path $SiLightOut
}
if (-not [string]::IsNullOrWhiteSpace($ZBufferOut)) {
    $ZBufferOut = Resolve-SharedPath -Path $ZBufferOut
}
if (-not [string]::IsNullOrWhiteSpace($NormalMapOut)) {
    $NormalMapOut = Resolve-SharedPath -Path $NormalMapOut
}
if (-not (Test-Path -LiteralPath $ExePath)) {
    throw "Exe not found: $ExePath"
}

Ensure-SingleMB3DLaunchEnvironment | Out-Null

$outDir = Split-Path -Parent $ImageOut
if (-not [string]::IsNullOrWhiteSpace($outDir) -and -not (Test-Path -LiteralPath $outDir)) {
    New-Item -ItemType Directory -Path $outDir -Force | Out-Null
}

Remove-Item $ImageOut -Force -ErrorAction SilentlyContinue
if (-not [string]::IsNullOrWhiteSpace($SiLightOut)) {
    $silDir = Split-Path -Parent $SiLightOut
    if (-not [string]::IsNullOrWhiteSpace($silDir) -and -not (Test-Path -LiteralPath $silDir)) {
        New-Item -ItemType Directory -Path $silDir -Force | Out-Null
    }
    Remove-Item $SiLightOut -Force -ErrorAction SilentlyContinue
}
if (-not [string]::IsNullOrWhiteSpace($ZBufferOut)) {
    $zDir = Split-Path -Parent $ZBufferOut
    if (-not [string]::IsNullOrWhiteSpace($zDir) -and -not (Test-Path -LiteralPath $zDir)) {
        New-Item -ItemType Directory -Path $zDir -Force | Out-Null
    }
    Remove-Item $ZBufferOut -Force -ErrorAction SilentlyContinue
}
if (-not [string]::IsNullOrWhiteSpace($NormalMapOut)) {
    $normalDir = Split-Path -Parent $NormalMapOut
    if (-not [string]::IsNullOrWhiteSpace($normalDir) -and -not (Test-Path -LiteralPath $normalDir)) {
        New-Item -ItemType Directory -Path $normalDir -Force | Out-Null
    }
    Remove-Item $NormalMapOut -Force -ErrorAction SilentlyContinue
}

$workingDir = $WorkingDir
if ([string]::IsNullOrWhiteSpace($workingDir)) {
    $workingDir = Split-Path -Parent $ExePath
}
$tempImageOut = New-TempSiblingPath -FinalPath $ImageOut
$tempSiLightOut = if (-not [string]::IsNullOrWhiteSpace($SiLightOut)) { New-TempSiblingPath -FinalPath $SiLightOut } else { '' }
$tempZBufferOut = if (-not [string]::IsNullOrWhiteSpace($ZBufferOut)) { New-TempSiblingPath -FinalPath $ZBufferOut } else { '' }
$tempNormalMapOut = if (-not [string]::IsNullOrWhiteSpace($NormalMapOut)) { New-TempSiblingPath -FinalPath $NormalMapOut } else { '' }

$argList = @('--render', $ScenePath, '--output', $tempImageOut)
if ($Threads -gt 0) {
    $argList += @('--threads', $Threads)
}
if (-not [string]::IsNullOrWhiteSpace($SiLightOut)) {
    $argList += @('--silight5-dump', $tempSiLightOut)
    if (-not $CapturedSiLight) {
        $argList += @('--silight5-dump-live')
    }
}
if (-not [string]::IsNullOrWhiteSpace($ZBufferOut)) {
    $argList += @('--zbuffer', $tempZBufferOut)
}
if (-not [string]::IsNullOrWhiteSpace($NormalMapOut)) {
    $argList += @('--normalmap', $tempNormalMapOut)
}
if ($Deterministic) {
    $argList += @('--deterministic')
}
if ($NoShadows) {
    $argList += @('--no-shadows')
}
if ($NoAO) {
    $argList += @('--no-ao')
}
if ($NoReflections) {
    $argList += @('--no-reflections')
}
if ($NoPostprocess) {
    $argList += @('--no-postprocess')
}

$argString = ($argList | ForEach-Object { Quote-ProcessArgument -Value ([string]$_) }) -join ' '
$proc = Start-Process -FilePath $ExePath -ArgumentList $argString -WorkingDirectory $workingDir -PassThru
$ok = $proc.WaitForExit($TimeoutMs)
if (-not $ok) {
    try { $proc.Kill() } catch {}
    throw "Timeout while rendering $ScenePath"
}

if (($proc.ExitCode -ne 0) -or (-not (Test-Path -LiteralPath $tempImageOut))) {
    throw "Render failed for $ScenePath with exit code $($proc.ExitCode)"
}
if ((-not [string]::IsNullOrWhiteSpace($SiLightOut)) -and (-not (Test-Path -LiteralPath $tempSiLightOut))) {
    throw "siLight5 dump missing for $ScenePath"
}
if ((-not [string]::IsNullOrWhiteSpace($ZBufferOut)) -and (-not (Test-Path -LiteralPath $tempZBufferOut))) {
    throw "zbuffer missing for $ScenePath"
}
if ((-not [string]::IsNullOrWhiteSpace($NormalMapOut)) -and (-not (Test-Path -LiteralPath $tempNormalMapOut))) {
    throw "normal map missing for $ScenePath"
}

$alphaFixed = $false
if ([System.IO.Path]::GetExtension($tempImageOut).ToLowerInvariant() -eq '.png') {
    $alphaFixed = Repair-FullyTransparentPng -Path $tempImageOut
}

Publish-TempArtifact -TempPath $tempImageOut -FinalPath $ImageOut
if (-not [string]::IsNullOrWhiteSpace($SiLightOut)) {
    Publish-TempArtifact -TempPath $tempSiLightOut -FinalPath $SiLightOut
}
if (-not [string]::IsNullOrWhiteSpace($ZBufferOut)) {
    Publish-TempArtifact -TempPath $tempZBufferOut -FinalPath $ZBufferOut
}
if (-not [string]::IsNullOrWhiteSpace($NormalMapOut)) {
    Publish-TempArtifact -TempPath $tempNormalMapOut -FinalPath $NormalMapOut
}

Write-Output ("exit=" + $proc.ExitCode)
Write-Output ("image=" + $ImageOut)
if ($alphaFixed) {
    Write-Output "alpha_repaired=1"
}
if (-not [string]::IsNullOrWhiteSpace($SiLightOut)) {
    Write-Output ("silight=" + $SiLightOut)
}
if (-not [string]::IsNullOrWhiteSpace($ZBufferOut)) {
    Write-Output ("zbuffer=" + $ZBufferOut)
}
if (-not [string]::IsNullOrWhiteSpace($NormalMapOut)) {
    Write-Output ("normalmap=" + $NormalMapOut)
}
