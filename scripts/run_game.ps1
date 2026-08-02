$RootDir = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $RootDir "build"

& (Join-Path $PSScriptRoot "build.ps1")
if (-not $?) { exit 1 }

$GameExe = Join-Path $BuildDir "game5281.exe"
if (Test-Path $GameExe) {
  Push-Location $RootDir
  & $GameExe
  Pop-Location
} else {
  Write-Error "game executable not found at $GameExe"
  exit 1
}
