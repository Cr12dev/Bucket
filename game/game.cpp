#include <Buckit.hpp>
#include "scripts/fps_player.hpp"

#include <cstdio>
#include <cmath>
#include <glm/glm.hpp>

buckit::buckit()
  : app_(std::make_unique<application>("Bucket Game"))
{
}

void buckit::run()
{
  awake();
  start();

  app_->run(
    []() {},
    [this](double dt) { update(dt); },
    [this]() { render(); }
  );
}

void buckit::awake()
{
  glEnable(GL_DEPTH_TEST);

  default_shader_ = std::make_shared<shader>(
    "shaders/default.vert", "shaders/default.frag"
  );
  basic_shader_ = std::make_shared<shader>(
    "shaders/examples/basic.vert.glsl", "shaders/examples/basic.frag.glsl"
  );
  checker_shader_ = std::make_shared<shader>(
    "shaders/default.vert", "shaders/examples/checker.frag.glsl"
  );
  pulse_shader_ = std::make_shared<shader>(
    "shaders/default.vert", "shaders/examples/pulse.frag.glsl"
  );
  lighting_shader_ = std::make_shared<shader>(
    "shaders/lighting.vert", "shaders/lighting.frag"
  );

  // register material shaders for per-entity use (paint::shader)
  shader_cache::put("default", default_shader_);
  shader_cache::put("basic", basic_shader_);
  shader_cache::put("checker", checker_shader_);
  shader_cache::put("pulse", pulse_shader_);
  shader_cache::put("lighting", lighting_shader_);

  
  cube_mesh_ = mesh::cube();
  checker_ = texture::checkerboard();
  skybox_.init();
  shadow_map_.init(2048);
  mirror_.init();
}

void buckit::start()
{
  fps_.on_update([this](int fps) {
    char title[64];
    std::snprintf(title, sizeof(title), "Bucket Game  |  %d FPS", fps);
    glfwSetWindowTitle(app_->get_window().native(), title);
  });

  camera_.set_position({ 0.0f, 1.7f, 0.0f });

  std::printf("[bucket] shaders: [1] default  [2] basic  [3] checker  [4] pulse  [5] lighting\n");
  select_shader(5);

  // daytime sun (directional)
  prefab::sun(scene_, vec3(0.3f, -0.8f, -0.5f),
              vec3(1.0f, 0.97f, 0.90f), 1.5f, "sol");

  // warm point light over the center bombsite
  prefab::point_light(scene_, vec3(0.0f, 4.5f, 0.0f),
                      vec3(1.0f, 0.92f, 0.78f), 2.0f, 16.0f, "luz_central");

  // cool accent light near the crate cluster
  // prefab::point_light(scene_, vec3(-20.0f, 3.0f, -15.0f),
  //                     vec3(0.65f, 0.80f, 1.0f), 4.0f, 12.0f, "luz_fria");

  build_map();

  atmosphere_.fog_enabled = false;

  // grab the glowing lamp crate by its id so update() can edit it (demo)
  lamp_e_ = find_object("lampara");

  // make muro_norte a metal: the metallic map's R channel = how metallic it is
  // (white = fully metallic, black = dielectric). Loaded and applied at runtime.
  if (material* muro = component<material>("muro_norte")) {
    muro->metallic = "textures/metal_metallic.png";
  }

  // render pipeline: named passes executed in order every frame
  pipeline_.add("mirror", [this]() { render_mirror_pass(); });
  pipeline_.add("shadows", [this]() { render_shadow_pass(); });
  pipeline_.add("main", [this]() { render_main_pass(); });
  pipeline_.add("crosshair", [this]() { draw_crosshair(last_w_, last_h_); });

  vec3 spawn = vec3(0.0f, 1.7f, 0.0f);
  player_entity_ = scene_.create_entity();
  scene_.add_behaviour<fps_player>(player_entity_, &camera_, &scene_,
                                   app_->get_window().native(), spawn);

  std::printf("[bucket] scenario objects:\n");
  scene_.for_each<tag>([&](entity e, tag& t) {
    transform* tr = scene_.get_component<transform>(e);
    if (tr) {
      std::printf("  '%s' pos=(%.2f, %.2f, %.2f) size=(%.2f, %.2f, %.2f)\n",
                  t.name.c_str(), tr->position.x, tr->position.y, tr->position.z,
                  tr->scale.x, tr->scale.y, tr->scale.z);
    } else {
      std::printf("  '%s' (no transform)\n", t.name.c_str());
    }
  });
}

bool buckit::load_map(const char* path)
{
  bool ok = scene_.load<transform, tag>(path);
  if (!ok) {
    std::printf("[bucket] could not load map '%s'\n", path);
  }
  return ok;
}

transform* buckit::object(uint32_t id)
{
  return scene_.get_component<transform>(entity{ id, 0 });
}

entity buckit::find_object(const std::string& id) const
{
  entity found = null_entity();
  scene_.for_each<tag>([&](entity e, const tag& t) {
    if (t.name == id)
      found = e;
  });
  return found;
}

