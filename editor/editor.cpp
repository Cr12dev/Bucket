#include "editor.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <Buckit.hpp>
#include <core/ecs/components/tag.hpp>
#include <core/ecs/components/transform.hpp>
#include <core/math/transform.hpp>
#include <renderer/render_command.hpp>

namespace {
constexpr float kPi = 3.14159265f;

float rad_to_deg(float rad) { return rad * 180.0f / kPi; }
float deg_to_rad(float deg) { return deg * kPi / 180.0f; }
} // namespace

editor::editor()
  : app_(std::make_unique<application>("Bucket Editor"))
{
}

void editor::run()
{
  awake();
  start();

  app_->run(
    []() {},
    [this](double dt) { update(dt); },
    [this]() { render(); }
  );

  shutdown_imgui();
}

void editor::awake()
{
  glEnable(GL_DEPTH_TEST);

  default_shader_ = std::make_shared<shader>(
    "shaders/default.vert", "shaders/default.frag"
  );
  lighting_shader_ = std::make_shared<shader>(
    "shaders/lighting.vert", "shaders/lighting.frag"
  );
  cube_mesh_ = mesh::cube();
  white_tex_ = texture::white();
  skybox_.init();
}

void editor::start()
{
  init_imgui();

  camera_.set_position({ 0.0f, 0.0f, 3.0f });

  // default scene with a few primitives
  const char* names[] = { "Cube", "Sphere", "Cylinder" };
  for (int i = 0; i < 3; ++i) {
    entity e = scene_.create_entity();
    auto& t = scene_.add_component<transform>(e);
    t.position.x = static_cast<float>(i) * 1.5f - 1.5f;
    scene_.add_component<tag>(e, names[i]);
  }

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
}

void editor::update(double dt)
{
  (void)dt;
  auto& win = app_->get_window();

  // -- editor camera orbit (middle or right mouse over the viewport) --
  double mx, my;
  glfwGetCursorPos(win.native(), &mx, &my);

  bool orbit =
    glfwGetMouseButton(win.native(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS ||
    glfwGetMouseButton(win.native(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

  if (viewport_hovered_ && orbit) {
    if (!dragging_) {
      dragging_ = true;
      last_mx_ = mx;
      last_my_ = my;
    }
    double dx = mx - last_mx_;
    double dy = my - last_my_;
    cam_yaw_ += static_cast<float>(dx) * 0.005f;
    cam_pitch_ += static_cast<float>(dy) * 0.005f;
    cam_pitch_ = std::max(-1.5f, std::min(1.5f, cam_pitch_));
    last_mx_ = mx;
    last_my_ = my;
  } else {
    dragging_ = false;
  }

  // scroll zoom (only while hovering the viewport)
  static double scroll_y = 0.0;
  static bool scroll_init = false;
  if (!scroll_init) {
    glfwSetScrollCallback(win.native(), [](GLFWwindow*, double, double y) {
      scroll_y += y;
    });
    scroll_init = true;
  }
  if (viewport_hovered_) {
    cam_dist_ -= static_cast<float>(scroll_y * 0.5);
  }
  scroll_y = 0.0;
  if (cam_dist_ < 0.5f) cam_dist_ = 0.5f;
  if (cam_dist_ > 50.0f) cam_dist_ = 50.0f;

  // update camera position & orientation (yaw/pitch in radians)
  float cx = cam_dist_ * std::cos(cam_pitch_) * std::sin(cam_yaw_);
  float cy = cam_dist_ * std::sin(cam_pitch_);
  float cz = cam_dist_ * std::cos(cam_pitch_) * std::cos(cam_yaw_);
  camera_.set_position({ cx, cy, cz });
  camera_.set_yaw(cam_yaw_ * 180.0f / kPi + 90.0f);
  camera_.set_pitch(-cam_pitch_ * 180.0f / kPi);

  // -- imgui new frame --
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();

  // -- keyboard shortcuts (ignored while typing in imgui widgets) --
  ImGuiIO& io = ImGui::GetIO();
  if (!io.WantCaptureKeyboard) {
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && selected_) {
      delete_entity(selected_);
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D) && selected_) {
      duplicate_entity(selected_);
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
      scene_ = scene{};
      selected_ = null_entity();
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
      map_popup_save_ = true;
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
      map_popup_load_ = true;
    }
  }
}

void editor::render()
{
  auto& win = app_->get_window();
  int ww = win.width();
  int wh = win.height();

  // clear main framebuffer (background behind imgui)
  render_command::set_viewport(0, 0, ww, wh);
  render_command::set_clear_color(0.08f, 0.08f, 0.10f, 1.0f);
  render_command::clear();

  // -- build gui --
  build_menu_bar();
  build_dockspace();
  if (show_viewport_) build_viewport();
  if (show_hierarchy_) build_hierarchy();
  if (show_properties_) build_properties();
  if (show_demo_) {
    ImGui::ShowDemoWindow(&show_demo_);
  }
  build_map_dialogs();
  build_status_bar();

  // -- render imgui overlay --
  ImGui::Render();
  glViewport(0, 0, ww, wh);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void editor::build_dockspace()
{
  ImGuiWindowFlags flags =
    ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus;

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  float status_height = ImGui::GetFrameHeight() + 8.0f;

  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - status_height));
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("##DockSpace", nullptr, flags);
  ImGui::PopStyleVar(3);

  ImGuiID dockspace_id = ImGui::GetID("BucketDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

  if (!dockspace_init_) {
    dockspace_init_ = true;

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

    ImGuiID left, center, right;
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.22f, &left, &center);
    ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.26f, &right, &center);

    ImGui::DockBuilderDockWindow("Scene Hierarchy", left);
    ImGui::DockBuilderDockWindow("Viewport", center);
    ImGui::DockBuilderDockWindow("Properties", right);
    ImGui::DockBuilderFinish(dockspace_id);
  }

  ImGui::End();
}

