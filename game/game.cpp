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
  cube_mesh_ = mesh::cube();
  checker_ = texture::checkerboard();
}

void buckit::start()
{
  fps_.on_update([this](int fps) {
    char title[64];
    std::snprintf(title, sizeof(title), "Bucket Game  |  %d FPS", fps);
    glfwSetWindowTitle(app_->get_window().native(), title);
  });

  camera_.set_position({ 0.0f, 0.0f, 3.0f });

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
  if (transform* obj = object(0)) {
    obj->position.y = 1.0f;
  }

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
}

void buckit::render()
{
  auto& win = app_->get_window();
  int w = win.width();
  int h = win.height();

  render_command::set_clear_color(0.08f, 0.08f, 0.12f, 1.0f);
  render_command::clear();

  camera_.set_perspective(60.0f, static_cast<float>(w) / h, 0.1f, 100.0f);

  default_shader_->bind();
  default_shader_->set_uniform("u_view_proj", camera_.view_projection().data());

  checker_.bind(0);
  default_shader_->set_uniform("u_albedo", 0);
  default_shader_->set_uniform("u_color", 1.0f, 1.0f, 1.0f, 1.0f);

  scene_.for_each<transform>([&](entity e, transform& t) {
    (void)e;
    mat4 model = t.matrix();
    default_shader_->set_uniform("u_model", model.data());
    render_command::draw_indexed(cube_mesh_);
  });
}
