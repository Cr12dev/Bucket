# Plan del juego — Bucket

FPS arena estilo CS 1.6 construido sobre un motor propio (OpenGL 3.3, C++20, ECS).

---

## 1. Estado actual (lo que ya tenemos)

### Motor / renderer
- Renderer Blinn-Phong con 8 luces: direccional (sol), point y spot.
- Skybox procedural de día (sol con disco + glow), fog exponencial.
- Sombras suaves PCF 5×5 (shadow map 2048², cámara ortográfica que sigue al jugador).
- Reflexión planar del suelo (pase de cámara espejo a media res, clip por plano).
- Multi-textura por entidad: albedo, normal, roughness, emisión (+ color de emisión).
- Texturas **por cara** en cajas (6 slots, orden: front/back/right/left/top/bottom).
- Cache de texturas (carga única y compartida) + triplanar UV automático.
- Prefabs del motor: box, floor, wall, crate, pillar, sandbag, sun, point/spot light.
- Raycast contra AABBs (slab method) para disparos.
- Editor con ImGui: gizmos, selección, panel de componentes (Transform, Tag, Paint, Light),
  save/load de niveles `.lev` (serialización binaria con strings versionables).
- Material slots genéricos (`material.hpp`) — infraestructura aún no usada por el juego.

### Gameplay
- Controlador FPS: WASD, sprint (Shift), salto (Espacio), gravedad, colisiones AABB deslizantes.
- Ratón (captura F1, libera ESC), vista en primera persona (eyepoint 1.7).
- Disparo por raycast (clic izquierdo, cooldown 0.25s), flash del objeto impactado, crosshair.
- Mapa demo estilo CS 1.6: plataforma central, escaleras, pilares, pasillos, cajas, sacos.

### Contenido demo
- 8 texturas generadas (madera, metal, ladrillo, lámpara emisiva) aplicadas en el mapa.

