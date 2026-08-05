#include <Buckit.hpp>

// ---------------------------------------------------------------------------
// Level content: the CS 1.6-style sandy arena. This is data, not logic:
// editing the map here is the only reason to touch this file.
// Every object carries a mandatory, unique id string (last parameter) so
// game code can find and edit it from update() by name.
// ---------------------------------------------------------------------------

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
                                 sand, 0.06f, "suelo");
  {
    paint& p = *scene_.get_component<paint>(floor_e);
    p.albedo = "textures/brick_albedo.png";
    p.roughness = "textures/brick_roughness.png";
    p.color = vec4(1.0f);
  }
  prefab::wall(scene_, vec3(0.0f, 3.0f, -30.0f), vec3(80.0f, 6.0f, 1.0f), wall, "muro_norte");
  prefab::wall(scene_, vec3(0.0f, 3.0f, 30.0f), vec3(80.0f, 6.0f, 1.0f), wall, "muro_sur");
  prefab::wall(scene_, vec3(-40.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 60.0f), wall, "muro_oeste");
  prefab::wall(scene_, vec3(40.0f, 3.0f, 0.0f), vec3(1.0f, 6.0f, 60.0f), wall, "muro_este");

  // --- central bombsite platform with stairs ---
  prefab::box(scene_, vec3(0.0f, 0.75f, 0.0f), vec3(14.0f, 1.5f, 10.0f), platform, "plataforma_central");
  prefab::box(scene_, vec3(8.5f, 1.0f, 0.0f), vec3(3.0f, 2.0f, 8.0f), platform, "escalera_d1");
  prefab::box(scene_, vec3(10.5f, 2.0f, 0.0f), vec3(3.0f, 2.0f, 8.0f), platform, "escalera_d2");
  prefab::box(scene_, vec3(-8.5f, 1.0f, 0.0f), vec3(3.0f, 2.0f, 8.0f), platform, "escalera_i1");
  prefab::box(scene_, vec3(-10.5f, 2.0f, 0.0f), vec3(3.0f, 2.0f, 8.0f), platform, "escalera_i2");

  // crates on the platform: wood-textured + a glowing lamp crate
  entity crate_e = prefab::crate(scene_, vec3(-3.0f, 2.5f, 0.0f), 2.0f, wood, "caja_madera1");
  {
    paint& p = *scene_.get_component<paint>(crate_e);
    p.albedo = "textures/wood_albedo.png";
    p.normal = "textures/wood_normal.png";
    p.roughness = "textures/wood_roughness.png";
    p.color = vec4(1.0f);
  }

  entity lamp_e = prefab::crate(scene_, vec3(2.0f, 2.5f, -2.0f), 2.0f, wood_dark, "lampara");
  {
    paint& p = *scene_.get_component<paint>(lamp_e);
    p.albedo = "textures/metal_albedo.png";
    p.roughness = "textures/metal_roughness.png";
    p.emission = "textures/emissive_lamp.png";
    p.emission_color = vec3(2.4f, 1.9f, 1.1f);
    p.color = vec4(1.0f);
  }

  // per-face albedo demo: every face gets its own texture
  entity face_e = prefab::crate(scene_, vec3(4.0f, 2.5f, 0.0f), 2.0f, stone, "caja_caras");
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
  prefab::pillar(scene_, vec3(-12.0f, 4.0f, -12.0f), 2.0f, 8.0f, stone, "pilar_no");
  prefab::pillar(scene_, vec3(12.0f, 4.0f, -12.0f), 2.0f, 8.0f, stone, "pilar_ne");
  prefab::pillar(scene_, vec3(-12.0f, 4.0f, 12.0f), 2.0f, 8.0f, stone, "pilar_so");
  prefab::pillar(scene_, vec3(12.0f, 4.0f, 12.0f), 2.0f, 8.0f, stone, "pilar_se");

  // --- west/east wall segments with doorways ---
  prefab::wall(scene_, vec3(-25.0f, 3.0f, -12.0f), vec3(2.0f, 6.0f, 8.0f), wall, "muro_oeste_n");
  prefab::wall(scene_, vec3(-25.0f, 3.0f, 2.0f), vec3(2.0f, 6.0f, 8.0f), wall, "muro_oeste_s");
  prefab::wall(scene_, vec3(25.0f, 3.0f, -12.0f), vec3(2.0f, 6.0f, 8.0f), wall, "muro_este_n");
  prefab::wall(scene_, vec3(25.0f, 3.0f, 2.0f), vec3(2.0f, 6.0f, 8.0f), wall, "muro_este_s");

  // --- arch / tunnel frame ---
  prefab::pillar(scene_, vec3(-4.0f, 2.5f, 18.0f), 2.0f, 5.0f, stone, "arco_i");
  prefab::pillar(scene_, vec3(4.0f, 2.5f, 18.0f), 2.0f, 5.0f, stone, "arco_d");
  prefab::box(scene_, vec3(0.0f, 5.5f, 18.0f), vec3(12.0f, 1.0f, 2.0f), stone, "arco_tope");

  // --- crate cluster NW ---
  entity c1 = prefab::crate(scene_, vec3(-20.0f, 1.0f, -15.0f), 2.0f, wood, "caja_madera2");
  entity c2 = prefab::crate(scene_, vec3(-20.0f, 3.0f, -15.0f), 2.0f, wood_dark, "caja_metal1");
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
  prefab::crate(scene_, vec3(-18.0f, 1.0f, -13.5f), 2.0f, crate_blue, "caja_azul1");
  prefab::crate(scene_, vec3(-16.0f, 1.0f, -15.0f), 2.0f, wood, "caja_madera3");
  prefab::crate(scene_, vec3(-16.0f, 3.0f, -15.0f), 2.0f, wood, "caja_madera4");

  // --- crate cluster SE ---
  prefab::crate(scene_, vec3(18.0f, 1.0f, 12.0f), 2.0f, wood, "caja_madera5");
  prefab::crate(scene_, vec3(20.0f, 1.0f, 14.0f), 2.0f, crate_green, "caja_verde1");
  prefab::crate(scene_, vec3(22.0f, 1.0f, 12.0f), 2.0f, wood_dark, "caja_madera6");

  // --- big blocks NE ---
  prefab::box(scene_, vec3(30.0f, 1.5f, -25.0f), vec3(3.0f, 3.0f, 3.0f), crate_blue, "bloque_azul");
  prefab::box(scene_, vec3(33.0f, 2.5f, -23.0f), vec3(3.0f, 5.0f, 3.0f), crate_green, "bloque_verde");

  // --- sandbags along the south wall ---
  prefab::sandbag(scene_, vec3(-10.0f, 0.5f, 28.25f), vec3(3.0f, 1.0f, 1.5f), stone, "saco_1");
  prefab::sandbag(scene_, vec3(-6.5f, 0.5f, 28.25f), vec3(3.0f, 1.0f, 1.5f), stone, "saco_2");
  prefab::sandbag(scene_, vec3(-3.0f, 0.5f, 28.25f), vec3(3.0f, 1.0f, 1.5f), stone, "saco_3");
  prefab::sandbag(scene_, vec3(4.0f, 0.5f, 28.25f), vec3(3.0f, 1.0f, 1.5f), stone, "saco_4");
  prefab::sandbag(scene_, vec3(7.5f, 0.5f, 28.25f), vec3(3.0f, 1.0f, 1.5f), stone, "saco_5");

  // --- scattered crates ---
  prefab::crate(scene_, vec3(-30.0f, 1.0f, 20.0f), 2.0f, crate_green, "caja_verde2");
  prefab::crate(scene_, vec3(-28.0f, 1.0f, 22.0f), 2.0f, wood, "caja_madera7");
  prefab::crate(scene_, vec3(36.0f, 1.0f, -5.0f), 2.0f, wood, "caja_madera8");
  prefab::crate(scene_, vec3(36.0f, 3.0f, -5.0f), 2.0f, wood_dark, "caja_madera9");
}
