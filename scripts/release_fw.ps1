# Builds the hub and gauge firmware and publishes it on the site: the hubs
# update themselves from it, and fetch the gauge builds to pass on to the
# gauges over Wi-Fi (docs/firmware-updates.md).
#
#   .\scripts\release_fw.ps1                      # every board, upload
#   .\scripts\release_fw.ps1 -Envs hub_p4,gauge   # some boards
#   .\scripts\release_fw.ps1 -Notes "What changed" # shown on the hub's page
#   .\scripts\release_fw.ps1 -NoUpload            # build and write firmware/ only
#
# Hub versions come from HUB_FW_VERSION in src/hub/hub_config.h, gauge versions
# from src/gauge/version.h. Each build goes to firmware/<board>/firmware.bin
# with a manifest.json beside it, then both are put into the site's KV store
# with wrangler (logged in with `npx wrangler login`).

param(
    [string[]]$Envs = @('hub', 'hub_p4', 'gauge', 'gauge_m5dial'),
    [string]$Notes = '',
    [switch]$NoUpload
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$chips = @{ hub = 'esp32c6'; hub_p4 = 'esp32p4'; gauge = 'gauge-ws175'; gauge_m5dial = 'gauge-m5dial' }

$header = Get-Content (Join-Path $root 'src\hub\hub_config.h') -Raw
if ($header -notmatch '#define HUB_FW_VERSION "([0-9]+\.[0-9]+\.[0-9]+)"') { throw 'HUB_FW_VERSION not found in src/hub/hub_config.h' }
$hubVersion = $Matches[1]
$gaugeHeader = Get-Content (Join-Path $root 'src\gauge\version.h') -Raw
if ($gaugeHeader -notmatch 'FW_VERSION_MAJOR (\d+)') { throw 'FW_VERSION_MAJOR not found in src/gauge/version.h' }
$gaugeVersion = $Matches[1]
if ($gaugeHeader -notmatch 'FW_VERSION_MINOR (\d+)') { throw 'FW_VERSION_MINOR not found' }
$gaugeVersion += '.' + $Matches[1]
if ($gaugeHeader -notmatch 'FW_VERSION_PATCH (\d+)') { throw 'FW_VERSION_PATCH not found' }
$gaugeVersion += '.' + $Matches[1]
$versions = @{ hub = $hubVersion; hub_p4 = $hubVersion; gauge = $gaugeVersion; gauge_m5dial = $gaugeVersion }
$date = Get-Date -Format 'yyyy-MM-dd'

if (-not (Get-Command node -ErrorAction SilentlyContinue)) { $env:Path = 'C:\Program Files\nodejs;' + $env:Path }
$pio = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"

Push-Location $root
try {
    foreach ($e in $Envs) {
        if (-not $chips.ContainsKey($e)) { throw "no chip known for env '$e' (add it to this script)" }
        $chip = $chips[$e]
        $version = $versions[$e]
        Write-Host "== $e ($chip) $version"
        # The toolchain's warnings go to stderr, which strict mode would treat as an error; the exit code decides.
        $eap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
        & $pio run -e $e
        $ErrorActionPreference = $eap
        if ($LASTEXITCODE -ne 0) { throw "build of $e failed" }
        $bin = Join-Path $root ".pio\build\$e\firmware.bin"
        if (-not (Test-Path $bin)) { throw "$bin not found" }

        $dir = Join-Path $root "firmware\$chip"
        New-Item -ItemType Directory -Force $dir | Out-Null
        $out = Join-Path $dir 'firmware.bin'
        Copy-Item $bin $out -Force
        $size = (Get-Item $out).Length
        $sha = (Get-FileHash $out -Algorithm SHA256).Hash.ToLower()
        $manifest = [ordered]@{ board = $chip; version = $version; size = $size; sha256 = $sha; date = $date; notes = $Notes; env = $e }
        $manifestPath = Join-Path $dir 'manifest.json'
        ($manifest | ConvertTo-Json -Compress) | Out-File -Encoding ascii -NoNewline $manifestPath
        Write-Host "   $size bytes, sha256 $sha"

        if (-not $NoUpload) {
            & npx wrangler kv key put "fw:${chip}:bin" --path $out --binding HG --remote
            if ($LASTEXITCODE -ne 0) { throw "upload of $chip firmware failed (run npx wrangler login?)" }
            & npx wrangler kv key put "fw:${chip}:manifest" --path $manifestPath --binding HG --remote
            if ($LASTEXITCODE -ne 0) { throw "upload of $chip manifest failed" }
            Write-Host "   published: https://oagauge.online/fw/$chip/manifest.json"
        }
    }
} finally {
    Pop-Location
}
