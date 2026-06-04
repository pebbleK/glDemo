param(
    [string]$Config = "Debug",
    [string]$BuildDir = "build",
    [string]$Target = "glDemo",
    [string]$VcpkgToolchain = "D:/cmd/vcpkg/scripts/buildsystems/vcpkg.cmake"
)

$ErrorActionPreference = "Stop"

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildPath = Join-Path $ProjectDir $BuildDir

if (-not (Test-Path -LiteralPath $BuildPath)) {
    New-Item -ItemType Directory -Path $BuildPath | Out-Null
}

$configureArgs = @(
    "-S", $ProjectDir,
    "-B", $BuildPath
)

if (Test-Path -LiteralPath $VcpkgToolchain) {
    $configureArgs += "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain"
}

cmake @configureArgs
cmake --build $BuildPath --config $Config --target $Target

$ExePath = Join-Path $BuildPath "$Config/$Target.exe"
if (-not (Test-Path -LiteralPath $ExePath)) {
    throw "Executable not found: $ExePath"
}

Push-Location (Split-Path -Parent $ExePath)
try {
    & $ExePath
}
finally {
    Pop-Location
}
