#include <Buckit.hpp>
#include "scripts/fps_player.hpp"

#include <cstdio>
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
              vec3(1.0f, 0.97f, 0.90f), 1.5f);

  // warm point light over the center bombsite
  prefab::point_light(scene_, vec3(0.0f, 4.5f, 0.0f),
                      vec3(1.0f, 0.92f, 0.78f), 6.0f, 16.0f);

  // cool accent light near the crate cluster
  prefab::point_light(scene_, vec3(-20.0f, 3.0f, -15.0f),
                      vec3(0.65f, 0.80f, 1.0f), 4.0f, 12.0f);

  build_map();

  vec3 spawn = vec3(0.0f, 1.7f, 0.0f);
  player_entity_ = scene_.create_entity();
  scene_.add_behaviour<fps_player>(player_entity_, &camera_, &scene_,
                                   app_->get_window().native(), spawn);

  std::printf("[bucket] scenario objects:\n");
  scene_.for_each<transform>([&](entity e, transform& t) {
    std::printf("  [%u] pos=(%.2f, %.2f, %.2f) size=(%.2f, %.2f, %.2f)\n",
                e.id, t.position.x, t.position.y, t.position.z,
                t.scale.x, t.scale.y, t.scale.z);
  });
}

// CS 1.6-style sandy arena built from engine prefabs
void buckit::build_map()
{
  const vec3 sand(0.80f, 0.75f, 0.58f);
  const vec3 wall(0.72f, 0.66f, 0.50f);
  const vec3 wood(0.52f, 0.36f, 0.20f);
  const vec3 wood_dark(0.42f, 0.30f, 0.18f);
  const vec3 stone(0.62f, 0.62f, 0.64f);
  const vec3 crate_blue(0.35f, 0.45f, 0.55f);
  const vec3 crate_green(0.35f, 0.52f, 0.35f);
  const vec3 platform(0.78f, 0.74f, 0.62f);

  // --- reflective floor and boundary walls ---
  entity floor_e = prefab::floor(scene_, vec3(0.0f, -0.25f, 0.0f), vec3(80.0f, 0.5f, 60.0f),
                                 sand, 0.45f);
  {
    paint& p = *scene_.get_component<paint>(floor_e);
    p.albedo = "textures/brick_albedo.png";
    p.roughness = "textures/brick_roughness.png";
    p.color = vec4(1.0f);
  }
  prefab::wall(scene_, vec3(0.0f, 3.0f, -30.0f), vec3(80.0f, 6.0f, 1.0f), wall);
  prefab::wall(scene_, vec3(0.0f, 3.0f, 30.0f), vec3(80.0f, 6.0f, 1.0f), wall);
  prefab::wall(scene_, vec3(-40.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 60.0f), wall);
  prefab::wall(scene_, vec3(40.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 60.0f), wall);

  // --- central bombsite platform with stairs ---
  prefab::box(scene_, vec3(0.0f, 0.75f, 0.0f), vec3(14.0f, 1.5f, 10.0f), platform);
  prefab::box(scene_, vec3(8.5f, 1.0f, 0.0f), vec3(3.0f, 2.0f, 8.0f), platform);
  prefab::box(scene_, vec3(10.5f, 2.0f, 0.0f), vec3(3.0f, 2.0f, 8.0f), platform);
  prefab::box(scene_, vec3(-8.5f, 1.0f, 0.0f), vec3(3.0f, 2.0f, 8.0f), platform);
  prefab::box(scene_, vec3(-10.5f, 2.0f, 0.0f), vec3(3.0f, 2.0f, 8.0f), platform);

  // crates on the platform: wood-textured + a glowing lamp crate
  entity crate_e = prefab::crate(scene_, vec3(-3.0f, 2.5f, 0.0f), 2.0f, wood);
  {
    paint& p = *scene_.get_component<paint>(crate_e);
    p.albedo = "textures/wood_albedo.png";
    p.normal = "textures/wood_normal.png";
    p.roughness = "textures/wood_roughness.png";
    p.color = vec4(1.0f);
  }

  entity lamp_e = prefab::crate(scene_, vec3(2.0f, 2.5f, -2.0f), 2.0f, wood_dark);
  {
    paint& p = *scene_.get_component<paint>(lamp_e);
    p.albedo = "textures/metal_albedo.png";
    p.roughness = "textures/metal_roughness.png";
    p.emission = "textures/emissive_lamp.png";
    p.emission_color = vec3(2.4f, 1.9f, 1.1f);
    p.color = vec4(1.0f);
  }

  // per-face albedo demo: every face gets its own texture
  entity face_e = prefab::crate(scene_, vec3(4.0f, 2.5f, 0.0f), 2.0f, stone);
  {
    paint& p = *scene_.get_component<paint>(face_e);
    p.normal = "textures/wood_normal.png";
    p.roughness = "textures/wood_roughness.png";
    p.color = vec4(1.0f);
    p.face_albedo[0] = "textures/wood_albedo.png";   // front
    p.face_albedo[1] = "textures/wood_albedo.png";   // back
    p.face_albedo[2] = "textures/metal_albedo.png";  // right
    p.face_albedo[3] = "textures/metal_albedo.png";  // left
    p.face_albedo[4] = "textures/brick_albedo.png";  // top
    p.face_albedo[5] = "textures/wood_albedo.png";   // bottom
  }

  // --- pillar ring ---
  prefab::pillar(scene_, vec3(-12.0f, 4.0f, -12.0f));
  prefab::pillar(scene_, vec3(12.0f, 4.0f, -12.0f));
  prefab::pillar(scene_, vec3(-12.0f, 4.0f, 12.0f));
  prefab::pillar(scene_, vec3(12.0f, 4.0f, 12.0f));

  // --- west/east wall segments with doorways ---
  prefab::wall(scene_, vec3(-25.0f, 3.0f, -12.0f), vec3(2.0f, 6.0f, 8.0f), wall);
  prefab::wall(scene_, vec3(-25.0f, 3.0f, 2.0f), vec3(2.0f, 6.0f, 8.0f), wall);
  prefab::wall(scene_, vec3(25.0f, 3.0f, -12.0f), vec3(2.0f, 6.0f, 8.0f), wall);
  prefab::wall(scene_, vec3(25.0f, 3.0f, 2.0f), vec3(2.0f, 6.0f, 8.0f), wall);

  // --- arch / tunnel frame ---
  prefab::pillar(scene_, vec3(-4.0f, 2.5f, 18.0f), 2.0f, 5.0f);
  prefab::pillar(scene_, vec3(4.0f, 2.5f, 18.0f), 2.0f, 5.0f);
  prefab::box(scene_, vec3(0.0f, 5.5f, 18.0f), vec3(12.0f, 1.0f, 2.0f), stone);

  // --- crate cluster NW ---
  entity c1 = prefab::crate(scene_, vec3(-20.0f, 1.0f, -15.0f), 2.0f, wood);
  entity c2 = prefab::crate(scene_, vec3(-20.0f, 3.0f, -15.0f), 2.0f, wood_dark);
  {
    paint& p1 = *scene_.get_component<paint>(c1);
    p1.albedo = "textures/wood_albedo.png";
    p1.normal = "textures/wood_normal.png";
    p1.roughness = "textures/wood_roughness.png";
    p1.color = vec4(1.0f);
    paint& p2 = *scene_.get_component<paint>(c2);
    p2.albedo = "textures/metal_albedo.png";
    p2.roughness = "textures/metal_roughness.png";
    p2.color = vec4(1.0f);
  }
  prefab::crate(scene_, vec3(-18.0f, 1.0f, -13.5f), 2.0f, crate_blue);
  prefab::crate(scene_, vec3(-16.0f, 1.0f, -15.0f), 2.0f, wood);
  prefab::crate(scene_, vec3(-16.0f, 3.0f, -15.0f), 2.0f, wood);

  // --- crate cluster SE ---
  prefab::crate(scene_, vec3(18.0f, 1.0f, 12.0f), 2.0f, wood);
  prefab::crate(scene_, vec3(20.0f, 1.0f, 14.0f), 2.0f, crate_green);
  prefab::crate(scene_, vec3(22.0f, 1.0f, 12.0f), 2.0f, wood_dark);

  // --- big blocks NE ---
  prefab::box(scene_, vec3(30.0f, 1.5f, -25.0f), vec3(3.0f, 3.0f, 3.0f), crate_blue);
  prefab::box(scene_, vec3(33.0f, 2.5f, -23.0f), vec3(3.0f, 5.0f, 3.0f), crate_green);

  // --- sandbags along the south wall ---
  prefab::sandbag(scene_, vec3(-10.0f, 0.5f, 28.25f));
  prefab::sandbag(scene_, vec3(-6.5f, 0.5f, 28.25f));
  prefab::sandbag(scene_, vec3(-3.0f, 0.5f, 28.25f));
  prefab::sandbag(scene_, vec3(4.0f, 0.5f, 28.25f));
  prefab::sandbag(scene_, vec3(7.5f, 0.5f, 28.25f));

  // --- scattered crates ---
  prefab::crate(scene_, vec3(-30.0f, 1.0f, 20.0f), 2.0f, crate_green);
  prefab::crate(scene_, vec3(-28.0f, 1.0f, 22.0f), 2.0f, wood);
  prefab::crate(scene_, vec3(36.0f, 1.0f, -5.0f), 2.0f, wood);
  prefab::crate(scene_, vec3(36.0f, 3.0f, -5.0f), 2.0f, wood_dark);
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

// Uploads the entity's paint (color, maps, per-face overrides) to a shader.
void buckit::bind_material(entity e, shader* s)
{
  const paint* p = scene_.get_component<paint>(e);

  vec4 color = p ? p->color : vec4(1.0f);
  if (e == hit_flash_ && hit_flash_timer_ > 0.0f) {
    vec3 flash = lerp(vec3(color.x, color.y, color.z),
                      vec3(1.0f, 0.9f, 0.2f), 0.8f);
    color = vec4(flash.x, flash.y, flash.z, color.w);
  }

  s->set_uniform("u_color", color.x, color.y, color.z, color.w);
  s->set_uniform("u_reflectivity", p ? p->reflectivity : 0.0f);
  s->set_uniform("u_uv_scale", p ? p->uv_scale : 0.0f);

  std::shared_ptr<texture> albedo = p ? texture_cache::load(p->albedo) : nullptr;
  std::shared_ptr<texture> normal = p ? texture_cache::load(p->normal) : nullptr;
  std::shared_ptr<texture> rough  = p ? texture_cache::load(p->roughness) : nullptr;
  std::shared_ptr<texture> emiss  = p ? texture_cache::load(p->emission) : nullptr;
  std::shared_ptr<texture> white  = texture_cache::white();

  (albedo ? albedo : white)->bind(0);
  s->set_uniform("u_albedo", 0);

  (normal ? normal : white)->bind(3);
  s->set_uniform("u_normal_map", 3);
  s->set_uniform("u_normal_enabled", normal ? 1.0f : 0.0f);

  (rough ? rough : white)->bind(4);
  s->set_uniform("u_roughness_map", 4);
  s->set_uniform("u_roughness_enabled", rough ? 1.0f : 0.0f);

  (emiss ? emiss : white)->bind(5);
  s->set_uniform("u_emission_map", 5);
  s->set_uniform("u_emission_enabled", emiss ? 1.0f : 0.0f);
  s->set_uniform("u_emission_color",
                 p ? p->emission_color.x : 0.0f,
                 p ? p->emission_color.y : 0.0f,
                 p ? p->emission_color.z : 0.0f);

  // per-face albedo overrides (texture units 6..11)
  for (int i = 0; i < 6; ++i) {
    std::shared_ptr<texture> face = p ? texture_cache::load(p->face_albedo[i]) : nullptr;
    (face ? face : white)->bind(6 + i);
    char name[40];
    std::snprintf(name, sizeof(name), "u_face_albedo[%d]", i);
    s->set_uniform(name, 6 + i);
    std::snprintf(name, sizeof(name), "u_face_albedo_enabled[%d]", i);
    s->set_uniform(name, face ? 1.0f : 0.0f);
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

void buckit::draw_crosshair(int w, int h)
{
  float cx = w * 0.5f;
  float cy = h * 0.5f;
  const float len = 7.0f;
  const float thick = 2.0f;

  mat4 ortho(glm::ortho(0.0f, static_cast<float>(w),
                        0.0f, static_cast<float>(h), -1.0f, 1.0f));

  glDisable(GL_DEPTH_TEST);
  default_shader_->bind();
  default_shader_->set_uniform("u_view_proj", ortho.data());
  default_shader_->set_uniform("u_color", 0.95f, 0.95f, 0.95f, 1.0f);

  auto bar = [&](float x, float y, float bw, float bh) {
    mat4 model = mat4::translate(vec3(x, y, 0.0f)) *
                 mat4::scale(vec3(bw, bh, 1.0f));
    default_shader_->set_uniform("u_model", model.data());
    render_command::draw_indexed(cube_mesh_);
  };

  bar(cx - len * 0.5f - thick * 0.5f, cy, len, thick);
  bar(cx + len * 0.5f + thick * 0.5f, cy, len, thick);
  bar(cx, cy - len * 0.5f - thick * 0.5f, thick, len);
  bar(cx, cy + len * 0.5f + thick * 0.5f, thick, len);

  glEnable(GL_DEPTH_TEST);
}

void buckit::render()
{
  auto& win = app_->get_window();
  int w = win.width();
  int h = win.height();

  render_command::set_clear_color(0.13f, 0.36f, 0.70f, 1.0f);
  render_command::clear();

  camera_.set_perspective(60.0f, static_cast<float>(w) / h, 0.1f, 100.0f);

  // skybox (procedural daytime atmosphere)
  skybox_.render(camera_);

  // gather lights from scene and track the sun direction
  lighting lights;
  scene_.for_each<light>([&](entity e, light& l) {
    (void)e;
    if (l.type == light_type::directional) {
      sun_dir_ = -l.direction.normalized();
      skybox_.set_sun_dir(sun_dir_);
    }
    lights.add(l);
  });

  bool full_lighting = active_shader_ == lighting_shader_.get() &&
                       shadow_map_.ready() && mirror_.ready();

  // --- planar reflection pass (mirror camera across the floor plane) ---
  if (full_lighting) {
    mirror_.resize(w / 2, h / 2);

    mirror_camera_ = camera_;
    mirror_camera_.set_position({ camera_.position().x,
                                  -camera_.position().y,
                                  camera_.position().z });
    mirror_camera_.set_pitch(-camera_.pitch());
    mirror_camera_.set_perspective(60.0f,
                                   static_cast<float>(w / 2) / (h / 2),
                                   0.1f, 100.0f);

    mirror_.begin();
    skybox_.render(mirror_camera_);

    lighting_shader_->bind();
    lighting_shader_->set_uniform("u_view_proj",
                                  mirror_camera_.view_projection().data());
    lighting_shader_->set_uniform("u_view_pos", mirror_camera_.position().x,
                                  mirror_camera_.position().y,
                                  mirror_camera_.position().z);
    lighting_shader_->set_uniform("u_ambient", 0.35f, 0.38f, 0.42f);
    lighting_shader_->set_uniform("u_fog_color", 0.75f, 0.80f, 0.88f);
    lighting_shader_->set_uniform("u_fog_start", 60.0f);
    lighting_shader_->set_uniform("u_fog_end", 140.0f);
    lighting_shader_->set_uniform("u_shadow_enabled", 0.0f);
    lighting_shader_->set_uniform("u_mirror_enabled", 0.0f);
    lighting_shader_->set_uniform("u_reflectivity", 0.0f);
    lighting_shader_->set_uniform("u_mirror_clip", 0.0f, 1.0f, 0.0f, 0.0f);
    lighting_shader_->set_uniform("u_mirror_clip_enabled", 1.0f);
    lights.upload(*lighting_shader_);

    scene_.for_each<transform>([&](entity e, transform& t) {
      bind_material(e, lighting_shader_.get());
      // never reflect the mirror into itself
      lighting_shader_->set_uniform("u_reflectivity", 0.0f);
      lighting_shader_->set_uniform("u_model", t.matrix().data());
      render_command::draw_indexed(cube_mesh_);
    });

    mirror_.end();
    glViewport(0, 0, w, h);
  }

  // --- shadow pass (directional light, PCF soft shadows) ---
  if (full_lighting) {
    shadow_map_.begin(sun_dir_, camera_.position());
    scene_.for_each<transform>([&](entity e, transform& t) {
      (void)e;
      shadow_map_.draw(t.matrix());
    });
    shadow_map_.end();
    glViewport(0, 0, w, h);
  }

  // --- main pass ---
  active_shader_->bind();
  active_shader_->set_uniform("u_view_proj", camera_.view_projection().data());
  active_shader_->set_uniform("u_camera_pos", camera_.position().x,
                              camera_.position().y, camera_.position().z);
  active_shader_->set_uniform("u_time", elapsed_);
  active_shader_->set_uniform("u_color_a", 0.1f, 0.1f, 0.4f, 1.0f);
  active_shader_->set_uniform("u_color_b", 0.9f, 0.7f, 0.2f, 1.0f);
  active_shader_->set_uniform("u_scale", 8.0f);

  if (full_lighting) {
    active_shader_->set_uniform("u_view_pos", camera_.position().x,
                                camera_.position().y, camera_.position().z);
    active_shader_->set_uniform("u_ambient", 0.35f, 0.38f, 0.42f);
    active_shader_->set_uniform("u_fog_color", 0.75f, 0.80f, 0.88f);
    active_shader_->set_uniform("u_fog_start", 60.0f);
    active_shader_->set_uniform("u_fog_end", 140.0f);

    shadow_map_.bind_depth(1);
    active_shader_->set_uniform("u_shadow_map", 1);
    active_shader_->set_uniform("u_light_view_proj",
                                shadow_map_.light_view_proj().data());
    active_shader_->set_uniform("u_shadow_enabled", 1.0f);
    active_shader_->set_uniform("u_shadow_bias", 0.004f);

    mirror_.bind_color(2);
    active_shader_->set_uniform("u_mirror_tex", 2);
    active_shader_->set_uniform("u_mirror_view_proj",
                                mirror_camera_.view_projection().data());
    active_shader_->set_uniform("u_mirror_enabled", 1.0f);
    active_shader_->set_uniform("u_mirror_clip_enabled", 0.0f);

    lights.upload(*lighting_shader_);
  }

  scene_.for_each<transform>([&](entity e, transform& t) {
    bind_material(e, active_shader_);
    active_shader_->set_uniform("u_model", t.matrix().data());
    render_command::draw_indexed(cube_mesh_);
  });

  draw_crosshair(w, h);
}