void editor::build_menu_bar()
{
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
        scene_ = scene{};
        selected_ = null_entity();
      }
      if (ImGui::MenuItem("Save Map", "Ctrl+S")) {
        map_popup_save_ = true;
      }
      if (ImGui::MenuItem("Load Map", "Ctrl+O")) {
        map_popup_load_ = true;
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        glfwSetWindowShouldClose(app_->get_window().native(), true);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      bool has_sel = static_cast<bool>(selected_);
      if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, has_sel)) {
        duplicate_entity(selected_);
      }
      if (ImGui::MenuItem("Delete", "Del", false, has_sel)) {
        delete_entity(selected_);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("Viewport", nullptr, &show_viewport_);
      ImGui::MenuItem("Scene Hierarchy", nullptr, &show_hierarchy_);
      ImGui::MenuItem("Properties", nullptr, &show_properties_);
      ImGui::Separator();
      ImGui::MenuItem("Show Grid", nullptr, &show_grid_);
      ImGui::MenuItem("Demo Window", nullptr, &show_demo_);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("About Bucket Editor");
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

void editor::build_viewport()
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Viewport");

  // -- viewport toolbar --
  {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 3));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
    ImGui::BeginChild("##viewport_toolbar", ImVec2(0, 28), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::TextUnformatted("Transform");
    ImGui::SameLine();

    auto tool_button = [this](const char* label, gizmo_mode mode) {
      bool active = (gizmo_mode_ == mode);
      if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TabActive));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_TabActive));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_TabActive));
      }
      if (ImGui::Button(label)) {
        gizmo_mode_ = mode;
      }
      if (active) ImGui::PopStyleColor(3);
      ImGui::SameLine();
    };

    tool_button("Move", gizmo_mode::translate);
    tool_button("Rotate", gizmo_mode::rotate);
    tool_button("Scale", gizmo_mode::scale);

    ImGui::Checkbox("Snap", &gizmo_snap_);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &show_grid_);

    ImGui::EndChild();
    ImGui::PopStyleVar(3);
  }

  ImVec2 size = ImGui::GetContentRegionAvail();
  int w = static_cast<int>(size.x);
  int h = static_cast<int>(size.y);
  viewport_hovered_ = false;

  if (w <= 0 || h <= 0) {
    ImGui::End();
    ImGui::PopStyleVar();
    return;
  }

  if (w != vp_w_ || h != vp_h_) {
    vp_w_ = w;
    vp_h_ = h;

    if (!fbo_) {
      glGenFramebuffers(1, &fbo_);
      glGenTextures(1, &color_tex_);
      glGenRenderbuffers(1, &rbo_);
    }

    glBindTexture(GL_TEXTURE_2D, color_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, vp_w_, vp_h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, vp_w_, vp_h_);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);
  }

  // render scene to FBO
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glViewport(0, 0, vp_w_, vp_h_);
  render_command::set_clear_color(0.16f, 0.16f, 0.20f, 1.0f);
  render_command::clear();

  camera_.set_perspective(60.0f, static_cast<float>(vp_w_) / vp_h_, 0.1f, 100.0f);

  // procedural skybox (atmosphere)
  skybox_.render(camera_);

  // grid
  if (show_grid_) {
    float grid_color[4] = { 0.35f, 0.38f, 0.45f, 1.0f };
    render_command::draw_grid(20, 1.0f, camera_.view_projection().data(), grid_color);
  }

  // gather lights
  lighting lights;
  scene_.for_each<light>([&](entity e, light& l) {
    (void)e;
    if (l.type == light_type::directional) {
      skybox_.set_sun_dir(-l.direction.normalized());
    }
    lights.add(l);
  });

  // lit objects
  lighting_shader_->bind();
  lighting_shader_->set_uniform("u_view_proj", camera_.view_projection().data());
  lighting_shader_->set_uniform("u_view_pos", camera_.position().x,
                                camera_.position().y, camera_.position().z);
  lighting_shader_->set_uniform("u_ambient", 0.25f, 0.26f, 0.30f);
  lighting_shader_->set_uniform("u_fog_color", 0.68f, 0.74f, 0.80f);
  lighting_shader_->set_uniform("u_fog_start", 20.0f);
  lighting_shader_->set_uniform("u_fog_end", 60.0f);
  lights.upload(*lighting_shader_);

  scene_.for_each<transform>([&](entity e, transform& t) {
    bool sel = (e == selected_);
    const paint* p = scene_.get_component<paint>(e);
    lighting_shader_->set_uniform("u_color",
      sel ? 1.0f : 0.6f,
      sel ? 0.8f : 0.6f,
      sel ? 0.2f : 0.7f,
      1.0f);
    lighting_shader_->set_uniform("u_uv_scale", p ? p->uv_scale : 0.0f);

    // per-entity texture binding (empty slots fall back to white)
    std::shared_ptr<texture> white = texture_cache::white();
    std::shared_ptr<texture> albedo = p ? texture_cache::load(p->albedo) : nullptr;
    std::shared_ptr<texture> normal = p ? texture_cache::load(p->normal) : nullptr;
    std::shared_ptr<texture> rough  = p ? texture_cache::load(p->roughness) : nullptr;
    std::shared_ptr<texture> emiss  = p ? texture_cache::load(p->emission) : nullptr;

    (albedo ? albedo : white)->bind(0);
    lighting_shader_->set_uniform("u_albedo", 0);
    (normal ? normal : white)->bind(3);
    lighting_shader_->set_uniform("u_normal_map", 3);
    lighting_shader_->set_uniform("u_normal_enabled", normal ? 1.0f : 0.0f);
    (rough ? rough : white)->bind(4);
    lighting_shader_->set_uniform("u_roughness_map", 4);
    lighting_shader_->set_uniform("u_roughness_enabled", rough ? 1.0f : 0.0f);
    (emiss ? emiss : white)->bind(5);
    lighting_shader_->set_uniform("u_emission_map", 5);
    lighting_shader_->set_uniform("u_emission_enabled", emiss ? 1.0f : 0.0f);
    lighting_shader_->set_uniform("u_emission_color",
                                  p ? p->emission_color.x : 0.0f,
                                  p ? p->emission_color.y : 0.0f,
                                  p ? p->emission_color.z : 0.0f);
    for (int i = 0; i < 6; ++i) {
      std::shared_ptr<texture> face = p ? texture_cache::load(p->face_albedo[i]) : nullptr;
      (face ? face : white)->bind(6 + i);
      char name[40];
      std::snprintf(name, sizeof(name), "u_face_albedo[%d]", i);
      lighting_shader_->set_uniform(name, 6 + i);
      std::snprintf(name, sizeof(name), "u_face_albedo_enabled[%d]", i);
      lighting_shader_->set_uniform(name, face ? 1.0f : 0.0f);
    }

    mat4 model = t.matrix();
    lighting_shader_->set_uniform("u_model", model.data());
    render_command::draw_indexed(cube_mesh_);
  });

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // display in imgui
  ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(color_tex_)),
               ImVec2(static_cast<float>(vp_w_), static_cast<float>(vp_h_)),
               ImVec2(0, 1), ImVec2(1, 0));

  ImVec2 img_min = ImGui::GetItemRectMin();
  ImVec2 img_max = ImGui::GetItemRectMax();
  viewport_hovered_ = ImGui::IsItemHovered();

  // click empty space to deselect
  if (viewport_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
    selected_ = null_entity();
  }

  // -- overlay info --
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  char overlay[64];
  std::snprintf(overlay, sizeof(overlay), "%.0f FPS", ImGui::GetIO().Framerate);
  draw_list->AddText(img_min + ImVec2(8, 6),
                     ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 0.85f)), overlay);
  if (selected_) {
    std::snprintf(overlay, sizeof(overlay), "%s", entity_label(selected_).c_str());
    ImVec2 text_size = ImGui::CalcTextSize(overlay);
    draw_list->AddText(img_max - ImVec2(text_size.x + 8, 22),
                       ImGui::ColorConvertFloat4ToU32(ImVec4(0.9f, 0.7f, 0.3f, 1.0f)), overlay);
  }

  // -- transform gizmo --
  if (selected_ && scene_.has_component<transform>(selected_)) {
    auto* t = scene_.get_component<transform>(selected_);

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(draw_list);
    ImGuizmo::SetRect(img_min.x, img_min.y, img_max.x - img_min.x, img_max.y - img_min.y);

    float view[16], proj[16], model[16];
    std::memcpy(view, camera_.view_matrix().data(), sizeof(view));
    std::memcpy(proj, camera_.projection_matrix().data(), sizeof(proj));
    std::memcpy(model, t->matrix().data(), sizeof(model));

    ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
    if (gizmo_mode_ == gizmo_mode::rotate) {
      op = ImGuizmo::ROTATE;
    } else if (gizmo_mode_ == gizmo_mode::scale) {
      op = ImGuizmo::SCALE;
    }

    float snap_vals[3] = { 0.25f, 0.25f, 0.25f };
    const float* snap = nullptr;
    if (gizmo_snap_) {
      if (op == ImGuizmo::ROTATE) {
        snap_vals[0] = snap_vals[1] = snap_vals[2] = 15.0f;
      }
      snap = snap_vals;
    }

    if (ImGuizmo::Manipulate(view, proj, op, ImGuizmo::WORLD, model, nullptr, snap)) {
      glm::vec3 scale, skew, translation;
      glm::quat orientation;
      glm::vec4 perspective;
      if (glm::decompose(glm::make_mat4(model), scale, orientation, translation, skew, perspective)) {
        t->position = { translation.x, translation.y, translation.z };
        glm::vec3 euler = glm::eulerAngles(orientation);
        t->rotation = { euler.x, euler.y, euler.z };
        t->scale = { scale.x, scale.y, scale.z };
      }
    }
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void editor::build_hierarchy()
{
  ImGui::Begin("Scene Hierarchy");

  if (ImGui::Button("+ Add Entity")) {
    create_entity("Entity");
  }

  ImGui::Separator();

  std::vector<entity> entities;
  scene_.for_each<transform>([&](entity e, transform&) {
    entities.push_back(e);
  });

  if (entities.empty()) {
    ImGui::TextDisabled("Scene is empty");
  }

  for (entity e : entities) {
    std::string label = entity_label(e) + "##" + std::to_string(e.id);
    bool selected = (e == selected_);
    if (ImGui::Selectable(label.c_str(), selected)) {
      selected_ = e;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      selected_ = e;
      ImGui::OpenPopup("##hierarchy_context");
    }
  }

  if (ImGui::BeginPopup("##hierarchy_context")) {
    if (ImGui::MenuItem("Duplicate")) {
      duplicate_entity(selected_);
    }
    if (ImGui::MenuItem("Delete")) {
      delete_entity(selected_);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Deselect")) {
      selected_ = null_entity();
    }
    ImGui::EndPopup();
  }

  ImGui::End();
}

void editor::build_properties()
{
  ImGui::Begin("Properties");

  if (!selected_) {
    ImGui::TextDisabled("No entity selected");
    ImGui::End();
    return;
  }

  // entity name (tag component)
  if (tag* t = scene_.get_component<tag>(selected_)) {
    char name[128];
    std::snprintf(name, sizeof(name), "%s", t->name.c_str());
    if (ImGui::InputText("Name", name, sizeof(name))) {
      t->name = name;
    }
  }

  ImGui::TextDisabled("Entity %u", selected_.id);
  ImGui::Separator();

  if (transform* t = scene_.get_component<transform>(selected_)) {
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::DragFloat3("Position", &t->position.x, 0.05f);
      vec3 rot_deg = { rad_to_deg(t->rotation.x), rad_to_deg(t->rotation.y), rad_to_deg(t->rotation.z) };
      if (ImGui::DragFloat3("Rotation", &rot_deg.x, 0.5f)) {
        t->rotation = { deg_to_rad(rot_deg.x), deg_to_rad(rot_deg.y), deg_to_rad(rot_deg.z) };
      }
      ImGui::DragFloat3("Scale", &t->scale.x, 0.05f, 0.01f, 100.0f);
    }
  }

  if (paint* p = scene_.get_component<paint>(selected_)) {
    if (ImGui::CollapsingHeader("Paint", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::ColorEdit4("Color", &p->color.x);
      ImGui::ColorEdit3("Emission Color", &p->emission_color.x);
      ImGui::SliderFloat("Reflectivity", &p->reflectivity, 0.0f, 1.0f);
      ImGui::SliderFloat("UV Scale (0 = mesh UV)", &p->uv_scale, 0.0f, 4.0f);

      ImGui::Separator();
      ImGui::TextUnformatted("Texture maps (path relative to project root)");
      char buf[512];
      auto path_edit = [&](const char* label, std::string& dst) {
        std::snprintf(buf, sizeof(buf), "%s", dst.c_str());
        ImGui::InputText(label, buf, sizeof(buf));
        dst = buf;
      };
      path_edit("Albedo", p->albedo);
      path_edit("Normal", p->normal);
      path_edit("Roughness", p->roughness);
      path_edit("Emission", p->emission);

      ImGui::Separator();
      ImGui::TextUnformatted("Per-face albedo (0=front 1=back 2=right 3=left 4=top 5=bottom)");
      for (int i = 0; i < 6; ++i) {
        std::snprintf(buf, sizeof(buf), "Face %d", i);
        path_edit(buf, p->face_albedo[i]);
      }
    }
  }

  if (light* l = scene_.get_component<light>(selected_)) {
    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
      const char* types[] = { "Directional", "Point", "Spot" };
      int current = static_cast<int>(l->type);
      if (ImGui::Combo("Type", &current, types, 3)) {
        l->type = static_cast<light_type>(current);
      }
      ImGui::ColorEdit3("Color", &l->color.x);
      ImGui::DragFloat("Intensity", &l->intensity, 0.05f, 0.0f, 100.0f);
      ImGui::DragFloat3("Position", &l->position.x, 0.05f);
      ImGui::DragFloat3("Direction", &l->direction.x, 0.05f);
      ImGui::DragFloat("Range", &l->range, 0.1f, 0.0f, 1000.0f);
      if (l->type == light_type::spot) {
        float inner_deg = rad_to_deg(std::acos(l->spot_cos_inner));
        float outer_deg = rad_to_deg(std::acos(l->spot_cos_outer));
        if (ImGui::SliderFloat("Inner Angle", &inner_deg, 1.0f, 89.0f)) {
          l->spot_cos_inner = std::cos(deg_to_rad(inner_deg));
        }
        if (ImGui::SliderFloat("Outer Angle", &outer_deg, 1.0f, 90.0f)) {
          l->spot_cos_outer = std::cos(deg_to_rad(outer_deg));
        }
      }
    }
  }

  // add component
  ImGui::Separator();
  if (ImGui::Button("+ Add Component", ImVec2(-1, 0))) {
    ImGui::OpenPopup("##add_component");
  }
  if (ImGui::BeginPopup("##add_component")) {
    if (!scene_.has_component<transform>(selected_)) {
      if (ImGui::MenuItem("Transform")) {
        scene_.add_component<transform>(selected_);
      }
    }
    if (!scene_.has_component<tag>(selected_)) {
      if (ImGui::MenuItem("Tag")) {
        scene_.add_component<tag>(selected_, entity_label(selected_));
      }
    }
    if (!scene_.has_component<light>(selected_)) {
      if (ImGui::MenuItem("Light")) {
        scene_.add_component<light>(selected_);
      }
    }
    if (!scene_.has_component<paint>(selected_)) {
      if (ImGui::MenuItem("Paint")) {
        scene_.add_component<paint>(selected_);
      }
    }
    ImGui::EndPopup();
  }

  ImGui::End();
}

