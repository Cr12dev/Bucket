#pragma once

#include "math/vec3.hpp"

// Global atmosphere of the game world: ambient light, distance fog and
// sky colors. It lives as buckit::atmosphere_ so game code can tune it
// from game.cpp (start()/update()), e.g. for a warm golden hour, an
// overcast day or a night scene. Values are in linear space; lighting.frag
// applies Reinhard tonemapping + gamma correction on top.
struct atmosphere {
  vec3 ambient = vec3(0.34f, 0.32f, 0.28f);    // fill light (shadow color)
  vec3 fog_color = vec3(0.88f, 0.84f, 0.74f);  // distance haze color
  float fog_start = 45.0f;                     // fog begins here
  float fog_end = 95.0f;                       // fog is fully opaque here
  bool fog_enabled = true;                     // set false to disable fog

  vec3 sky_top = vec3(0.50f, 0.62f, 0.76f);        // zenith (pale hazy blue)
  vec3 sky_horizon = vec3(0.90f, 0.88f, 0.80f);    // horizon (warm haze, no cyan)
  vec3 sky_ground = vec3(0.55f, 0.49f, 0.40f);     // below the horizon

  vec3 sun_color = vec3(1.0f, 0.96f, 0.88f);   // overrides the directional light
  float sun_intensity = 1.5f;

};
