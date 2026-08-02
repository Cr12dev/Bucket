#pragma once

// -- platform --
#include "platform/platform.hpp"

// -- core --
#include "core/init.hpp"
#include "core/window.hpp"
#include "core/timer.hpp"
#include "core/input.hpp"
#include "core/fps_counter.hpp"
#include "core/application.hpp"
#include "core/camera.hpp"

// -- math --
#include "core/math/vec2.hpp"
#include "core/math/vec3.hpp"
#include "core/math/vec4.hpp"
#include "core/math/mat4.hpp"
#include "core/math/transform.hpp"

// -- ecs --
#include "core/ecs/entity.hpp"
#include "core/ecs/component.hpp"
#include "core/ecs/scene.hpp"
#include "core/ecs/system.hpp"
#include "core/ecs/behaviour.hpp"
#include "core/ecs/behaviour_component.hpp"
#include "core/ecs/components/transform.hpp"
#include "core/ecs/components/tag.hpp"
#include "core/ecs/components/light.hpp"
#include "core/ecs/components/paint.hpp"

// -- tools --
#include "core/prefab.hpp"
#include "core/raycast.hpp"

// -- renderer --
#include "renderer/shader.hpp"
#include "renderer/buffer.hpp"
#include "renderer/vertex.hpp"
#include "renderer/mesh.hpp"
#include "renderer/texture.hpp"
#include "renderer/texture_cache.hpp"
#include "renderer/material.hpp"
#include "renderer/render_command.hpp"
#include "renderer/lighting.hpp"
#include "renderer/skybox.hpp"
#include "renderer/shadow_map.hpp"
#include "renderer/planar_mirror.hpp"

// -- game loop --
#include "core/game_loop.hpp"
