param(
    [string]$VcpkgRoot = 'D:\vcpkg',
    [string]$CompilerRoot = 'C:\msys64\ucrt64'
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$manifestDir = Join-Path $projectRoot 'config\vcpkg'
. (Join-Path $PSScriptRoot 'toolchain.ps1')
$toolchain = Get-ZombieToolchain -VcpkgRoot $VcpkgRoot -CompilerRoot $CompilerRoot
$previousPath = $env:PATH
$previousConcurrency = $env:VCPKG_MAX_CONCURRENCY

try {
    $env:PATH = "$(Join-Path $CompilerRoot 'bin');$previousPath"
    $env:VCPKG_MAX_CONCURRENCY = '4'
    Push-Location $manifestDir
    try {
        & $toolchain.Vcpkg install --triplet x64-mingw-dynamic --host-triplet x64-mingw-dynamic `
            "--x-install-root=$projectRoot\vcpkg_installed"
        if ($LASTEXITCODE -ne 0) { throw 'Dependency installation failed.' }
    } finally { Pop-Location }
} finally {
    $env:PATH = $previousPath
    $env:VCPKG_MAX_CONCURRENCY = $previousConcurrency
}
