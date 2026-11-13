param(
    [Parameter(Mandatory = $true)][string]$OutputDirectory,
    [string]$VisualStudioPath
)

$ErrorActionPreference = 'Stop'
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$output = [IO.Path]::GetFullPath($OutputDirectory)
if ($output.Equals($repo, [StringComparison]::OrdinalIgnoreCase) -or
    $output.StartsWith($repo.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Build output must be outside the reference repository.'
}
if (-not (Test-Path -LiteralPath $output -PathType Container)) {
    throw 'OutputDirectory must be an existing external directory (the prepared Pascal tree).'
}
$dll = Join-Path $output 'portable_math.dll'
if (Test-Path -LiteralPath $dll) { throw 'Refusing to overwrite an existing portable_math.dll.' }
if (-not $VisualStudioPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Install Visual Studio C++ tools, or supply -VisualStudioPath.' }
    $VisualStudioPath = (& $vswhere -latest -version '[17.0,18.0)' -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1)
    if ($LASTEXITCODE -ne 0 -or -not $VisualStudioPath) { throw 'Visual Studio x86/x64 C++ tools were not found.' }
}
$vcvars = Join-Path $VisualStudioPath 'VC\Auxiliary\Build\vcvarsall.bat'
if (-not (Test-Path -LiteralPath $vcvars)) { throw "Missing compiler environment: $vcvars" }
$source = Join-Path $repo 'external_formulas\vendored\portable_math\portable_math.c'
$build = Join-Path $output ('portable-math-build-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $build | Out-Null
# Every compiler artifact belongs to the fresh external build directory. /MT
# avoids a separate Visual C++ redistributable requirement for the math DLL.
$batch = Join-Path $build 'build.cmd'
$batchText = @"
@echo off
chcp 65001 >nul
setlocal DisableDelayedExpansion
call "$vcvars" x86
if errorlevel 1 exit /b %errorlevel%
cl /nologo /O2 /arch:SSE2 /fp:strict /MT /D MB3D_PORTABLE_MATH_DLL /LD /Fe:portable_math.dll "$source"
exit /b %errorlevel%
"@
$batchText = $batchText.Replace("`r`n", "`n").Replace("`n", "`r`n") + "`r`n"
[IO.File]::WriteAllText($batch, $batchText, [Text.UTF8Encoding]::new($false))
Push-Location $build
try {
    # Match the build runbook: environments containing both PATH and Path can
    # confuse Visual Studio's tools. Python also passes the Unicode batch path
    # through CreateProcessW, preserving spaces and non-ASCII directory names.
    & python -c 'import os,subprocess,sys; sys.exit(subprocess.call(sys.argv[1:],env={k.upper():v for k,v in os.environ.items()}))' $env:ComSpec '/d' '/c' $batch
    if ($LASTEXITCODE -ne 0) { throw "Math DLL build failed ($LASTEXITCODE)." }
    if (-not (Test-Path -LiteralPath 'portable_math.dll')) { throw 'Compiler did not produce portable_math.dll.' }
    Copy-Item -LiteralPath (Join-Path $build 'portable_math.dll') -Destination $dll
} finally { Pop-Location }
Get-FileHash -LiteralPath $dll -Algorithm SHA256
