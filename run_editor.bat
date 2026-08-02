@echo off
rem Runs the editor executable from the project root (needed for shaders).
cd /d "%~dp0"
if exist "build\editor.exe" (
    echo Running build\editor.exe
    "build\editor.exe"
) else (
    echo No editor found. Run: cmake --build build
)
