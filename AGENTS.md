# Bucket Game Engine — AGENTS.md

## Project structure

```
Bucket/
├── engine/          # Core game engine (shared library)
│   ├── core/        # Window, input, timing, math, ECS
│   ├── renderer/    # OpenGL abstraction, shaders, pipelines
│   └── resource/    # Asset loading, management
├── editor/          # Standalone editor app (links engine)
│   ├── viewport/    # 3D scene viewport
│   └── texture/     # Texture editor tools
├── game/            # The actual game (links engine)
├── cmake/           # Custom CMake modules
├── shaders/         # GLSL source files
└── third_party/     # Vendored deps (git submodules or FetchContent)
```

## Build system

- **CMake** — required. Generator: Ninja or Visual Studio.
- **Toolchain** — Clang (not MSVC, not GCC). Enforce via `CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER`.
- OpenGL dependencies: `glad`, `GLFW`, `glm`, `stb_image`, `ImGui`.
- Prefer `FetchContent` for third-party; write a convenience `cmake/` module per dep if the setup is non-trivial.

### Build commands

```bash
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build           # all targets
cmake --build build -t editor # single target
cmake --build build -t game
```

### Build & run scripts

Multi-step tasks (configure + build + launch) go through scripts so agents don't chain raw commands:

| Script | Purpose |
|--------|---------|
| `scripts/build.ps1` / `scripts/build.sh` | Configure + build all |
| `scripts/run_game.ps1` / `scripts/run_game.sh` | Build + launch game executable |
| `scripts/run_editor.ps1` / `scripts/run_editor.sh` | Build + launch editor executable |

The `.ps1` variants run on Windows (PowerShell), `.sh` on Linux/macOS. If a script doesn't exist yet, **create both** when adding the feature it wraps.

## Coding conventions

- C++20 or later, no exceptions unless discussed.
- Headers: `.hpp`, sources: `.cpp`.
- Naming: `snake_case` for functions/variables, `PascalCase` for types, `kCamelCase` for constants.
- Prefer forward declarations over includes in headers.
- OpenGL calls go through a thin abstraction layer (no raw `gl*` outside `engine/renderer/`).

## Testing

- TBD: add test framework when one is chosen (GoogleTest or Catch2).

## Decision confidence rule

If you are not at least 80% confident in a design or tooling decision, **ask the user before proceeding**. This covers architecture choices, API design, library selection, and any deviation from the conventions above.
