param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$ConfigureOnly,
    [switch]$Run,
    [switch]$Test,
    [string]$VcpkgRoot = 'D:\vcpkg',
    [string]$CompilerRoot = 'C:\msys64\ucrt64'
)

$ErrorActionPreference = 'Stop'
if ($ConfigureOnly -and ($Run -or $Test)) { throw '-ConfigureOnly cannot be combined with -Run or -Test.' }
$projectRoot = Split-Path $PSScriptRoot -Parent
. (Join-Path $PSScriptRoot 'toolchain.ps1')
$toolchain = Get-ZombieToolchain -VcpkgRoot $VcpkgRoot -CompilerRoot $CompilerRoot
$preset = "windows-$($Configuration.ToLowerInvariant())"
$buildDir = Join-Path $projectRoot "build\$Configuration"
$installedDir = Join-Path $projectRoot 'vcpkg_installed'
$dependencyBin = Join-Path $installedDir 'x64-mingw-dynamic\bin'
if ($Configuration -eq 'Debug') {
    $dependencyBin = Join-Path $installedDir 'x64-mingw-dynamic\debug\bin'
}
$previousPath = $env:PATH

try {
    $env:PATH = "$(Join-Path $CompilerRoot 'bin');$dependencyBin;$previousPath"
    Push-Location $projectRoot
    try {
        & $toolchain.CMake --preset $preset `
            "-DCMAKE_CXX_COMPILER=$($toolchain.CXX)" `
            "-DCMAKE_MAKE_PROGRAM=$($toolchain.Ninja)" `
            "-DCMAKE_TOOLCHAIN_FILE=$($toolchain.CMakeToolchain)"
        if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed. Install dependencies with scripts/setup.ps1 first.' }

        if (!$ConfigureOnly) {
            & $toolchain.CMake --build --preset $preset
            if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
        }

        if ($Test) {
            $ctest = Join-Path (Split-Path $toolchain.CMake) 'ctest.exe'
            & $ctest --test-dir $buildDir --output-on-failure --no-tests=error
            if ($LASTEXITCODE -ne 0) { throw 'Runtime tests failed.' }
        }

        if ($Run) {
            $executable = Join-Path $buildDir 'bin\ZombieGame.exe'
            if (!(Test-Path -LiteralPath $executable)) {
                throw 'No game executable yet. Add src/main.cpp in the next development stage.'
            }
            Push-Location (Split-Path $executable)
            try {
                & $executable
                if ($LASTEXITCODE -ne 0) { throw "Game exited with code $LASTEXITCODE" }
            } finally { Pop-Location }
        }
    } finally { Pop-Location }
} finally { $env:PATH = $previousPath }