void editor::build_status_bar()
{
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  float height = ImGui::GetFrameHeight() + 8.0f;
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - height));
  ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, height));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
  ImGui::Begin("##status_bar", nullptr,
    ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
  ImGui::PopStyleVar(2);

  ImGuiIO& io = ImGui::GetIO();
  ImGui::Text("FPS %.0f  |  %.2f ms", io.Framerate, 1000.0 / io.Framerate);
  ImGui::SameLine();

  const auto* pool = scene_.get_pool<transform>();
  ImGui::Text("Entities %d", pool ? pool->size() : 0);
  ImGui::SameLine();

  ImGui::Text("Camera dist %.2f", cam_dist_);
  ImGui::SameLine();

  const char* mode = "Move";
  if (gizmo_mode_ == gizmo_mode::rotate) mode = "Rotate";
  else if (gizmo_mode_ == gizmo_mode::scale) mode = "Scale";
  ImGui::Text("Gizmo %s%s", mode, gizmo_snap_ ? " (snap)" : "");

  if (!status_message_.empty()) {
    ImGui::SameLine();
    ImGui::TextUnformatted(status_message_.c_str());
  }

  float avail = ImGui::GetContentRegionAvail().x;
  std::string sel = selected_ ? entity_label(selected_) : "none";
  float sel_w = ImGui::CalcTextSize(sel.c_str()).x;
  ImGui::SameLine(avail - sel_w);
  ImGui::Text("Selection: %s", sel.c_str());

  ImGui::End();
}