### Build / herramientas
- CMake + Ninja; exe de juego con sufijo aleatorio (game####.exe), limpieza automática de exes viejos.
- `run_game.bat` / `run_editor.bat`.

---

## 2. Lo que falta para un "buen juego"

### Fase 1 — P0: el juego jugable de verdad
1. **Armas reales** — no solo raycast suelto:
   - Armas con datos (cadencia, daño, spread, alcance, munición en cargador y reserva, recarga).
   - Modelo en pantalla (arma en primera persona) y animación de disparo/recarga (bob).
   - Muzzle flash + sonido de disparo.
   - Switch de armas (1/2/3...).
2. **Vida y daño**:
   - Salud del jugador, armadura opcional, HUD de vida y munición.
   - Daño recibido de enemigos con feedback (sangrado de pantalla, sonido).
   - Muerte del jugador → pantalla de muerte → respawn (o reinicio de ronda).
3. **Enemigos con IA básica**:
   - Estado: patrol → alert → chase → attack.
   - Line of sight con raycast, distancias de reacción, persecución con pathfinding simple (waypoints).
   - Disparo con precisión/probabilidad, daño variable.
   - Muerte: drops de munición/vida, contador de bajas.
4. **Rondas / objetivos**:
   - Onda de enemigos creciente (arena shooter) o modo objetivo tipo bomba (CS).
   - HUD: ronda, enemigos restantes, tiempo.

### Fase 2 — P1: contenido y sensación
5. **Modelos 3D reales** (sustituir cajas por meshes cargados):
   - Cargador de OBJ/GLTF, materiales desde archivo, normales por vértice.
   - Enemigos humanoides (bot simplificado o modelos low-poly).
   - Texturas de alta calidad por mapa (specular + AO baked).
6. **Sonido (OpenAL)**:
   - Sistema de audio 3D: pasos, disparos, impactos, recargas, ambiente del mapa.
   - Volúmenes por distancia, reverb básico opcional.
7. **Partículas**:
   - Impactos (polvo en paredes, sangre en enemigos), shell casings, humo, chispas.
   - Sprite particles con shader de billboard + depth test.
8. **Mapas y jugabilidad**:
   - 2-3 mapas completos (editados con el editor + guardados en `.lev`).
   - Spawn points por equipo, compra de armas (estilo CS) o armas por recogida en el suelo.
   - Interactivos: puertas, zonas de bomba, cajas destructibles.

### Fase 3 — P2: pulido técnico
9. **Post-procesado**: bloom (emisión real), tonemapping + gamma, vignette.
10. **Antialiasing** (MSAA o FXAA) y control de calidad gráfica.
11. **Rendimiento**:
    - Frustum culling, occlusion culling por portal/región.
    - Instancing para cajas repetidas, batching de meshes estáticos.
    - Shadows: CSM (cascaded) para evitar sombras duras a distancia.
12. **Mallas y animación**:
    - Componente `mesh` en el ECS (en vez de asumir cubo) con material por entidad.
    - Skeleton + skinning básico para enemigos.

### Fase 4 — P3: experiencia completa
13. **Menús**: principal (jugar/opciones/salir), pausa (ESC), pantalla de muerte.
14. **Opciones**: sensibilidad del ratón, volumen, resolución, calidad, tecla de captura.
15. **Guardado**: progreso de rondas, ajustes, mejores puntuaciones (leaderboard local).
16. **Feedback**: hitmarker, damage numbers, headshot bonus, estadísticas finales de partida.
17. **Tutorial**/modo práctica: enemigos estáticos como dummies.

---

## 3. Lo que necesita el motor

### Núcleo de rendering
1. **Render passes con pipeline explícito** — el render está acoplado al bucle del juego (espejo/sombras/main inline en `render()`); extraer un sistema de pases ordenados (pass list) que el juego configure.
2. **Material system real** — `material.hpp` existe pero no se usa: unir shader + slots de textura + parámetros (color, roughness, emisión) + shader batching por material.
3. **Batch renderer** — reducir draw calls: agrupar por mesh+material, instancing para cajas/partículas, static geometry batching por región.
4. **Componente `mesh` en el ECS** — el render asume cubo; entidad = transform + mesh (referencia a un asset) + material; puerta para modelos reales.
5. **Post-proceso**: framebuffer HDR (floating point), bloom (umbral + downsample blur), tonemapping + corrección gamma, vignette.
6. **Antialiasing**: MSAA en el framebuffer principal o FXAA como post-pass.
7. **Sombras completas**: CSM (cascadas) para el sol, sombras de point/spot lights (cubemap), ajuste de bias por cascada.
8. **Culling**: frustum culling por entidad (AABB), y a futuro occlusion culling por celdas/portales en interiores.
9. **Text rendering** — atlas de fuentes (SDF para nitidez a cualquier escala); HUD actual es solo una crosshair con shader ad-hoc.
10. **Sprite/particle renderer** — billboards con depth test y blending, sistema de partículas genérico (emitters configurables por asset).
11. **UI en runtime** — un sistema de widgets sencillo para el juego (menús, barras, daño), distinto del ImGui del editor.
12. **HDR/skybox dinámico** — hora del día interpolable (color del cielo, sol, intensidad de luces) para variedad visual por mapa.

### Recursos / assets
13. **Asset manager central** — el `texture_cache` es un caso particular; generalizar: carga única + referencias por ruta + contadores, con hot-reload en dev.
14. **Cargador de modelos** — OBJ primero (tinyobjloader) y glTF después (assimp o minifetch) con normales/tangentes/UVs y materiales por submalla.
15. **Formatos de asset propios** — `.bmat` (material), `.bmesh` (malla cocinada), font atlas `.fnt`; versionados y con checksum.
16. **Shader preprocessor** — `#include` y macros por plataforma, y **hot-reload de shaders** en dev (detectar cambio de archivo y recompilar en caliente).
17. **Carga asíncrona** — cola de carga en segundo hilo para texturas/modelos grandes, con assets placeholder hasta que lleguen.
18. **Empaquetado** — build de assets (cooking) + paquete `.pak` opcional para distribución.

### Gameplay engine
19. **Scene graph** — hijos/padres con transform relativos (armas en la mano del jugador, enemigos con partes), world-space caching.
20. **Eventos** — bus de eventos (damage, death, shot, pickups) para desacoplar sistemas (HUD escucha sin acoplarse).
21. **Input actions** — capa de "acciones" renombrables (move, jump, shoot) sobre GLFW: rebinding, gamepad, ejes suavizados.
22. **Audio (OpenAL-soft)** — listener = cámara, fuentes 3D con distancia/rolloff, categorías de volumen (SFX/música/ambiente), streams para música.
23. **Física** — mantener AABB propio o integrar Jolt/Bullet para rigid bodies (puertas, destructibles, proyectiles con gravedad).
24. **Pathfinding** — grafo de waypoints + A* para la IA de enemigos; debug draw de rutas.
25. **Sistema de partículas de gameplay** — pooling, emisores en el ECS, fin de vida auto.
26. **Tweens/coroutines** — animar valores (puertas, flashes, cámara) sin lógica manual por frame.
27. **Timer/reloj del juego** — tiempo de ronda, alarma de bomba, pausa que no afecta a menus.
28. **Settings/config** — `engine.ini` (resolución, calidad, volumen, sensibilidad) cargado al arranque y guardado al cerrar.

### Infraestructura
29. **Logging** — sistema con niveles (debug/info/warn/error), a consola y archivo, timestamps; sustituir `printf` disperso.
30. **Profiling** — frame timer por pase (shadow/mirror/main/post), contadores draw calls/triángulos, overlay ImGui "Performance".
31. **Debug tools** — ventana ImGui de entidades del juego (inspect/editar en runtime), debug draw de colliders y raycasts.
32. **Memoria** — allocadores de frame (stack) para datos temporales de render; evitar allocaciones en el hot path.
33. **Tests unitarios** — math (vec/mat/transform), ECS (create/remove/serialize), raycast, serialización `.lev`; integrados en CI.
34. **CI + builds multiplataforma** — Linux + Windows en GitHub Actions; warnings como errores, clang-tidy básico.
35. **Sistema de configuración de proyectos** — separar "motor" (lib) de "juego" (código + contenido) para reutilizar el motor en otros juegos.

---

## 4. Refactors recomendados (antes de la Fase 2)

| Refactor | Motivo |
|---|---|
| Componente `mesh` + `material` en el ECS | El render asume `cube_mesh_`; no aguanta modelos reales |
| Sistema de audio central | Sin él no hay disparos ni ambiente |
| Sistema de UI propio en el juego (texto/sprites) | El HUD actual es una crosshair con shader ad-hoc |
| Física: mantener AABB propio o evaluar Bullet | Para puertas, destructibles y enemigos conviene rigid bodies |
| Versionar formato `.lev` | Cambios de componentes romperán mapas guardados |
| Pasar uniforms por entidad a un `material` real | Evitar 20+ `set_uniform` por entidad por frame |

---

## 5. Roadmap sugerido

```
Fase 1 (P0)  →  Fase 2 (P1)  →  Fase 3 (P2)  →  Fase 4 (P3)
  jugable        contenido        pulido          experiencia
  (armas,          (modelos,         (post-proceso,   (menús, opciones,
   vida, IA,        sonido,           rendimiento,      guardado,
   rondas)          partículas,       CSM, animación)   feedback)
                    mapas)
```

**Criterio de salida de cada fase:** la fase anterior debe jugarse de principio a fin sin glitches críticos (no crashear, colisiones correctas, rendimiento estable ≥ 60 FPS).
