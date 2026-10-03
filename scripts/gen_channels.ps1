# Regenerates web/data/channels.json from the firmware's channel tables.
# Run it whenever lib/hg_haltech/haltech_channels.def or units.cpp change.
# Uses the WinLibs g++ installed by winget when g++ is not already on PATH.

$ErrorActionPreference = 'Stop'

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    $package = Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Directory -Filter 'BrechtSanders.WinLibs*' |
        Select-Object -First 1
    if (-not $package) {
        throw 'g++ not found. Install it with: winget install BrechtSanders.WinLibs.POSIX.UCRT'
    }
    $env:PATH = (Join-Path $package.FullName 'mingw64\bin') + ';' + $env:PATH
}

$root = Split-Path $PSScriptRoot -Parent
$exe = Join-Path $env:TEMP 'hg_gen_channels.exe'
Push-Location $root
try {
    & g++ -std=c++17 -O1 -static -I lib/hg_haltech -I lib/hg_can -I lib/hg_proto -I lib/hg_config `
        scripts/gen_channels.cpp lib/hg_haltech/haltech_decode.cpp lib/hg_haltech/units.cpp -o $exe
    if ($LASTEXITCODE -ne 0) { throw 'compile failed' }
    New-Item -ItemType Directory -Force web/data | Out-Null
    $json = & $exe
    [System.IO.File]::WriteAllText((Join-Path $root 'web/data/channels.json'), ($json -join "`n") + "`n", (New-Object System.Text.UTF8Encoding $false))
    Write-Host "web/data/channels.json written ($($json.Length) bytes)"
} finally {
    Pop-Location
}
