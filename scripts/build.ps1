param(
  [string]$Generator = "Ninja"
)

$RootDir = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $RootDir "build"

if (-not (Test-Path $BuildDir)) {
  New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
}

cmake -B $BuildDir -G $Generator `
  -DCMAKE_C_COMPILER=clang `
  -DCMAKE_CXX_COMPILER=clang++ `
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON `
  -Wno-deprecated

if ($?) {
  cmake --build $BuildDir
}
