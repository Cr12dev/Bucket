@echo off
rem Runs the most recently built game executable (name changes every build).
cd /d "%~dp0"
for /f "delims=" %%e in ('dir /b /o-d "build\game*.exe" 2^>nul') do (
    echo Running build\%%e
    "build\%%e"
    goto :done
)
echo No game executable found. Run: cmake --build build
:done