void editor::build_map_dialogs()
{
  // -- save map --
  if (map_popup_save_) {
    ImGui::OpenPopup("Save Map");
    map_popup_save_ = false;
  }
  if (ImGui::BeginPopupModal("Save Map", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::InputText("Map name", map_name_, sizeof(map_name_));
    if (ImGui::Button("Save")) {
      std::string path = std::string(map_name_) + ".lev";
      if (scene_.save<transform, tag, paint>(path.c_str())) {
        status_message_ = "Map saved: " + path;
      } else {
        status_message_ = "Failed to save map: " + path;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  // -- load map --
  if (map_popup_load_) {
    ImGui::OpenPopup("Load Map");
    map_popup_load_ = false;
  }
  if (ImGui::BeginPopupModal("Load Map", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::InputText("Map name", map_name_, sizeof(map_name_));
    ImGui::TextUnformatted("Available maps:");
    ImGui::BeginChild("##map_list", ImVec2(280, 150), true);
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
      if (!entry.is_regular_file()) continue;
      if (entry.path().extension() != ".lev") continue;
      std::string stem = entry.path().stem().string();
      if (ImGui::Selectable(stem.c_str(), stem == map_name_)) {
        std::snprintf(map_name_, sizeof(map_name_), "%s", stem.c_str());
      }
    }
    ImGui::EndChild();
    if (ImGui::Button("Load")) {
      std::string path = std::string(map_name_) + ".lev";
      if (scene_.load<transform, tag, paint>(path.c_str())) {
        selected_ = null_entity();
        status_message_ = "Map loaded: " + path;
      } else {
        status_message_ = "Failed to load map: " + path;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void editor::init_imgui()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigWindowsMoveFromTitleBarOnly = true;

  // -- refined dark style --
  ImGuiStyle& s = ImGui::GetStyle();
  s.WindowRounding = 4.0f;
  s.FrameRounding = 3.0f;
  s.GrabRounding = 3.0f;
  s.TabRounding = 3.0f;
  s.ChildRounding = 3.0f;
  s.PopupRounding = 3.0f;
  s.FramePadding = ImVec2(6, 4);
  s.ItemSpacing = ImVec2(8, 5);
  s.DockingSeparatorSize = 2.0f;

  ImVec4* c = s.Colors;
  c[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
  c[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
  c[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
  c[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.13f, 0.16f, 1.00f);
  c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
  c[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
  c[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
  c[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.19f, 1.00f);
  c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
  c[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
  c[ImGuiCol_Tab] = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
  c[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
  c[ImGuiCol_TabActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
  c[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
  c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
  c[ImGuiCol_Button] = ImVec4(0.13f, 0.13f, 0.16f, 1.00f);
  c[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
  c[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.32f, 1.00f);
  c[ImGuiCol_Header] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
  c[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
  c[ImGuiCol_HeaderActive] = ImVec4(0.27f, 0.27f, 0.33f, 1.00f);
  c[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
  c[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.30f, 0.37f, 1.00f);
  c[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.48f, 1.00f);
  c[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.70f, 0.30f, 1.00f);
  c[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.45f, 0.80f, 0.40f);

  ImGui_ImplGlfw_InitForOpenGL(app_->get_window().native(), true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void editor::shutdown_imgui()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

std::string editor::entity_label(const entity& e) const
{
  if (const tag* t = scene_.get_component<tag>(e)) {
    if (!t->name.empty()) return t->name;
  }
  return "Entity " + std::to_string(e.id);
}

std::string editor::make_unique_name(const std::string& base) const
{
  for (int i = 1; i < 1000; ++i) {
    std::string candidate = base;
    if (i > 1) {
      candidate += " (" + std::to_string(i) + ")";
    }
    bool taken = false;
    scene_.for_each<tag>([&](entity, const tag& t) {
      if (t.name == candidate) taken = true;
    });
    if (!taken) return candidate;
  }
  return base;
}

void editor::create_entity(const char* name)
{
  entity e = scene_.create_entity();
  scene_.add_component<transform>(e);
  scene_.add_component<tag>(e, make_unique_name(name));
  selected_ = e;
}

void editor::duplicate_entity(const entity& e)
{
  if (!scene_.is_alive(e)) return;

  entity n = scene_.create_entity();
  if (const transform* t = scene_.get_component<transform>(e)) {
    scene_.add_component<transform>(n, *t);
  }
  if (const tag* t = scene_.get_component<tag>(e)) {
    scene_.add_component<tag>(n, make_unique_name(t->name));
  }
  selected_ = n;
}

void editor::delete_entity(const entity& e)
{
  if (!scene_.is_alive(e)) return;
  scene_.destroy_entity(e);
  if (e == selected_) {
    selected_ = null_entity();
  }
}
