param(
    [string]$Config = "Debug",
    [string]$BuildDir = "build",
    [string]$Target = "glDemo",
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Platform = "x64",
    [string]$VcpkgToolchain = "D:/cmd/vcpkg/scripts/buildsystems/vcpkg.cmake",
    [string]$VcpkgTriplet = "x64-windows"
)

$ErrorActionPreference = "Stop"

$ProcessPath = [Environment]::GetEnvironmentVariable("Path", "Process")
if ($ProcessPath) {
    [Environment]::SetEnvironmentVariable("PATH", $null, "Process")
    [Environment]::SetEnvironmentVariable("Path", $ProcessPath, "Process")
}

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildPath = Join-Path $ProjectDir $BuildDir

if (-not (Test-Path -LiteralPath $BuildPath)) {
    New-Item -ItemType Directory -Path $BuildPath | Out-Null
}

$configureArgs = @(
    "-S", $ProjectDir,
    "-B", $BuildPath
)

if ($Generator) {
    $configureArgs += @("-G", $Generator)
}

if ($Platform -and $Generator -like "Visual Studio*") {
    $configureArgs += @("-A", $Platform)
}

if (Test-Path -LiteralPath $VcpkgToolchain) {
    $configureArgs += @(
        "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain",
        "-DVCPKG_TARGET_TRIPLET=$VcpkgTriplet"
    )
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
