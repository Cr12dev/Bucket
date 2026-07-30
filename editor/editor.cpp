#include "editor.hpp"

#include <cstdio>
#include <cmath>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <Buckit.hpp>

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
  cube_mesh_ = mesh::cube();
}

void editor::start()
{
  init_imgui();

  camera_.set_position({ 0.0f, 0.0f, 3.0f });

  // default scene with a few primitives
  for (int i = 0; i < 3; ++i) {
    entity e = scene_.create_entity();
    auto& t = scene_.add_component<transform>(e);
    t.position.x = static_cast<float>(i) * 1.5f - 1.5f;
  }
}

void editor::update(double dt)
{
  // -- editor camera orbit --
  auto& win = app_->get_window();

  double mx, my;
  glfwGetCursorPos(win.native(), &mx, &my);

  if (glfwGetMouseButton(win.native(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
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

  // scroll zoom
  static double scroll_y = 0.0;
  static bool scroll_init = false;
  if (!scroll_init) {
    glfwSetScrollCallback(win.native(), [](GLFWwindow*, double, double y) {
      scroll_y += y;
    });
    scroll_init = true;
  }
  cam_dist_ -= static_cast<float>(scroll_y * 0.5);
  if (cam_dist_ < 0.5f) cam_dist_ = 0.5f;
  if (cam_dist_ > 50.0f) cam_dist_ = 50.0f;
  scroll_y *= 0.9;

  // update camera position & orientation (yaw/pitch in degrees)
  float cx = cam_dist_ * std::cos(cam_pitch_) * std::sin(cam_yaw_);
  float cy = cam_dist_ * std::sin(cam_pitch_);
  float cz = cam_dist_ * std::cos(cam_pitch_) * std::cos(cam_yaw_);
  camera_.set_position({ cx, cy, cz });
  camera_.set_yaw(cam_yaw_ * 180.0f / 3.14159265f + 90.0f);
  camera_.set_pitch(-cam_pitch_ * 180.0f / 3.14159265f);

  // -- imgui new frame --
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void editor::render()
{
  auto& win = app_->get_window();
  int ww = win.width();
  int wh = win.height();

  // clear main framebuffer (background behind imgui)
  render_command::set_viewport(0, 0, ww, wh);
  render_command::set_clear_color(0.12f, 0.12f, 0.14f, 1.0f);
  render_command::clear();

  // -- build gui (viewport renders to its own FBO) --
  build_menu_bar();
  build_viewport();
  build_hierarchy();
  build_properties();

  if (show_demo_) {
    ImGui::ShowDemoWindow(&show_demo_);
  }

  // -- render imgui overlay --
  ImGui::Render();
  glViewport(0, 0, ww, wh);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void editor::build_menu_bar()
{
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Scene")) {
        scene_ = scene{};
      }
      if (ImGui::MenuItem("Save")) {}
      if (ImGui::MenuItem("Load")) {}
      ImGui::Separator();
      if (ImGui::MenuItem("Exit")) {
        glfwSetWindowShouldClose(app_->get_window().native(), true);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("Demo Window", nullptr, &show_demo_);
      ImGui::MenuItem("Show Grid", nullptr, &show_grid_);
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

  ImVec2 size = ImGui::GetContentRegionAvail();
  int w = static_cast<int>(size.x);
  int h = static_cast<int>(size.y);

  if (w > 0 && h > 0) {
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

    // render to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, vp_w_, vp_h_);
    render_command::set_clear_color(0.18f, 0.18f, 0.22f, 1.0f);
    render_command::clear();

    if (show_grid_) {
      render_command::draw_grid(20, 1.0f);
    }

  // render scene
  camera_.set_perspective(60.0f, static_cast<float>(vp_w_) / vp_h_, 0.1f, 100.0f);
  default_shader_->bind();
  default_shader_->set_uniform("u_view_proj", camera_.view_projection().data());
  default_shader_->set_uniform("u_albedo", 0);
  default_shader_->set_uniform("u_color", 0.6f, 0.6f, 0.7f, 1.0f);

  scene_.for_each<transform>([&](entity e, transform& t) {
    bool sel = (e == selected_);
    default_shader_->set_uniform("u_color",
      sel ? 1.0f : 0.6f,
      sel ? 0.8f : 0.6f,
      sel ? 0.2f : 0.7f,
      1.0f);
    mat4 model = t.matrix();
    default_shader_->set_uniform("u_model", model.data());
    render_command::draw_indexed(cube_mesh_);
  });

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // display in imgui
    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(color_tex_)),
                 ImVec2(static_cast<float>(vp_w_), static_cast<float>(vp_h_)),
                 ImVec2(0, 1), ImVec2(1, 0));

    // click to deselect (actual ray-picking would go here)
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      selected_ = null_entity();
    }
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void editor::build_hierarchy()
{
  ImGui::Begin("Scene Hierarchy");

  if (ImGui::Button("+ Add Entity")) {
    entity e = scene_.create_entity();
    scene_.add_component<transform>(e);
    selected_ = e;
  }

  ImGui::Separator();

  scene_.for_each<transform>([&](entity e, transform&) {
    char label[32];
    std::snprintf(label, sizeof(label), "Entity %u", e.id);
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
    if (e == selected_) {
      flags |= ImGuiTreeNodeFlags_Selected;
    }
    ImGui::TreeNodeEx(label, flags);
    if (ImGui::IsItemClicked()) {
      selected_ = e;
    }
    ImGui::TreePop();
  });

  ImGui::End();
}

void editor::build_properties()
{
  ImGui::Begin("Properties");

  if (selected_ && scene_.has_component<transform>(selected_)) {
    auto* t = scene_.get_component<transform>(selected_);
    if (!t) return;

    ImGui::Text("Entity %u", selected_.id);
    ImGui::Separator();
    ImGui::Text("Transform");
    ImGui::DragFloat3("Position", &t->position.x, 0.05f);
    ImGui::DragFloat3("Rotation", &t->rotation.x, 0.05f);
    ImGui::DragFloat3("Scale",    &t->scale.x, 0.05f, 0.01f, 100.0f);
  }

  ImGui::End();
}

void editor::init_imgui()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplGlfw_InitForOpenGL(app_->get_window().native(), true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void editor::shutdown_imgui()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
