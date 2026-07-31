#include <Buckit.hpp>
#include "scripts/free_look.hpp"
#include "scripts/orbit.hpp"

#include <cstdio>

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
}

void buckit::start()
{
  fps_.on_update([this](int fps) {
    char title[64];
    std::snprintf(title, sizeof(title), "Bucket Game  |  %d FPS", fps);
    glfwSetWindowTitle(app_->get_window().native(), title);
  });

  camera_.set_position({ 0.0f, 0.0f, 3.0f });

  std::printf("[bucket] shaders: [1] default  [2] basic  [3] checker  [4] pulse  [5] lighting\n");
  select_shader(5);

  // sun (directional)
  {
    entity sun = scene_.create_entity();
    light& l = scene_.add_component<light>(sun);
    l.type = light_type::directional;
    l.color = { 1.0f, 0.95f, 0.85f };
    l.intensity = 1.2f;
    l.direction = { 0.3f, -0.8f, -0.5f };
  }

  // point light
  {
    entity pt = scene_.create_entity();
    light& l = scene_.add_component<light>(pt);
    l.type = light_type::point;
    l.color = { 0.4f, 0.7f, 1.0f };
    l.intensity = 4.0f;
    l.position = { 1.2f, 1.0f, 0.8f };
    l.range = 6.0f;
  }

  // bind the level map; everything saved in the editor is replicated here
  bool map_loaded = load_map("level.lev");

  entity player = scene_.create_entity();
  scene_.add_component<transform>(player);
  scene_.add_behaviour<free_look>(player, &camera_);

  if (!map_loaded) {
    entity obj = scene_.create_entity();
    scene_.add_component<transform>(obj);
    scene_.add_behaviour<orbit>(obj);
  }

  // the map is now bound to the game: edit objects by their id
  // if (transform* obj = object(0)) {
  //   obj->position.y = 1.0f;
  // }

  std::printf("[bucket] scene objects:\n");
  scene_.for_each<transform>([&](entity e, transform& t) {
    std::printf("  [%u] pos=(%.2f, %.2f, %.2f)\n",
                e.id, t.position.x, t.position.y, t.position.z);
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

void buckit::update(double dt)
{
  scene_.update(static_cast<float>(dt));
  fps_.tick(dt);

  elapsed_ += static_cast<float>(dt);

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

void buckit::render()
{
  auto& win = app_->get_window();
  int w = win.width();
  int h = win.height();

  render_command::set_clear_color(0.08f, 0.08f, 0.12f, 1.0f);
  render_command::clear();

  camera_.set_perspective(60.0f, static_cast<float>(w) / h, 0.1f, 100.0f);

  // skybox (procedural atmosphere)
  skybox_.render(camera_);

  // gather lights from scene
  lighting lights;
  scene_.for_each<light>([&](entity e, light& l) {
    (void)e;
    if (l.type == light_type::directional) {
      skybox_.set_sun_dir(-l.direction.normalized());
    }
    lights.add(l);
  });

  active_shader_->bind();
  active_shader_->set_uniform("u_view_proj", camera_.view_projection().data());
  active_shader_->set_uniform("u_camera_pos", camera_.position().x,
                              camera_.position().y, camera_.position().z);
  active_shader_->set_uniform("u_time", elapsed_);
  active_shader_->set_uniform("u_color_a", 0.1f, 0.1f, 0.4f, 1.0f);
  active_shader_->set_uniform("u_color_b", 0.9f, 0.7f, 0.2f, 1.0f);
  active_shader_->set_uniform("u_scale", 8.0f);

  if (active_shader_ == lighting_shader_.get()) {
    active_shader_->set_uniform("u_view_pos", camera_.position().x,
                                camera_.position().y, camera_.position().z);
    active_shader_->set_uniform("u_ambient", 0.22f, 0.24f, 0.28f);
    active_shader_->set_uniform("u_fog_color", 0.68f, 0.74f, 0.80f);
    active_shader_->set_uniform("u_fog_start", 10.0f);
    active_shader_->set_uniform("u_fog_end", 30.0f);
    lights.upload(*lighting_shader_);
  }

  checker_.bind(0);
  active_shader_->set_uniform("u_albedo", 0);
  active_shader_->set_uniform("u_color", 1.0f, 1.0f, 1.0f, 1.0f);

  scene_.for_each<transform>([&](entity e, transform& t) {
    (void)e;
    mat4 model = t.matrix();
    active_shader_->set_uniform("u_model", model.data());
    render_command::draw_indexed(cube_mesh_);
  });
}
