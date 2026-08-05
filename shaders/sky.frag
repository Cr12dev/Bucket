#version 330 core

in vec3 v_ray;

uniform vec3 u_camera_pos;
uniform vec3 u_sun_dir;
uniform vec3 u_sky_top;
uniform vec3 u_sky_horizon;
uniform vec3 u_sky_ground;

out vec4 frag_color;

void main()
{
  vec3 dir = normalize(v_ray - u_camera_pos);
  float up = dir.y;

  // atmosphere gradient (approximate Rayleigh scattering)
  vec3 top     = u_sky_top;
  vec3 horizon = u_sky_horizon;
  vec3 ground  = u_sky_ground;

  float t = clamp(up * 0.5 + 0.5, 0.0, 1.0);
  // steeper gradient: the blue zenith only appears high up, mid-sky stays
  // warm hazy (a dusty desert look) instead of turning blue everywhere
  vec3 col = mix(horizon, top, pow(t, 2.4));

  // sun disc + glow
  float sun_angle = max(dot(dir, normalize(u_sun_dir)), 0.0);
  float disc = pow(sun_angle, 900.0);
  float glow = pow(sun_angle, 10.0) * 0.55;
  col += vec3(1.0, 0.92, 0.75) * (disc * 2.2 + glow);

  // ground below horizon
  col = mix(ground, col, smoothstep(-0.02, 0.05, up));

  // horizon haze (lighter for a clear daytime sky)
  col = mix(col, horizon, exp(-abs(up) * 6.0) * 0.15);

  frag_color = vec4(col, 1.0);
}
