function Get-ZombieToolchain {
    param([string]$VcpkgRoot, [string]$CompilerRoot)

    $toolchain = @{
        Vcpkg = Join-Path $VcpkgRoot 'vcpkg.exe'
        CMake = Join-Path $VcpkgRoot 'downloads\tools\cmake-4.4.4-windows\cmake-4.4.4-windows-x86_64\bin\cmake.exe'
        Ninja = Join-Path $VcpkgRoot 'downloads\tools\ninja-1.13.2-windows\ninja.exe'
        CC = Join-Path $CompilerRoot 'bin\gcc.exe'
        CXX = Join-Path $CompilerRoot 'bin\g++.exe'
        CMakeToolchain = Join-Path $VcpkgRoot 'scripts\buildsystems\vcpkg.cmake'
    }
    foreach ($entry in $toolchain.GetEnumerator()) {
        if (!(Test-Path -LiteralPath $entry.Value -PathType Leaf)) {
            throw "Required tool not found ($($entry.Key)): $($entry.Value)"
        }
    }
    return [pscustomobject]$toolchain
}
