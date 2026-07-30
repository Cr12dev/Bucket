#include <Buckit.hpp>
#include "scripts/free_look.hpp"
#include "scripts/orbit.hpp"

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
  camera_.set_position({ 0.0f, 0.0f, 3.0f });

  entity player = scene_.create_entity();
  scene_.add_component<transform>(player);
  scene_.add_behaviour<free_look>(player, &camera_);

  entity obj = scene_.create_entity();
  scene_.add_component<transform>(obj);
  scene_.add_behaviour<orbit>(obj);
}

void buckit::update(double dt)
{
  scene_.update(static_cast<float>(dt));
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