void buckit::shoot_ray()
{
  raycast_hit hit = raycast_scene(scene_, camera_.position(),
                                  camera_.forward(), 500.0f, player_entity_);
  hit_flash_ = hit.entity_hit;
  hit_flash_timer_ = hit.hit ? 0.35f : 0.0f;
  if (hit.hit) {
    std::printf("[bucket] ray hit entity %u at (%.2f, %.2f, %.2f)\n",
                hit.entity_hit.id, hit.point.x, hit.point.y, hit.point.z);
  } else {
    std::printf("[bucket] ray missed\n");
  }
}

void buckit::update(double dt)
{
  scene_.update(static_cast<float>(dt));
  fps_.tick(dt);

  elapsed_ += static_cast<float>(dt);

  if (hit_flash_timer_ > 0.0f) {
    hit_flash_timer_ -= static_cast<float>(dt);
    if (hit_flash_timer_ < 0.0f) hit_flash_timer_ = 0.0f;
  }
  if (shoot_cooldown_ > 0.0f) {
    shoot_cooldown_ -= static_cast<float>(dt);
  }
  if (input::mouse_button_down(GLFW_MOUSE_BUTTON_LEFT) && shoot_cooldown_ <= 0.0f) {
    shoot_ray();
    shoot_cooldown_ = 0.25f;
  }

  if (input::key_pressed(GLFW_KEY_1)) select_shader(1);
  if (input::key_pressed(GLFW_KEY_2)) select_shader(2);
  if (input::key_pressed(GLFW_KEY_3)) select_shader(3);
  if (input::key_pressed(GLFW_KEY_4)) select_shader(4);
  if (input::key_pressed(GLFW_KEY_5)) select_shader(5);

  // --- demo: edit map objects from update() by their id string ---
  // Any object created with a prefab has a mandatory id. Find it once
  // (find_object) or edit directly by name from update():
  //   component<transform>("muro_norte")->position.x = 0;   // pos / rot / scale
  //   component<paint>("caja_madera1")->color = vec4(1, 0, 0, 1);
  //   component<paint>("lampara")->emission_color = vec3(3, 2, 1);
  //   component<light>("luz_central")->intensity = 10;      // range / color / ...
  //   component<mesh_component>("caja_caras")->id = "quad";
  // if (lamp_e_) {
  //   transform* lt = component<transform>(lamp_e_.id);
  //   paint* lp = component<paint>(lamp_e_.id);
  //   if (lt && lp) {
  //     // pulse the emission intensity
  //     float pulse = 0.5f + 0.5f * std::sin(elapsed_ * 2.2f);
  //     lp->emission_color = vec3(1.2f + 2.4f * pulse, 0.9f + 1.9f * pulse, 0.5f + 1.1f * pulse);
  //     // slow spin around Y
  //     lt->rotation.y += static_cast<float>(dt) * 1.6f;
  //   }
  // }

  /**
   * Ejemplo de movimiento de un objeto del mapa (muro_norte) desde el update() del juego.
   * Se puede editar cualquier componente de un objeto del mapa desde el update() del juego.
   * En este caso el movimiento es en el eje Z, y se mueve hacia adelante y hacia atrás entre -12.0f y 0.0f.
   */
  transform* muro_norte_transform = component<transform>("muro_norte");  
  if (muro_norte_transform) {
    

    if (muro_norte_transform->position.z > -12.0f) {
      muro_norte_transform->position.z += static_cast<float>(dt) * 0;
    } else {
      muro_norte_transform->position.z += static_cast<float>(dt) * 0.5f;
    }
  }

  /**
   * Ejemplo de cambio de color de un objeto del mapa (muro_norte) desde el update() del juego.
   * Se puede editar cualquier componente de un objeto del mapa desde el update() del juego.
   * En este caso el color cambia a rojo, verde o azul dependiendo de la tecla presionada (R, G o B).
   * Se utiliza el componente paint del objeto para cambiar su color.
   */

  paint* muro_norte_paint = component<paint>("muro_norte");
  if (muro_norte_paint) {
    // Cambiar el color del muro_norte a rojo cuando se presiona la tecla 'R'
    if (input::key_pressed(GLFW_KEY_R)) {
      muro_norte_paint->color = vec4(1.0f, 0.0f, 0.0f, 1.0f); // Rojo
    }
    // Cambiar el color del muro_norte a verde cuando se presiona la tecla 'G'
    if (input::key_pressed(GLFW_KEY_G)) {
      muro_norte_paint->color = vec4(0.0f, 1.0f, 0.0f, 1.0f); // Verde
    }
    // Cambiar el color del muro_norte a azul cuando se presiona la tecla 'B'
    if (input::key_pressed(GLFW_KEY_B)) {
      muro_norte_paint->color = vec4(0.0f, 0.0f, 1.0f, 1.0f); // Azul
    }
  }

  /**
   * Ejemplo de cambio de color de un objeto del mapa (suelo) desde el update() del juego.
   * Se puede editar cualquier componente de un objeto del mapa desde el update() del juego.
   * En este caso el color cambia a amarillo cuando se presiona la tecla 'Y'.
   * Se utiliza el componente paint del objeto para cambiar su color.
   */
  paint* floor = component<paint>("suelo");
  if (floor) {
    // Cambiar el color de la plataforma principal a amarillo cuando se presiona la tecla 'Y'
    if (input::key_pressed(GLFW_KEY_Y)) {
      floor->color = vec4(1.0f, 1.0f, 0.0f, 1.0f); // Amarillo
    }
  }
  material* floor_mt = component<material>("suelo");
  if (floor_mt) {
    floor_mt->albedo = "";
  }

  /**
   * Ejemplo de edicion del material de un objeto (lampara) desde el update().
   * El material es el componente paint, accesible con el alias `material`
   * para que quede mas claro: component<material>("id")->shader / albedo / ...
   */
  material* lampara_mat = component<material>("lampara");
  if (lampara_mat) {
    // [6] cambia el shader del material en tiempo real
    if (input::key_pressed(GLFW_KEY_6)) {
      lampara_mat->shader = (lampara_mat->shader == "checker") ? "lighting" : "checker";
      std::printf("[bucket] lampara shader -> '%s'\n", lampara_mat->shader.c_str());
    }
  }

  /**
   * Ejemplo de edicion de la atmosfera global desde el update().
   * buckit::atmosphere_ controla la luz ambiente (sombras), la niebla y el
   * cielo. La tecla [7] cicla entre presets. Tambien se puede editar campo
   * a campo en cualquier momento:
   *   atmosphere_.ambient = vec3(0.32f, 0.30f, 0.26f);    // relleno (color de sombras)
   *   atmosphere_.fog_color = vec3(0.82f, 0.78f, 0.70f);  // niebla
   *   atmosphere_.fog_start = 45.0f; atmosphere_.fog_end = 95.0f;
   *   atmosphere_.sky_top = vec3(0.26f, 0.48f, 0.82f);    // zenith
   *   atmosphere_.sky_horizon = vec3(0.80f, 0.84f, 0.88f);
   *   atmosphere_.sky_ground = vec3(0.48f, 0.42f, 0.34f);
   *   atmosphere_.sun_color = vec3(1.0f, 0.97f, 0.90f);   // color del sol
   *   atmosphere_.sun_intensity = 1.5f;
   */
  if (input::key_pressed(GLFW_KEY_7)) {
    atmosphere_preset_ = (atmosphere_preset_ + 1) % 3;
    switch (atmosphere_preset_) {
      case 0:
        atmosphere_ = atmosphere();
        std::printf("[bucket] atmosfera: dia (default)\n");
        break;
      case 1: {
        // nublado: gris neutro, sin tinte azul
        atmosphere_.ambient = vec3(0.34f, 0.34f, 0.32f);
        atmosphere_.fog_color = vec3(0.78f, 0.78f, 0.76f);
        atmosphere_.fog_start = 40.0f; atmosphere_.fog_end = 90.0f;
        atmosphere_.sky_top = vec3(0.56f, 0.57f, 0.58f);
        atmosphere_.sky_horizon = vec3(0.74f, 0.74f, 0.72f);
        atmosphere_.sky_ground = vec3(0.50f, 0.48f, 0.44f);
        atmosphere_.sun_color = vec3(0.95f, 0.94f, 0.92f);
        atmosphere_.sun_intensity = 1.0f;
        std::printf("[bucket] atmosfera: nublado (neutro)\n");
        break;
      }
      default: {
        // atardecer: calido
        atmosphere_.ambient = vec3(0.30f, 0.22f, 0.16f);
        atmosphere_.fog_color = vec3(0.96f, 0.78f, 0.55f);
        atmosphere_.fog_start = 30.0f; atmosphere_.fog_end = 70.0f;
        atmosphere_.sky_top = vec3(0.42f, 0.30f, 0.42f);
        atmosphere_.sky_horizon = vec3(0.98f, 0.72f, 0.42f);
        atmosphere_.sky_ground = vec3(0.45f, 0.34f, 0.24f);
        atmosphere_.sun_color = vec3(1.0f, 0.85f, 0.60f);
        atmosphere_.sun_intensity = 1.8f;
        std::printf("[bucket] atmosfera: atardecer\n");
        break;
      }
    }
  }

  // [8] activa / desactiva la niebla
  if (input::key_pressed(GLFW_KEY_8)) {
    atmosphere_.fog_enabled = !atmosphere_.fog_enabled;
    std::printf("[bucket] fog: %s\n", atmosphere_.fog_enabled ? "activado" : "desactivado");
  }
}

void buckit::select_shader(int index)
{
  switch (index) {
    case 1: active_shader_ = default_shader_.get(); break;
    case 2: active_shader_ = basic_shader_.get();  break;
    case 3: active_shader_ = checker_shader_.get(); break;
    case 4: active_shader_ = pulse_shader_.get();  break;
    case 5: active_shader_ = lighting_shader_.get(); break;
    default: return;
  }
  std::printf("[bucket] active shader: %d\n", index);
}
