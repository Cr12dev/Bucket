$RootDir = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $RootDir "build"

& (Join-Path $PSScriptRoot "build.ps1")
if (-not $?) { exit 1 }

$EditorExe = Join-Path $BuildDir "editor.exe"
if (Test-Path $EditorExe) {
  Push-Location $RootDir
  & $EditorExe
  Pop-Location
} else {
  Write-Error "editor executable not found at $EditorExe"
  exit 1
}
