#include <Buckit.hpp>

// ---------------------------------------------------------------------------
// Rendering side of the game loop: render pipeline passes, per-entity
// drawing and the crosshair. This file rarely changes; gameplay edits
// belong in game.cpp.
// ---------------------------------------------------------------------------

// Uploads the shared per-frame uniforms (view, fog, shadow, mirror, lights)
// to a shader. Uniform lookups are cached, so shaders without a given
// uniform skip it silently.
void buckit::bind_scene_uniforms(shader* s)
{
  s->set_uniform("u_view_proj", camera_.view_projection().data());
  s->set_uniform("u_view_pos", camera_.position().x,
                                camera_.position().y, camera_.position().z);
  s->set_uniform("u_ambient", atmosphere_.ambient.x, atmosphere_.ambient.y,
                              atmosphere_.ambient.z);        // ambient from the global atmosphere
  s->set_uniform("u_fog_color", atmosphere_.fog_color.x, atmosphere_.fog_color.y,
                                atmosphere_.fog_color.z);    // fog color from the global atmosphere
  s->set_uniform("u_fog_start", atmosphere_.fog_start);     // fog start distance
  s->set_uniform("u_fog_end", atmosphere_.fog_end);         // fog end distance
  s->set_uniform("u_fog_enabled", atmosphere_.fog_enabled ? 1.0f : 0.0f);

  if (shadow_map_.ready()) {
    shadow_map_.bind_depth(1);
    s->set_uniform("u_shadow_map", 1);
    s->set_uniform("u_light_view_proj", shadow_map_.light_view_proj().data());
    s->set_uniform("u_shadow_enabled", 1.0f);
    s->set_uniform("u_shadow_bias", 0.004f);
  }

  if (mirror_.ready()) {
    mirror_.bind_color(2);
    s->set_uniform("u_mirror_tex", 2);
    s->set_uniform("u_mirror_view_proj", mirror_camera_.view_projection().data());
    s->set_uniform("u_mirror_enabled", 1.0f);
    s->set_uniform("u_mirror_clip_enabled", 0.0f);
  }

  lights_.upload(*s);
}

// Draws every scene entity with the camera's frustum culling applied.
// Each entity uses its mesh (mesh_component) and material shader
// (paint::shader), falling back to the global active shader.
void buckit::draw_scene_entities()
{
  frustum view_frustum = frustum::from_view_proj(camera_.view_projection());

  shader* bound = nullptr;
  scene_.for_each<transform>([&](entity e, transform& t) {
    mat4 model = t.matrix();

    // frustum culling
    vec3 aabb_min, aabb_max;
    aabb_from_model(model, aabb_min, aabb_max);
    vec3 center = (aabb_min + aabb_max) * 0.5f;
    vec3 half = (aabb_max - aabb_min) * 0.5f;
    if (!view_frustum.contains(center, half))
      return;

    const paint* p = scene_.get_component<paint>(e);
    const mesh_component* mc = scene_.get_component<mesh_component>(e);

    shader* s = active_shader_;
    if (p && !p->shader.empty()) {
      std::shared_ptr<shader> custom = shader_cache::get(p->shader);
      if (custom) s = custom.get();
    }

    if (s != bound) {
      s->bind();
      bind_scene_uniforms(s);
      s->set_uniform("u_camera_pos", camera_.position().x,
                                    camera_.position().y, camera_.position().z);
      s->set_uniform("u_time", elapsed_);
      s->set_uniform("u_color_a", 0.1f, 0.1f, 0.4f, 1.0f);
      s->set_uniform("u_color_b", 0.9f, 0.7f, 0.2f, 1.0f);
      s->set_uniform("u_scale", 8.0f);
      bound = s;
    }

    material_bind::bind(s, p);
    if (e == hit_flash_ && hit_flash_timer_ > 0.0f) {
      vec4 color = p ? p->color : vec4(1.0f);
      vec3 flash = lerp(vec3(color.x, color.y, color.z),
                        vec3(1.0f, 0.9f, 0.2f), 0.8f);
      s->set_uniform("u_color", flash.x, flash.y, flash.z, color.w);
    }

    s->set_uniform("u_model", model.data());
    render_command::draw_indexed(*mesh_cache::get(mc ? mc->id : "cube"));
  });
}

void buckit::render()
{
  auto& win = app_->get_window();
  last_w_ = win.width();
  last_h_ = win.height();

  render_command::set_clear_color(atmosphere_.sky_horizon.x, atmosphere_.sky_horizon.y,
                                  atmosphere_.sky_horizon.z, 1.0f);
  render_command::clear();

  camera_.set_perspective(60.0f, static_cast<float>(last_w_) / last_h_, 0.1f, 100.0f);

  // skybox (procedural atmosphere, colors from atmosphere_)
  skybox_.render(camera_, atmosphere_);

  // gather lights from scene and track the sun direction
  lights_.clear();
  scene_.for_each<light>([&](entity e, light& l) {
    (void)e;
    if (l.type == light_type::directional) {
      sun_dir_ = -l.direction.normalized();
      skybox_.set_sun_dir(sun_dir_);
      // the global atmosphere drives the sun's color/intensity
      l.color = atmosphere_.sun_color;
      l.intensity = atmosphere_.sun_intensity;
    }
    lights_.add(l);
  });

  pipeline_.run();
}

