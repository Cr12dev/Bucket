# Bucket Game Engine — Plan de Desarrollo

## Stack técnico

| Aspecto | Decisión |
|---------|----------|
| Lenguaje | C++20 (sin excepciones salvo discusión) |
| Compilador | Clang (MSVC y GCC no permitidos) |
| Build | CMake + Ninja |
| OpenGL | 3.3 Core Profile |
| ECS | Custom (Entity-Component-System propio) |
| Editor GUI | Dear ImGui |
| Juego | Sandbox / prototipo |
| Plataformas | Windows, macOS, Linux |

## Principios multiplataforma

| Aspecto | Estrategia |
|---------|-----------|
| Ventana / input | GLFW |
| OpenGL loader | glad (gl 3.3 core, commitear el .h/.c) |
| Matemáticas | glm (header-only) |
| Texturas | stb_image |
| UI editor | Dear ImGui (backend GLFW + OpenGL3) |
| Build | CMake + Clang en todas (MSVC no permitido ni en Windows) |
| Scripts | `.ps1` en Windows, `.sh` en macOS/Linux |
| Paths | `std::filesystem::path` + `generic_string()` — nunca `\` |
| Shaders | GLSL 330 **core** (incompatible `compatibility` en macOS) |

---

## Fase 0 — Base multiplataforma

**Objetivo:** Ventana + renderizar un triángulo en las 3 OS.

| Paso | Archivos clave |
|-------|---------------|
| CMakeLists.txt con FetchContent (glad, GLFW, glm, stb_image, ImGui) | `CMakeLists.txt`, `cmake/FetchGLAD.cmake`, `cmake/FetchGLFW.cmake`, `cmake/FetchGLM.cmake`, `cmake/FetchSTB.cmake`, `cmake/FetchImGui.cmake` |
| Toolchain clang enforce | `CMakeLists.txt` (`CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER`) |
| Scripts build & run | `scripts/build.ps1`, `scripts/build.sh`, `scripts/run_game.ps1`, `scripts/run_game.sh`, `scripts/run_editor.ps1`, `scripts/run_editor.sh` |
| GLFW init + OpenGL 3.3 core context | `engine/core/window.cpp` + `window.hpp` |
| glad load + game loop | `engine/core/application.cpp` + `application.hpp` |
| Shader abstraction (compile + link) | `engine/renderer/shader.hpp` + `shader.cpp` |
| VertexBuffer + VertexArray | `engine/renderer/buffer.hpp` + `buffer.cpp` |
| Triángulo renderizado | `shaders/triangle.vert`, `shaders/triangle.frag` |
| Delta time | `engine/core/timer.cpp` + `timer.hpp` |

**Verificación:** `cmake --build build -t game` → ventana con triángulo.

---

## Fase 1 — ECS + Matemáticas + Cámara

**Objetivo:** Motor de entidades funcional con transformaciones y cámara 3D.

| Paso | Archivos clave |
|-------|---------------|
| Entity (ID numérico), Component (tag base) | `engine/core/ecs/entity.hpp`, `engine/core/ecs/component.hpp` |
| Scene con sparse set pools | `engine/core/ecs/scene.hpp` + `scene.cpp` |
| System base + SystemManager | `engine/core/ecs/system.hpp` + `system_manager.cpp` |
| Componentes core: Transform, Tag | `engine/core/ecs/components/transform.hpp`, `engine/core/ecs/components/tag.hpp` |
| glm wrappers (vec3, mat4, quat) | `engine/core/math/vec3.hpp`, `engine/core/math/mat4.hpp`, `engine/core/math/transform.hpp` |
| Cámara 3D (lookAt, perspective, free-look) | `engine/core/camera.hpp` + `camera.cpp` |
| Input system poll (teclado + mouse) | `engine/core/input.hpp` + `input.cpp` |

**Verificación:** Escena con entidades transformadas, cámara orbita.

---

## Fase 2 — Renderer

**Objetivo:** Pipeline de renderizado con meshes, materiales y texturas.

| Paso | Archivos clave |
|-------|---------------|
| Mesh (VAO + EBO + material slot) | `engine/renderer/mesh.hpp` + `mesh.cpp` |
| Texture loader (stb_image, flip Y) | `engine/renderer/texture.hpp` + `texture.cpp` |
| Material (shader + texture + uniforms) | `engine/renderer/material.hpp` + `material.cpp` |
| RenderCommand (clear, draw, viewport) | `engine/renderer/render_command.hpp` |
| Renderer3D (scene → draw calls) | `engine/renderer/renderer3d.hpp` + `renderer3d.cpp` |

**Verificación:** Mesh con textura renderizado.

---

## Fase 3 — Recursos

**Objetivo:** Carga y caché de assets.

| Paso | Archivos clave |
|-------|---------------|
| ResourceHandle + ResourceCache | `engine/resource/resource_cache.hpp` + `resource_cache.cpp` |
| Model loader (.obj) | `engine/resource/model_loader.hpp` + `model_loader.cpp` |
| Texture manager | `engine/resource/texture_manager.hpp` + `texture_manager.cpp` |

**Verificación:** Cargar modelo .obj + textura desde disco.

---

## Fase 4 — Editor

**Objetivo:** Aplicación editor con viewport 3D, hierarchy y texture editor.

| Paso | Archivos clave |
|-------|---------------|
| ImGui docking setup + panel system | `editor/viewport/imgui_layer.hpp` + `imgui_layer.cpp` |
| Viewport 3D (FBO → ImGui::Image) | `editor/viewport/viewport_panel.hpp` + `viewport_panel.cpp` |
| Entity hierarchy panel | `editor/scene_hierarchy_panel.hpp` + `scene_hierarchy_panel.cpp` |
| Inspector panel (transform, tag) | `editor/inspector_panel.hpp` + `inspector_panel.cpp` |
| Texture editor base | `editor/texture/texture_editor.hpp` + `texture_editor.cpp` |
| Editor app (Application subclass) | `editor/main.cpp` |

**Verificación:** `cmake --build build -t editor` → viewport interactivo + panels.

---

## Fase 5 — Juego

**Objetivo:** Aplicación jugable con escena de prueba.

| Paso | Archivos clave |
|-------|---------------|
| Player controller system (wasd + mouse) | `game/player_system.hpp` + `player_system.cpp` |
| Escena de prueba (modelos, luces, suelo) | `game/scenes/test_scene.cpp` |
| Game app (Application subclass) | `game/main.cpp` |

**Verificación:** `cmake --build build -t game` → free-look en escena con assets.

---

## Orden de verificación

```
Fase 0 → cmake --build build -t game    → triángulo en ventana (Win + Linux al menos)
Fase 1 → test manual de ECS + cámara     → escena con entidades
Fase 2 → mesh con textura                → objeto 3D visible
Fase 3 → load .obj desde disco           → modelo complejo renderizado
Fase 4 → cmake --build build -t editor   → viewport + panels
Fase 5 → free-look + escena de prueba    → experiencia jugable
```

Cada fase debe compilar sin warnings en Clang de las 3 OS. No pasar a la siguiente sin verificar la anterior.

## Reglas durante implementación

1. Cada fase produce un tag commit funcional (`fase-0`, `fase-1`, ...)
2. No avanzar sin verificar la fase actual
3. Si estás por debajo del 80% de confianza en diseño o herramienta, **preguntar al usuario**
4. Nunca usar `#ifdef _WIN32` sin wrapper — diferencias van en `engine/platform/`
5. GLSL siempre `#version 330 core` (macOS no acepta `compatibility`)
6. Assets con rutas relativas al binario (`argv[0]` → `std::filesystem::path`)
7. Scripts `.ps1` y `.sh` para tareas multi-paso — crear ambos si uno no existe
