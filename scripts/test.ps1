# Runs the host unit tests (pio test -e native).
# Uses the WinLibs g++ installed by winget when g++ is not already on PATH.
# Extra arguments are passed through, e.g.  .\scripts\test.ps1 -f test_native_proto

$ErrorActionPreference = 'Stop'

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    $package = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Directory -Filter 'BrechtSanders.WinLibs*' |
        Select-Object -First 1
    if (-not $package) {
        throw 'g++ not found. Install it with: winget install BrechtSanders.WinLibs.POSIX.UCRT'
    }
    $env:PATH = (Join-Path $package.FullName 'mingw64\bin') + ';' + $env:PATH
}

Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e native @args
} finally {
    Pop-Location
}