// --- planar reflection pass (mirror camera across the floor plane) ---
void buckit::render_mirror_pass()
{
  if (active_shader_ != lighting_shader_.get() ||
      !shadow_map_.ready() || !mirror_.ready()) {
    return;
  }

  mirror_.resize(last_w_ / 2, last_h_ / 2);

  mirror_camera_ = camera_;
  mirror_camera_.set_position({ camera_.position().x,
                                -camera_.position().y,
                                camera_.position().z });
  mirror_camera_.set_pitch(-camera_.pitch());
  mirror_camera_.set_perspective(60.0f,
                                 static_cast<float>(last_w_ / 2) / (last_h_ / 2),
                                 0.1f, 100.0f);

  frustum mirror_frustum = frustum::from_view_proj(mirror_camera_.view_projection());

  mirror_.begin();
  skybox_.render(mirror_camera_, atmosphere_);

  lighting_shader_->bind();
  lighting_shader_->set_uniform("u_view_proj",
                                mirror_camera_.view_projection().data());
  lighting_shader_->set_uniform("u_view_pos", mirror_camera_.position().x,
                                mirror_camera_.position().y,
                                mirror_camera_.position().z);
  lighting_shader_->set_uniform("u_ambient", atmosphere_.ambient.x, atmosphere_.ambient.y,
                                              atmosphere_.ambient.z);
  lighting_shader_->set_uniform("u_fog_color", atmosphere_.fog_color.x, atmosphere_.fog_color.y,
                                               atmosphere_.fog_color.z);
  lighting_shader_->set_uniform("u_fog_start", atmosphere_.fog_start);
  lighting_shader_->set_uniform("u_fog_end", atmosphere_.fog_end);
  lighting_shader_->set_uniform("u_fog_enabled", atmosphere_.fog_enabled ? 1.0f : 0.0f);
  lighting_shader_->set_uniform("u_shadow_enabled", 0.0f);            // disable shadows in the mirror pass
  lighting_shader_->set_uniform("u_mirror_enabled", 0.0f);            // disable mirror reflections in the mirror pass
  lighting_shader_->set_uniform("u_reflectivity", 0.12f);              // disable reflectivity in the mirror pass
  lighting_shader_->set_uniform("u_mirror_clip", 0.0f, 1.0f, 0.0f, 0.0f);
  lighting_shader_->set_uniform("u_mirror_clip_enabled", 1.0f);
  lights_.upload(*lighting_shader_);

  scene_.for_each<transform>([&](entity e, transform& t) {
    mat4 model = t.matrix();

    vec3 aabb_min, aabb_max;
    aabb_from_model(model, aabb_min, aabb_max);
    vec3 center = (aabb_min + aabb_max) * 0.5f;
    vec3 half = (aabb_max - aabb_min) * 0.5f;
    if (!mirror_frustum.contains(center, half))
      return;

    material_bind::bind(lighting_shader_.get(), scene_.get_component<paint>(e));
    // never reflect the mirror into itself
    lighting_shader_->set_uniform("u_reflectivity", 0.0f);
    lighting_shader_->set_uniform("u_model", model.data());
    const mesh_component* mc = scene_.get_component<mesh_component>(e);
    render_command::draw_indexed(*mesh_cache::get(mc ? mc->id : "cube"));
  });

  mirror_.end();
  glViewport(0, 0, last_w_, last_h_);
}

// --- shadow pass (directional light, PCF soft shadows) ---
void buckit::render_shadow_pass()
{
  if (active_shader_ != lighting_shader_.get() ||
      !shadow_map_.ready() || !mirror_.ready()) {
    return;
  }

  shadow_map_.begin(sun_dir_, camera_.position());
  scene_.for_each<transform>([&](entity e, transform& t) {
    (void)e;
    const mesh_component* mc = scene_.get_component<mesh_component>(e);
    shadow_map_.draw(t.matrix(), *mesh_cache::get(mc ? mc->id : "cube"));
  });
  shadow_map_.end();
  glViewport(0, 0, last_w_, last_h_);
}

// --- main pass ---
void buckit::render_main_pass()
{
  active_shader_->bind();
  active_shader_->set_uniform("u_view_proj", camera_.view_projection().data());
  active_shader_->set_uniform("u_camera_pos", camera_.position().x,
                              camera_.position().y, camera_.position().z);
  active_shader_->set_uniform("u_time", elapsed_);
  active_shader_->set_uniform("u_color_a", 0.1f, 0.1f, 0.4f, 1.0f);
  active_shader_->set_uniform("u_color_b", 0.9f, 0.7f, 0.2f, 1.0f);
  active_shader_->set_uniform("u_scale", 8.0f);

  if (active_shader_ == lighting_shader_.get()) {
    bind_scene_uniforms(active_shader_);
  }

  draw_scene_entities();

  draw_crosshair(last_w_, last_h_);
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
